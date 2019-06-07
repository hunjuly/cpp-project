#include "stdafx.h"
#include "SegmenterPool.h"
#include "Segmenter.h"
#include "FFMpegCommand.h"
#include "TestSegmenter.h"

//#define _TEST //For Apple test

namespace Service
{
	namespace Streaming
	{
		SegmenterPool::~SegmenterPool()
		{
			_ASSERT_EXPR(segmenters.size() == 0, L"You must execute RemoveAll() before Terminate");

			activityTimethread.WaitStop(INFINITE);
		}

		tstring SegmenterPool::CreateSegmenter(const Json::JsonDictionary& jsonData)
		{
			tstring segmenterId;

			cs.SafeLock([&]()
			{
				tstring deviceId = jsonData.String(L"ClientId");

				if(IsExistSegmenter(deviceId))
					RemoveSegmenter(deviceId);

				FFMpegCommand command(jsonData);

				Segmenter* segmenter = new Segmenter(deviceId, command);

				segmenters.insert(pair<tstring, Segmenter*>(segmenter->DeviceId(), segmenter));

				if(activityTimethread.IsRunning() == false)
				{
					activityTimethread.Execute(&SegmenterPool::ActivityTimethread, this, 0);
				}

				segmenterId = segmenter->SegmenterId();
			});

			return segmenterId;
		}

		bool SegmenterPool::IsExistSegmenter(tstring deviceId)
		{
#ifdef _TEST
			if(clientId == L"Test")
				return true;
#endif

			bool exist = false;

			cs.SafeLock([&]()
			{
				auto itr = segmenters.find(deviceId);

				exist = itr != segmenters.end();
			});

			return exist;
		}

		void SegmenterPool::RemoveSegmenter(tstring deviceId)
		{
			cs.SafeLock([&]()
			{
				auto itr = segmenters.find(deviceId);

				if(itr != segmenters.end())
				{
					delete itr->second;

					segmenters.erase(itr);
				}
			});
		}

		void SegmenterPool::RemoveAll()
		{
			cs.SafeLock([&]()
			{
				for(pair<tstring, Segmenter*> itr : segmenters)
				{
					delete itr.second;
				}

				segmenters.clear();
			});
		}

		DWORD SegmenterPool::ActivityTimethread(LPVOID pThis)
		{
			while(true)
			{
				cs.SafeLock([&]()
				{
					auto itr = segmenters.begin();

					while(itr != segmenters.end())
					{
						if(itr->second->IsTimeOver(5 * 60))
						{
							Segmenter* segmenter = itr->second;

							segmenters.erase(itr);
							delete segmenter;

							itr = segmenters.begin();
						}
						else
							itr++;
					}
				});

				if(segmenters.size() == 0)
				{
					break;
				}

				Sleep(1000);
			}

			return 0;
		}

#ifdef _TEST
		static TestSegmenter testSegmenter;
#endif

		void SegmenterPool::SendSegment(tstring deviceId, Http::Response* response, const tstring& filename)
		{
#ifdef _TEST
			if(clientId == L"Test")
			{
				testSegmenter.SendSegment(response, filename);
				return;
			}
#endif
			cs.SafeLock([&]()
			{
				auto itr = segmenters.find(deviceId);

				if(itr != segmenters.end())
				{
					Segmenter* segmenter = itr->second;
					segmenter->SendSegment(response, filename);
				}
			});
		}

		void SegmenterPool::SendIndexList(tstring deviceId, Http::Response* response, const tstring& filename)
		{
#ifdef _TEST
			if(clientId == L"Test")
			{
				testSegmenter.SendIndexList(response, filename);
				return;
			}
#endif
			cs.SafeLock([&]()
			{
				auto itr = segmenters.find(deviceId);

				if(itr != segmenters.end())
				{
					Segmenter* segmenter = itr->second;
					segmenter->SendIndexList(response, filename);
				}
			});
		}
	}
}