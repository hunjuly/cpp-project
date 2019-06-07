#include "stdafx.h"
#include "FFMpegQualityOption.h"

namespace FFMpeg
{
	void QualityOption::Update(int wantBitrate, int sourceWidth, int sourceHeight)
	{
		videoBitrate = wantBitrate;
		videoBitrateTolence = videoBitrate  * 1.2;
		audioBitrate = videoBitrate <= 1000 ? 64 : 128;
		audioSampleRate = videoBitrate <= 1000 ? 22050 : 44100;
		audioChannels = videoBitrate <= 500 ? 1 : 2;
		keyframe = 60;
		frameRate = 23.976;

		//profile�� ���ٰ� ������ �ƴϴ�. ��ȭ���� main ��ȭ���� high
		profile = (videoBitrate <= 3000) ? L"-profile : v main - level : v 4.0" : L"-profile:v high -level:v 4.1";

		//ultrafast, superfast, veryfast, faster, fast, medium, slow, slower, veryslow, placebo
		/*
		bufsize�� maxrate�� �����ؾ� �Ѵ�. �� �׷��� maxrate�� �ణ �����Ѵ�.
		crf�� 0�̸� maxrate�� �����Ѵ�.
		crf�� �ʹ� ������ ȭ���� ������. ������ �������� ã�ƾ� �Ѵ�.
		*/
		if (videoBitrate <= 500)
		{
			compress = L"fast";
			crf = 40;
			frameRate = 15;
			keyframe = 90;

			this->outputWidth = 640;
		}
		else if (videoBitrate <= 1000)
		{
			compress = L"faster";
			crf = 25;
			keyframe = 90;
			this->outputWidth = 960;
		}
		else if (videoBitrate <= 3000)
		{
			compress = L"veryfast";
			crf = 18;
			this->outputWidth = 1280;
		}
		else if (videoBitrate <= 5000)
		{
			compress = L"superfast";
			crf = 13;
			this->outputWidth = 1920;
		}
		else
		{
			compress = L"ultrafast";
			crf = 8;
			frameRate = 30.0;
			this->outputWidth = 1920;
		}

		this->outputHeight = (outputWidth * sourceHeight) / sourceWidth;

		if (this->outputHeight % 2 == 1)
		{
			this->outputHeight++;
		}
	}
}

/* profile by device
H.264 Main profile 3.1: iPad (all versions), Apple TV 2 and later, and iPhone 4 and later.
H.264 Main Profile 4.0: Apple TV 3 and later, iPad 2 and later, and iPhone 4S and later
H.264 High Profile 4.0: Apple TV 3 and later, iPad 2 and later, and iPhone 4S and later.
H.264 High Profile 4.1: iPad 2 and later and iPhone 4S and later.

https://developer.apple.com/library/mac/documentation/NetworkingInternet/Conceptual/StreamingMediaGuide/UsingHTTPLiveStreaming/UsingHTTPLiveStreaming.html
*/
