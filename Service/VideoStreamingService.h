#pragma once
#include "iservice.h"
#include "SegmenterPool.h"

namespace Service
{	
	class VideoStreamingService :public IService
	{
	public:
		virtual bool IsCanProcess(const tstring& serviceName);
		virtual void ProcessRequest(const Http::Receive& receive, Http::Response* response);
		virtual void Uninitialize();

	private:
		void PreparePlay(const Http::Receive& receive, Http::Response* response);
		void ResponseM3u8(const Http::Receive& receive, Http::Response* response);
		void ResponseTsFile(const Http::Receive& receive, Http::Response* response);
		void StopPlay(const Http::Receive& receive, Http::Response* response);

		Streaming::SegmenterPool segmenterPool;
	};
}