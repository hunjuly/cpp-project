#include "stdafx.h"
#include "ChapterMetadata.h"
#include "FFMpeg.h"

namespace MediaInformation
{
	ChapterMetadata::ChapterMetadata(AVChapter *ch)
	{
		start = ch->start * av_q2d(ch->time_base);
		end = ch->end   * av_q2d(ch->time_base);

		ReadMetaData(ch->metadata, &description);
	}

	Utility::Json::JsonDictionary ChapterMetadata::ToJson() const
	{
		Utility::Json::JsonMutableDictionary json;

		json.AddValue(L"start", String::StringFrom(start));
		json.AddValue(L"end", String::StringFrom(end));
		json.AddValue(L"description", description);

		return json;
	}
}