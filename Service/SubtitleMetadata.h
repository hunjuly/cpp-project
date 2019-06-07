#pragma once

class AVStream;

namespace MediaInformation
{
	class SubtitleMetadata
	{
	public:
		SubtitleMetadata(int index, AVStream *st);

		int index;
		std::wstring codecName;
		vector<BYTE> header;
		int type;
	};
}
