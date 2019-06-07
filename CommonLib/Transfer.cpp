#include "StdAfx.h"
#include "Transfer.h"
#include "Network.h"
#include "TransferPool.h"

namespace CommonLib
{
	namespace Network
	{
		namespace Async
		{
			using namespace Io::Iocp;

			Transfer::Transfer(ITransferPool& transferPool, Io::Iocp::RequestHandler& requester)
				: IRequestable((HANDLE)CreateTcpSocket(), requester), transferPool(transferPool)
			{
				RefreshLastSignalTime();

				isConnected = false;
				isDoingClose = false;

				GUID GuidGetSocketAddrs = WSAID_GETACCEPTEXSOCKADDRS;
				DWORD dwBytes;

				WSAIoctl(IRequestable::Socket(),
						 SIO_GET_EXTENSION_FUNCTION_POINTER,
						 &GuidGetSocketAddrs,
						 sizeof(GuidGetSocketAddrs),
						 &lpfnGetAcceptExSockAddrs,
						 sizeof(lpfnGetAcceptExSockAddrs),
						 &dwBytes,
						 NULL,
						 NULL);
			}

			void Transfer::Reset()
			{
				Stop();
				RefreshLastSignalTime();

				isConnected = false;
				isDoingClose = false;

				IRequestable::Initialize(CreateTcpSocket());
				transferPool.ReturnUsedTransfer(this);
			}

			Transfer::~Transfer(void)
			{
				_ASSERT(IRequestable::Socket() == INVALID_SOCKET);
			}

			BYTE* Transfer::SocketAddressBuffer()
			{
				return this->socketAddrBuffer;
			}

			void Transfer::ClientConnected()
			{
				_ASSERT(isDoingClose == false && isConnected == false);

				RefreshLastSignalTime();

				isConnected = true;

				Connected();
			}

			void Transfer::SockAddr(sockaddr** pRemoteAddr, sockaddr** pLocalAddr)
			{
				int remoteLen = 0;
				int localLen = 0;

				lpfnGetAcceptExSockAddrs(
					SocketAddressBuffer()
					, 0
					, sizeof(sockaddr_in) + 16
					, sizeof(sockaddr_in) + 16
					, pLocalAddr
					, &localLen
					, pRemoteAddr
					, &remoteLen
					);
			}

			tstring Transfer::RemoteIp()
			{
				sockaddr* pRemoteAddr = nullptr;
				sockaddr* pLocalAddr = nullptr;

				SockAddr(&pRemoteAddr, &pLocalAddr);
				
				vector<WCHAR> buffer(255);

				return InetNtop(AF_INET, &((sockaddr_in*)pRemoteAddr)->sin_addr, &buffer[0], buffer.size());
			}

			tstring Transfer::LocalIp()
			{
				sockaddr* pRemoteAddr = nullptr;
				sockaddr* pLocalAddr = nullptr;

				SockAddr(&pRemoteAddr, &pLocalAddr);

				vector<WCHAR> buffer(255);

				return InetNtop(AF_INET, &((sockaddr_in*)pLocalAddr)->sin_addr, &buffer[0], buffer.size());
			}

			void Transfer::Stop()
			{
				if(IRequestable::IsAvailable())
				{
					SOCKET tempHandle = IRequestable::Socket();

					IRequestable::ResetHandle();

					closesocket(tempHandle);
				}
			}

			void Transfer::CloseConnection()
			{
				if(isDoingClose == false)
				{
					isDoingClose = true;

					Disconnected();

					::shutdown(IRequestable::Socket(), SD_BOTH);

					::TransmitFile(IRequestable::Socket(), NULL, 0, 0, this, 0, TF_DISCONNECT | TF_REUSE_SOCKET);
				}
			}

			void Transfer::RequestProcessed(int receivedBytes, LPOVERLAPPED pOverlapped)
			{
				RefreshLastSignalTime();

				//RequestProcessed()를 실행하는 중인데
				//Stop()에 의해서 CloseConnection()이 되면 안 되니까 락 걸었다.
				closeCs.SafeLock([&]()
				{
					if(receivedBytes > 0)
					{
						RequestProcessed(receivedBytes);
					}
					else if(IsDisconnectedByClient())
					{
						CloseConnection();
					}
					else if(receivedBytes == 0) //CloseConnection()이 호출되면 이 코드가 호출된다.
					{
						isDoingClose = false;
						isConnected = false;

						transferPool.ReturnUsedTransfer(this);
					}
				});
			}

			bool Transfer::IsDisconnectedByClient()
			{
				return isConnected == true && isDoingClose == false;
			}

			void Transfer::SendFinish()
			{
				CloseConnection();
			}

			void Transfer::DoTransfer(BYTE* buffer, int requestSize, const function<int(WSABUF wsaBuf, int* transferSize)>& task)
			{
				WSABUF wsaBuf;
				{
					ZeroMemory((OVERLAPPED*)this, sizeof(OVERLAPPED));
					wsaBuf.len = requestSize;
					wsaBuf.buf = (char*)buffer;
				}

				int transferSize = 0;
				int result = task(wsaBuf, &transferSize);

				if(result == 0)
				{
					//송수신을 요청했더니 기다릴 필요 없이 데이터를 바로 가져왔다.
					//여기서 PostQueue로 명령어를 넣어서 처리하려고 했다.
					//그러나 IOCP는 명령을 바로 처리해도 Queue에 명령어를 넣는 것 같다.
					//그래서 중복해서 명령을 처리하는 경우가 발생했다.
					//그러니까 명령어를 바로 처리했다고 하더라도. 아무것도 하면 안된다.
					//어딘가에서 GetQueue 실행해서 처리가 될 것이다.
				}
				else if(result == SOCKET_ERROR && WSA_IO_PENDING == WSAGetLastError()) //요청 결과를 기다린다.
				{
				}
				else
					CloseConnection();
			}

			void Transfer::RecvRequest(BYTE* buffer, int requestSize)
			{
				currentTask = Task::Recv;

				DoTransfer(buffer, requestSize, [&](WSABUF wsaBuf, int* transferSize)
				{
					DWORD flags = 0;
					return WSARecv(IRequestable::Socket(), &wsaBuf, 1, (LPDWORD)transferSize, &flags, this, NULL);
				});
			}

			void Transfer::SendRequest(BYTE* buffer, int requestSize)
			{
				currentTask = Task::Send;

				DoTransfer(buffer, requestSize, [&](WSABUF wsaBuf, int* transferSize)
				{
					return WSASend(IRequestable::Socket(), &wsaBuf, 1, (LPDWORD)transferSize, 0, this, NULL);
				});
			}

			void Transfer::SendRequest(const vector<BYTE>& buffer)
			{
				currentTask = Task::Send;

				DoTransfer((BYTE*)&buffer[0], buffer.size(), [&](WSABUF wsaBuf, int* transferSize)
				{
					return WSASend(IRequestable::Socket(), &wsaBuf, 1, (LPDWORD)transferSize, 0, this, NULL);
				});
			}

			int Transfer::IdleTime()
			{
				return (int)(::GetTickCount64() - lastSignalTime) / 1000;
			}

			void Transfer::RefreshLastSignalTime()
			{
				lastSignalTime = ::GetTickCount64();
			}
		}
	}
}
