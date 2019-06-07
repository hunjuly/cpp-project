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

				_ASSERT_EXPR(IRequestable::IsAvailable() == false, L"�����ϱ� ���� ConnectionListener�� Stop�ؾ� �մϴ�.");
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

					//Port�� ������̴�.
					CloseSocket();

					return false;
				}

				if(::listen(IRequestable::Socket(), backLog) == SOCKET_ERROR)
				{
					THROW_LIBRARY(Diagnostics::GetErrorMessage(WSAGetLastError()));
				}

				//Accept ��û�� �̸� �ɾ���´�.
				for(int i = 0; i < transferPool.GetPoolSize(); ++i)
					RequestAccept();

				return true;
			}

			void ConnectionListener::Stop()
			{
				CloseSocket();

				//transferPool�� aRequestHandler�� ������ �ٲٸ� �� �ȴ�. ����� �߻��Ѵ�.
				transferPool.Uninitialize();

				IRequestable::GetRequestHandler().Stop();
			}

			void ConnectionListener::CloseSocket()
			{
				//Start()�� ������ �Ŀ� Stop�� �ϸ� �Ʒ� ������ ������Ű�� ����
				if(IRequestable::IsAvailable())
				{
					SOCKET tempHandle = IRequestable::Socket();

					IRequestable::ResetHandle();

					closesocket(tempHandle);
				}
			}

			//AcceptEx�� ������ ��û�ϴ� ���� ������ ���� �ڵ鿡 ���� �ٸ� CompKey(Accept*)�� ��û�ϸ�
			//�翬�� �ϳ��� Accept�� �Ҹ��� �ȴ�. ���� Accept�� �ν��Ͻ��� �ϳ����� �ϰ�
			//Request �� �� ���� Overlapped ����ü�� �ٸ��� �ؼ� ��û�� ����.
			//Overlapped�� ����ؼ� ���⿡ transfer�� ������ �ȴ�.
			bool ConnectionListener::RequestAccept()
			{
				//void ConnectionListener::RequestProcessed(int receivedBytes, LPOVERLAPPED pOverlapped)����
				//while() ���������� ���ؼ� �Ѵ�.
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

					//handle�� NULL�̸� ���� ���� ���̴�. �׳� �����Ѵ�.
					if(result == TRUE)
					{
						Debug::Output(L"\r\n******************\r\n\r\n\r\nDid Direct Connected Check ConnectionListener\r\n\r\n\r\n******************\r\n");
					}
					else if(IRequestable::IsAvailable())
					{
						int errorCode = WSAGetLastError();

						//errorCode == WSAEINVAL�� ���� �߻��Ѵ�.
						//��ȿ���� ���� ���ڶ�� ���ε� socket�� close���� ���� ���̴�.
						//errorCode==ERROR_NETNAME_DELETED(64)�� ��쵵 �߻��Ѵٴµ� �� �𸣰ڴ�.
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

				//void ConnectionListener::Stop()���� closesocket�� �����ϸ� RequestProcessed�� ȣ��ȴ�.
				//handle==0 ������ ���� accept�� �Ȱ��� closesocket���� �� �� �ִ�.
				//handle==0 ���� �ٸ� ����� ������?
				if(IRequestable::IsAvailable() == false)
					return;

				transfer->ClientConnected();

				//�� �κ��� ���Ķ�.
				//false==RequestAccept()�� �Ǵ� �� closesocket�� �� �� transfer�� accept �ϱ� �����ε�
				//����ȭ ������ �� �� �ٵ��� �ʿ䰡 �ְڴ�.
				//�׷��� �ϴ� �̷��� �ص� ū ������ ���� ���δ�.
				while(false == RequestAccept())
				{
					Sleep(1000);
				}
			}
		}
	}
}
