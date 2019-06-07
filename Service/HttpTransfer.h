#pragma once
#include "HttpResponse.h"
#include "HttpReceive.h"
#include "HttpBuffer.h"

using namespace Network::Async;

namespace Http
{
	class HttpTransfer : public Transfer
	{
	public:
		HttpTransfer(ITransferPool& transferPool, Io::Iocp::RequestHandler& requester);
		virtual ~HttpTransfer(void);

	protected:
		virtual void Connected();
		virtual void RequestProcessed(int transferredBytes);
		virtual void Disconnected();

	private:
		Http::Receive receive;
		Http::Response response;

		HttpBuffer headerBuffer;
		HttpBuffer postDataBuffer;

	private:
		enum class TaskType
		{
			RecvHeader
			, RecvPostData
			, RequestProcessing
			, SendResponse
			, SendResponsePostData
			, SendEnd
			, StandBy
		} currentTask;

		void ResponseToClient();

		void RequestRecvHeader();
		void RequestRecvPostData();
		void TryRequestSendData();
		void TryRequestSendPostData();
		void TryResponseToClient();
	};
}