#include "StdAfx.h"
#include "HttpTransfer.h"
#include "ServiceHandler.h"

using namespace Network;
using namespace Network::Async;

namespace Http
{
	HttpTransfer::HttpTransfer(ITransferPool& transferPool, Io::Iocp::RequestHandler& requester) :Transfer(transferPool, requester)
	{
		currentTask = TaskType::StandBy;
	}

	HttpTransfer::~HttpTransfer(void)
	{}

	void HttpTransfer::Connected()
	{
		headerBuffer.Reset(1024 * 4);

		receive.Reset();

		RequestRecvHeader();
	}

	void HttpTransfer::Disconnected()
	{
		currentTask = TaskType::SendEnd;
	}

	void HttpTransfer::RequestProcessed(int transferredBytes)
	{
		if(currentTask == TaskType::RecvHeader)
		{
			headerBuffer.IncreaseWritePos(transferredBytes);

			while(headerBuffer.HasLine())
			{
				receive.AddHeader(headerBuffer.GetLineAndRemove());

				if(receive.HasHeader())
					break;
			}

			if(receive.HasHeader())
			{
				if(receive.contentLength > 0)
				{
					postDataBuffer.Reset(receive.contentLength);

					//headerBuffer에서 남은 걸 postBuffer로 옮긴다.
					postDataBuffer.AddBuffer(headerBuffer);

					TryResponseToClient();
				}
				else
					ResponseToClient();
			}
			else
				RequestRecvHeader();
		}
		else if(currentTask == TaskType::RecvPostData)
		{
			postDataBuffer.IncreaseWritePos(transferredBytes);

			TryResponseToClient();
		}
		else if(currentTask == TaskType::SendResponse)
		{
			response.data.IncreaseReadPos(transferredBytes);

			TryRequestSendData();
		}
		else if(currentTask == TaskType::SendResponsePostData)
		{
			response.data.IncreaseReadPostDataPos(transferredBytes);

			TryRequestSendPostData();
		}
	}

	void HttpTransfer::RequestRecvHeader()
	{
		currentTask = TaskType::RecvHeader;
		Transfer::RecvRequest(headerBuffer.CurrentWritePosition(), headerBuffer.RemainedWriteSize());
	}

	void HttpTransfer::RequestRecvPostData()
	{
		currentTask = TaskType::RecvPostData;
		Transfer::RecvRequest(postDataBuffer.CurrentWritePosition(), postDataBuffer.RemainedWriteSize());
	}

	void HttpTransfer::TryRequestSendData()
	{
		if(response.data.RemainedReadSize() > 0)
		{
			currentTask = TaskType::SendResponse;
			Transfer::SendRequest(response.data.CurrentReadPosition(), response.data.RemainedReadSize());
		}
		else
			TryRequestSendPostData();
	}

	void HttpTransfer::TryRequestSendPostData()
	{
		if(response.data.RemainedReadPostDataSize() > 0)
		{
			auto buffer = response.data.CurrentReadPostDataPosition();

			//버퍼가 0이라면 파일이 삭제되는 등 읽을 수 없는 상황이다.
			//그냥 종료한다.
			if(0 < buffer.size())
			{
				currentTask = TaskType::SendResponsePostData;
				Transfer::SendRequest(buffer);
			}
			else
			{
				response.data.Close();
				SendFinish();
			}
		}
		else
		{
			response.data.Close();
			SendFinish();
		}
	}

	void HttpTransfer::TryResponseToClient()
	{
		if(postDataBuffer.RemainedWriteSize() == 0)
		{
			receive.SetPostData(postDataBuffer.GetData());

			ResponseToClient();
		}
		else
			RequestRecvPostData();
	}

	void HttpTransfer::ResponseToClient()
	{
		receive.Log();

		response.Reset();

		currentTask = TaskType::RequestProcessing;

		Service::ServiceHandler::Instance().ProcessRequest(receive, &response);

		response.Log();

		TryRequestSendData();
	}
}