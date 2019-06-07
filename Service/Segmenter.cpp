#include "StdAfx.h"
#include "Segmenter.h"
#include "HttpResponse.h"
#include "Config.h"
#include "ProcessPool.h"

namespace Service
{
	namespace Streaming
	{
		Segmenter::Segmenter(tstring deviceId, const FFMpegCommand& command)
		{
			this->deviceId = deviceId;
			
			SetUniqueSegmenterId();

			indexListMimeType = command.IsAudioOnly() ? "audio/x-mpegurl" : "application/vnd.apple.mpegurl";

			StartEncoding(command.Get(OutputFolder(), SegmentListSize));

			WaitEnoughSegment();

			_ASSERT(workingContinued);
		}

		Segmenter::~Segmenter(void)
		{
			StopEncoding();
		}

		void Segmenter::DeleteDeviceFolder()
		{
			if(Path::IsExist(DeviceFolder()))
			{
				Path::DeleteFolder(DeviceFolder(), true);
			}
		}

		void Segmenter::StartEncoding(string ffmpegCommand)
		{
			workingContinued = true;

			lastRequestedIndex = 0;
			lastUsedTime = ::timeGetTime();

			DeleteDeviceFolder();

			Path::CreateFolder(OutputFolder(), true);

			process.Start(ffmpegCommand);
			ProcessPool::Instance().AddProcess(process.Handle());

			controlNumberOfSegmentThread.Execute(&Segmenter::ControlNumberOfSegmentThread, this, 0);
		}

		void Segmenter::SetUniqueSegmenterId()
		{
			do
			{
				segmenterId = CreateUuid();
			} while(Path::IsExist(OutputFolder()));
		}

		void Segmenter::StopEncoding()
		{
			_ASSERT(workingContinued);

			workingContinued = false;

			process.Stop();
			ProcessPool::Instance().RemoveProcess(process.Handle());

			controlNumberOfSegmentThread.WaitStop(INFINITE);

			DeleteDeviceFolder();
		}

		int Segmenter::NumberOfSegment()
		{
			LPCWSTR extList[] = {L"ts"};

			return Path::FileCount(OutputFolder(), extList, 1);
		}

		void Segmenter::WaitEnoughSegment()
		{
			//Index.m3u8이 비어있는 상태에서 리턴하면 iOS에서 멈춘다.
			//그래서 ts 파일이 4개 이상이면 서비스를 시작한다.(3개는 정상, 1개는 생성중 그래서 4개)
			int count = 0;

			while(NumberOfSegment() <= 4)
			{
				count++;

				if(count == 50) // count 10sec
					break;

				Sleep(100);
			}
		}

		DWORD Segmenter::ControlNumberOfSegmentThread(LPVOID pThis)
		{
			auto wildPath = Path::Combine(OutputFolder(), L"*.ts");

			while(workingContinued)
			{
				LPCWSTR extList[] = {L"ts"};

				if(SegmentListSize < Path::FileCount(wildPath))
				{
					process.Pause();

					Sleep(5000);
				}
				else {
					process.Resume();

					Sleep(1000);
				}
			}

			return 0;
		}

		const tstring& Segmenter::DeviceId()
		{
			return deviceId;
		}

		const tstring& Segmenter::SegmenterId()
		{
			return segmenterId;
		}

		void Segmenter::SendSegment(Http::Response* response, const tstring& filename)
		{
			lastUsedTime = ::timeGetTime();

			lastRequestedIndex = String::MakeInt(filename);

			tstring path = Path::Combine(OutputFolder(), filename);

			if(Path::IsExist(path))
			{
				Io::File::FileReader reader(path);

				response->WithContent("video/MP2T", reader.ReadToEnd());
			}
			else
			{
				response->NotFound();
			}

			DeletePrevSegment();
		}

		void Segmenter::DeletePrevSegment()
		{
			tstring prevFileName = String::Format(L"%s\\%05d.ts", OutputFolder().c_str(), lastRequestedIndex - 15);

			if(Path::IsExist(prevFileName))
			{
				Path::DeleteFile(prevFileName);
			}
		}

		void Segmenter::SendIndexList(Http::Response* response, const tstring& filename)
		{
			lastUsedTime = ::timeGetTime();

			tstring path = Path::Combine(OutputFolder(), filename);

			if(Path::IsExist(path))
			{
				Io::File::FileReader reader(path);

				response->WithContent(indexListMimeType.c_str(), reader.ReadToEnd());
			}
			else
			{
				response->NotFound();
			}
		}

		bool Segmenter::IsTimeOver(int sec)
		{
			return sec < LastUsedTimeSec();
		}

		DWORD Segmenter::LastUsedTimeSec()
		{
			_ASSERT(lastUsedTime > 0);

			DWORD result = ::timeGetTime() - lastUsedTime;

			if(IsTimeRestarted())
				result = (::timeGetTime() + (ULONG_MAX - lastUsedTime));

			return result / 1000;
		}

		bool Segmenter::IsTimeRestarted()
		{
			return lastUsedTime > ::timeGetTime();
		}
		
		tstring Segmenter::DeviceFolder()
		{
			return Path::Combine(Config::Instance().StreamingTempFolder(), deviceId);
		}

		tstring Segmenter::OutputFolder()
		{
			return Path::Combine(DeviceFolder(), segmenterId);
		}
	}
}
