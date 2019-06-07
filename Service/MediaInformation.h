#pragma once

#include "MovieMetadata.h"

namespace MediaInformation
{
	class SubtitleHeader
	{
	public:
		tstring type;
		bool isEmbedded;
		int embeddedIndex;
		int dataPosition;
		int dataSize;
	};

	vector<BYTE> MovieThumbnail(const tstring& fileName, float time);
	vector<tstring> MovieFileList(const tstring& requestedFolder);

	vector<SubtitleHeader> SubtitleList(const tstring& movieFile);
	vector<SubtitleHeader> SubtitleList(const tstring& movieFile, const MovieMetadata& movieData);
	vector<BYTE> SubtitleData(const tstring& movieFile, tstring type, bool isEmbedded, int embeddedIndex);
	Utility::Json::JsonArray SubtitleListToJson(const vector<MediaInformation::SubtitleHeader>& subtitleList);
}
