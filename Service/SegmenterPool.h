#pragma once
#include "Segmenter.h"

namespace Service
{
	namespace Streaming
	{
		class SegmenterPool
		{
		public:
			tstring CreateSegmenter(const Json::JsonDictionary& jsonData);
			void RemoveSegmenter(tstring deviceId);
			void RemoveAll();
			bool IsExistSegmenter(tstring deviceId);
			~SegmenterPool();

			void SendSegment(tstring deviceId, Http::Response* response, const tstring& filename);
			void SendIndexList(tstring deviceId, Http::Response* response, const tstring& filename);

		private:
			std::map<tstring, Segmenter*> segmenters;
			Threading::CriticalSection cs;

			Threading::Thread<SegmenterPool> activityTimethread;
			DWORD ActivityTimethread(LPVOID pThis);
		};
	}
}