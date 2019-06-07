#pragma once

class AVStream;

namespace MediaInformation
{
	class AudioMetadata
	{
	public:
		AudioMetadata(int index, AVStream *st);
		Utility::Json::JsonDictionary ToJson() const;

		int index;
		int samplerate;
		int channels;
		int bitrate;
		std::wstring format;
		std::wstring description;
	};
}
