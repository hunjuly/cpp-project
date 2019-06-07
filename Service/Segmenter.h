#pragma once
#include "Segment.h"
#include "FFMpegCommand.h"

namespace Http
{
	class Response;
}

using namespace CommonLib::Utility;

namespace Service
{
	namespace Streaming
	{
		class Segmenter
		{
		public:
			Segmenter(tstring deviceId, const FFMpegCommand& command);
			virtual ~Segmenter(void);

			const tstring& DeviceId();
			const tstring& SegmenterId();

			void SendSegment(Http::Response* response, const tstring& filename);
			void SendIndexList(Http::Response* response, const tstring& filename);

			bool IsTimeOver(int sec);

		private:
			DWORD lastUsedTime;
			DWORD LastUsedTimeSec();
			bool IsTimeRestarted();

		private:
			void StartEncoding(string ffmpegCommand);
			void StopEncoding();
			void DeleteDeviceFolder();

			string indexListMimeType;

			bool workingContinued;

			Threading::Thread<Segmenter> controlNumberOfSegmentThread;
			DWORD ControlNumberOfSegmentThread(LPVOID pThis);

			tstring deviceId;
			tstring segmenterId;

			System::Process process;

			int lastRequestedIndex;
			tstring OutputFolder();
			tstring DeviceFolder();
			void SetUniqueSegmenterId();
			void DeletePrevSegment();

			void WaitEnoughSegment();
			int NumberOfSegment();

			enum
			{
				SegmentListSize = 20
			};
		};
	}
}