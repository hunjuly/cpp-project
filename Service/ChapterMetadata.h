#pragma once

class AVChapter;

namespace MediaInformation
{
	class ChapterMetadata
	{
	public:
		ChapterMetadata(AVChapter *ch);
		Utility::Json::JsonDictionary ToJson() const;

	private:
		int64_t start;
		int64_t end;
		std::wstring description;
	};
}