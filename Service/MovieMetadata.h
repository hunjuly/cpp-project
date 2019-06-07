#pragma once
#include "VideoMetadata.h"
#include "AudioMetadata.h"
#include "SubtitleMetadata.h"
#include "ChapterMetadata.h"

class AVFormatContext;

using namespace CommonLib::Utility;

namespace MediaInformation
{
	class MovieMetadata
	{
	public:
		MovieMetadata(const tstring& filename);
		
		Json::JsonDictionary ToJson();
		std::vector<BYTE> Thumbnail();

		const tstring filename;
		bool isAvailable = false;
		long long fileSize = 0;
		int bitrate = 0;
		int duration = 0;
		std::wstring description;

		std::vector<VideoMetadata> videos;
		std::vector<AudioMetadata> audios;
		std::vector<SubtitleMetadata> subtitles;
		std::vector<ChapterMetadata> chapters;

		void UpdateMetadata(AVFormatContext *ic);
	};
}