#pragma once

using namespace Utility;

namespace FFMpeg
{
	class QualityOption
	{
	public:
		void Update(int wantBitrate, int sourceWidth, int sourceHeight);

		int outputWidth = 0;
		int outputHeight = 0;
		tstring profile = L"";
		int crf = 0;
		float frameRate = 0;
		int audioBitrate = 0;
		int audioSampleRate = 0;
		int audioChannels = 0;
		int videoBitrate = 0;
		int videoBitrateTolence = 0;
		int keyframe = 0;
		LPCTSTR compress = L"";
	};
}