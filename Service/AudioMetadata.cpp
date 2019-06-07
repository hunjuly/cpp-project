#include "stdafx.h"
#include "AudioMetadata.h"
#include "FFMpeg.h"

namespace MediaInformation
{
	AudioMetadata::AudioMetadata(int index, AVStream *st)
	{
		this->index = index;
		bitrate = st->codec->bit_rate;
		samplerate = st->codec->sample_rate;
		channels = st->codec->channels;

		AVCodec* dec = avcodec_find_decoder(st->codec->codec_id);
		format = String::MultiByteToWideChar(dec->name);

		ReadMetaData(st->metadata, &description);

		if (description.length() == 0)
		{
			description = String::Format(L"Audio(%d)", index);
		}
	}

	Utility::Json::JsonDictionary AudioMetadata::ToJson() const
	{
		Utility::Json::JsonMutableDictionary json;

		json.AddValue(L"index", index);
		json.AddValue(L"samplerate", samplerate);
		json.AddValue(L"channels", channels);
		json.AddValue(L"bitrate", bitrate);
		json.AddValue(L"format", format);
		json.AddValue(L"description", description);

		return json;
	}
}
