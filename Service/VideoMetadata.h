#pragma once

class AVStream;

namespace MediaInformation
{
	class VideoMetadata
	{
	public:
		VideoMetadata(int index, AVStream *st);
		Utility::Json::JsonDictionary ToJson() const;

		int index;
		int width;
		int height;
		int bitrate;
		float frameRate;
		std::wstring format;
		std::wstring description;
	};
}