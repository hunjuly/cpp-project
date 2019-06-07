#pragma once
#include "IRequestable.h"
#include "CriticalSection.h"

namespace CommonLib
{
	namespace Network
	{
		namespace Async
		{		
			class ITransferPool;

			class Transfer : public Io::Iocp::IRequestable
			{
			public: //IRequestable
				virtual void RequestProcessed(int receivedBytes, LPOVERLAPPED pOverlapped);

			public:
				void ClientConnected();
				tstring RemoteIp();
				tstring LocalIp();
				BYTE* SocketAddressBuffer();

				void Stop();
				int IdleTime();
				void Reset();

			protected:
				Transfer(ITransferPool& transferPool, Io::Iocp::RequestHandler& requester);
				virtual ~Transfer(void);

				void RecvRequest(BYTE* buffer, int size);
				void SendRequest(BYTE* buffer, int size);
				void SendRequest(const vector<BYTE>& buffer);
				void SendFinish();
				void CloseConnection();

			private:
				bool IsDisconnectedByClient();				
				void DoTransfer(BYTE* buffer, int requestSize, const function<int(WSABUF wsaBuf, int* transferSize)>& task);

				ITransferPool& transferPool;

				Threading::CriticalSection closeCs;

				bool isConnected;
				bool isDoingClose;

				BYTE socketAddrBuffer[(sizeof(sockaddr_in)+16)*2];
				LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockAddrs;
				void SockAddr(sockaddr** pRemoteAddr, sockaddr** pLocalAddr);

				enum class Task{
					Send
					,Recv
				} currentTask;

				ULONGLONG lastSignalTime;
				void RefreshLastSignalTime();

			protected:
				virtual void Disconnected()=0;
				virtual void Connected()=0;
				virtual void RequestProcessed(int transferredBytes)=0;
			};
		}
	}
}
