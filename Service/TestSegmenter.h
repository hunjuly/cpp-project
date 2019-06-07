#pragma once

namespace Http
{
	class Response;
}

namespace Service
{
	namespace Streaming
	{
		class TestSegmenter
		{
		public:
			TestSegmenter();
			virtual ~TestSegmenter();

			virtual void SendSegment(Http::Response* response, const tstring& filename);
			virtual void SendIndexList(Http::Response* response, const tstring& filename);
			virtual bool IsTimeOver(int sec);

		private:
			string GetFFMpegCommand();
			tstring OutputFolder();
			System::Process process;
		};
	}
}