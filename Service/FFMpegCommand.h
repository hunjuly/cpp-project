#pragma once
#include "FFMpegQualityOption.h"

using namespace Utility;

class FFMpegCommand
{
public:
	FFMpegCommand(const Json::JsonDictionary& recvData);

	string Get(tstring outputFolder, int segmentListSize) const;
	bool IsAudioOnly() const;

private:
	tstring sourceFile;
	int videoIndex;
	int audioIndex;
	int startTime;

	FFMpeg::QualityOption option;
};

