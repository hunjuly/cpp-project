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

				//RequestProcessed()�� �����ϴ� ���ε�
				//Stop()�� ���ؼ� CloseConnection()�� �Ǹ� �� �Ǵϱ� �� �ɾ���.
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
					else if(receivedBytes == 0) //CloseConnection()�� ȣ��Ǹ� �� �ڵ尡 ȣ��ȴ�.
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
					//�ۼ����� ��û�ߴ��� ��ٸ� �ʿ� ���� �����͸� �ٷ� �����Դ�.
					//���⼭ PostQueue�� ��ɾ �־ ó���Ϸ��� �ߴ�.
					//�׷��� IOCP�� ����� �ٷ� ó���ص� Queue�� ��ɾ �ִ� �� ����.
					//�׷��� �ߺ��ؼ� ����� ó���ϴ� ��찡 �߻��ߴ�.
					//�׷��ϱ� ��ɾ �ٷ� ó���ߴٰ� �ϴ���. �ƹ��͵� �ϸ� �ȵȴ�.
					//��򰡿��� GetQueue �����ؼ� ó���� �� ���̴�.
				}
				else if(result == SOCKET_ERROR && WSA_IO_PENDING == WSAGetLastError()) //��û ����� ��ٸ���.
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
