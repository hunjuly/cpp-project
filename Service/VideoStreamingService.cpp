#include "stdafx.h"
#include "VideoStreamingService.h"
#include "HttpReceive.h"
#include "HttpResponse.h"

using namespace CommonLib::Utility;

namespace Service
{
	using namespace Streaming;

	bool VideoStreamingService::IsCanProcess(const tstring& serviceName)
	{
		return String::IsEqual(L"VideoStreaming", serviceName, true);
	}

	void VideoStreamingService::Uninitialize()
	{
		segmenterPool.RemoveAll();

		if (Path::IsExist(Config::Instance().StreamingTempFolder()))
		{
			Path::DeleteFolder(Config::Instance().StreamingTempFolder(), true);
		}
	}

	void VideoStreamingService::ProcessRequest(const Http::Receive& receive, Http::Response* response)
	{
		//절전모드 진입 금지
		if (Config::Instance().IgnorePowerSaveMode)
			::SetThreadExecutionState(ES_SYSTEM_REQUIRED);

		if (String::IsEqual(receive.urlPath.back(), L"PreparePlay.htm"))
			PreparePlay(receive, response);
		else if (String::IsEqual(receive.urlPath.back(), L"StopPlay.htm"))
			StopPlay(receive, response);
		else if (String::IsEqual(receive.serviceType, L".m3u8", true))
			ResponseM3u8(receive, response);
		else if (String::IsEqual(receive.serviceType, L".ts", true))
			ResponseTsFile(receive, response);
	}

	void VideoStreamingService::PreparePlay(const Http::Receive& receive, Http::Response* response)
	{
		tstring fullpath = receive.JsonData().Path(L"TargetPath");

		if (Path::IsExist(fullpath))
		{
			tstring segmenterId = segmenterPool.CreateSegmenter(receive.JsonData());

			Json::JsonMutableDictionary sendData;
			sendData.AddValue(L"SegmenterId", segmenterId);

			response->WithContent(sendData);
		}
		else
		{
			response->NotFound();
		}
	}

	void VideoStreamingService::StopPlay(const Http::Receive& receive, Http::Response* response)
	{
		tstring segmenterId = receive.JsonData().String(L"ClientId");

		if (segmenterPool.IsExistSegmenter(segmenterId))
		{
			segmenterPool.RemoveSegmenter(segmenterId);
		}
		else
		{
			response->NotFound();
		}
	}

	void VideoStreamingService::ResponseM3u8(const Http::Receive& receive, Http::Response* response)
	{
		if (receive.urlPath.size() == 4)
		{
			tstring deviceId = receive.urlPath[1];

			if (segmenterPool.IsExistSegmenter(deviceId))
			{
				segmenterPool.SendIndexList(deviceId, response, receive.urlPath[3]);
			}
			else
				response->NotFound();
		}
		else
		{
			response->NotFound();
		}
	}

	void VideoStreamingService::ResponseTsFile(const Http::Receive& receive, Http::Response* response)
	{
		if (receive.urlPath.size() == 4)
		{
			tstring deviceId = receive.urlPath[1];

			if (segmenterPool.IsExistSegmenter(deviceId))
			{
				segmenterPool.SendSegment(deviceId, response, receive.urlPath[3]);
			}
			else
				response->NotFound();
		}
		else
		{
			response->NotFound();
		}
	}
}
