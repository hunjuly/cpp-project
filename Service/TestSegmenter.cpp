#include "stdafx.h"
#include "TestSegmenter.h"
#include "FFMpegCommand.h"
#include "ProcessPool.h"
#include "HttpResponse.h"

using namespace CommonLib::Utility;

namespace Service
{
	namespace Streaming
	{
		//mediastreamvalidator -t 60 http://192.168.1.11:4978/VideoStreaming/Test/index.m3u8
		//mediastreamvalidator -p http://192.168.1.11:4978/VideoStreaming/Test/index.m3u8

		TestSegmenter::TestSegmenter()
		{
			if(Path::IsExist(OutputFolder()) == false)
			{
				Path::CreateFolder(OutputFolder(), true);
			}

			process.Start(GetFFMpegCommand());
		}

		TestSegmenter::~TestSegmenter()
		{
			process.Stop();
			ProcessPool::Instance().RemoveProcess(process.Handle());

			if(Path::IsExist(OutputFolder()))
			{
				Path::DeleteFolder(OutputFolder(), true);
			}
		}

		void TestSegmenter::SendIndexList(Http::Response* response, const tstring& filename)
		{
			tstring path = Path::Combine(OutputFolder(), filename);

			if(Path::IsExist(path))
			{
				Io::File::FileReader reader(path);
				vector<BYTE> buffer = reader.ReadToEnd();
				buffer.erase(buffer.begin() + 49, buffer.begin() + 71);

				response->WithContent("application/vnd.apple.mpegurl", buffer);
			}
			else
			{
				response->NotFound();
			}
		}

		void TestSegmenter::SendSegment(Http::Response* response, const tstring& filename)
		{
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
		}

		bool TestSegmenter::IsTimeOver(int sec)
		{
			return false;
		}

		string TestSegmenter::GetFFMpegCommand()
		{
			//커맨드를 하드코딩해서 리턴하도록 한다.
			assert(false);

			return "";
			//FFMpegCommand command(L"C:\\nGinTemp\\test.avi", 1000, 0, 1, 2000, 640, 480);

			//return command.Get(OutputFolder(),0);
		}

		tstring TestSegmenter::OutputFolder()
		{
			return Path::Combine(Config::Instance().StreamingTempFolder(), L"Test");
		}
	}
}