#pragma once
#include "IRequestable.h"
#include "RequestHandler.h"
#include "TransferPool.h"

namespace CommonLib
{
	namespace Network
	{
		namespace Async
		{
			class ConnectionListener : public Io::Iocp::IRequestable
			{
			public:
				ConnectionListener(ITransferPool& transferPool, int numberOfThreads);
				virtual ~ConnectionListener(void);

				virtual void RequestProcessed(int receivedBytes, LPOVERLAPPED pOverlapped);
				bool Start(int port, int backLog);
				void Stop();

			private:
				bool RequestAccept();
				void CloseSocket();

				ITransferPool& transferPool;
				LPFN_ACCEPTEX lpfnAcceptEx;
				Io::Iocp::RequestHandler* requester;
				
				Io::Iocp::RequestHandler& CreateRequesterHandler(int numberOfThreads);
			};
		}
	}
}
