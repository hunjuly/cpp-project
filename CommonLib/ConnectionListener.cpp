#include "StdAfx.h"
#include "ConnectionListener.h"
#include "Network.h"
#include "Transfer.h"

namespace CommonLib
{
	namespace Network
	{
		namespace Async
		{
			ConnectionListener::ConnectionListener(ITransferPool& transferPool, int numberOfThreads)
				: IRequestable(CreateRequesterHandler(numberOfThreads)), transferPool(transferPool)
			{
				this->lpfnAcceptEx = NULL;
			}

			Io::Iocp::RequestHandler& ConnectionListener::CreateRequesterHandler(int numberOfThreads)
			{
				requester = new Io::Iocp::RequestHandler(numberOfThreads, 0);

				return *requester;
			}

			ConnectionListener::~ConnectionListener(void)
			{
				delete requester;

				_ASSERT_EXPR(IRequestable::IsAvailable() == false, L"종료하기 전에 ConnectionListener을 Stop해야 합니다.");
			}

			bool ConnectionListener::Start(int port, int backLog)
			{
				IRequestable::Initialize(CreateTcpSocket());

				GUID GuidAcceptEx = WSAID_ACCEPTEX;
				DWORD dwBytes;

				WSAIoctl(IRequestable::Socket(),
						 SIO_GET_EXTENSION_FUNCTION_POINTER,
						 &GuidAcceptEx,
						 sizeof(GuidAcceptEx),
						 &lpfnAcceptEx,
						 sizeof(lpfnAcceptEx),
						 &dwBytes,
						 NULL,
						 NULL);

				SOCKADDR_IN servAddr;
				{
					ZeroMemory(&servAddr, sizeof(SOCKADDR_IN));
					servAddr.sin_family = AF_INET;
					servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
					servAddr.sin_port = htons(port);
				}

				while(::bind(IRequestable::Socket(), (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
				{
					printf("Error=%d", GetLastError());

					//Port가 사용중이다.
					CloseSocket();

					return false;
				}

				if(::listen(IRequestable::Socket(), backLog) == SOCKET_ERROR)
				{
					THROW_LIBRARY(Diagnostics::GetErrorMessage(WSAGetLastError()));
				}

				//Accept 요청을 미리 걸어놓는다.
				for(int i = 0; i < transferPool.GetPoolSize(); ++i)
					RequestAccept();

				return true;
			}

			void ConnectionListener::Stop()
			{
				CloseSocket();

				//transferPool와 aRequestHandler의 순서를 바꾸면 안 된다. 데드락 발생한다.
				transferPool.Uninitialize();

				IRequestable::GetRequestHandler().Stop();
			}

			void ConnectionListener::CloseSocket()
			{
				//Start()가 실패한 후에 Stop을 하면 아래 조건을 만족시키지 못함
				if(IRequestable::IsAvailable())
				{
					SOCKET tempHandle = IRequestable::Socket();

					IRequestable::ResetHandle();

					closesocket(tempHandle);
				}
			}

			//AcceptEx는 여러번 요청하는 것이 맞지만 같은 핸들에 서로 다른 CompKey(Accept*)로 요청하면
			//당연히 하나의 Accept만 불리게 된다. 따라서 Accept의 인스턴스는 하나여야 하고
			//Request 할 때 마다 Overlapped 구로체를 다르게 해서 요청을 하자.
			//Overlapped는 상속해서 여기에 transfer를 넣으면 된다.
			bool ConnectionListener::RequestAccept()
			{
				//void ConnectionListener::RequestProcessed(int receivedBytes, LPOVERLAPPED pOverlapped)에서
				//while() 빠져나가기 위해서 한다.
				if(IRequestable::IsAvailable() == false)
					return true;

				Transfer* transfer = transferPool.IdleTransfer();

				if(transfer != NULL)
				{
					ZeroMemory((OVERLAPPED*)transfer, sizeof(OVERLAPPED));

					DWORD dwBytes = 0;

					BOOL result = lpfnAcceptEx(
						IRequestable::Socket()
						, transfer->Socket()
						, transfer->SocketAddressBuffer()
						, 0
						, sizeof(sockaddr_in) + 16
						, sizeof(sockaddr_in) + 16
						, &dwBytes
						, transfer
						);

					//handle이 NULL이면 서버 종료 중이다. 그냥 종료한다.
					if(result == TRUE)
					{
						Debug::Output(L"\r\n******************\r\n\r\n\r\nDid Direct Connected Check ConnectionListener\r\n\r\n\r\n******************\r\n");
					}
					else if(IRequestable::IsAvailable())
					{
						int errorCode = WSAGetLastError();

						//errorCode == WSAEINVAL가 종종 발생한다.
						//유효하지 않은 인자라는 뜻인데 socket이 close되지 않은 것이다.
						//errorCode==ERROR_NETNAME_DELETED(64)인 경우도 발생한다는데 난 모르겠다.
						if(WSA_IO_PENDING != errorCode)
						{
							transferPool.RemakeTransfer(transfer);
							return false;
						}
					}
				}

				return true;
			}

			void ConnectionListener::RequestProcessed(int receivedBytes, LPOVERLAPPED pOverlapped)
			{
				Transfer* transfer = (Transfer*)pOverlapped;

				//void ConnectionListener::Stop()에서 closesocket을 실행하면 RequestProcessed가 호출된다.
				//handle==0 인지를 보면 accept가 된건지 closesocket인지 알 수 있다.
				//handle==0 말고 다른 방법은 없을까?
				if(IRequestable::IsAvailable() == false)
					return;

				transfer->ClientConnected();

				//이 부분은 고쳐라.
				//false==RequestAccept()가 되는 건 closesocket이 안 된 transfer를 accept 하기 때문인데
				//동기화 문제를 좀 더 다듬을 필요가 있겠다.
				//그러나 일단 이렇게 해도 큰 문제는 없어 보인다.
				while(false == RequestAccept())
				{
					Sleep(1000);
				}
			}
		}
	}
}
