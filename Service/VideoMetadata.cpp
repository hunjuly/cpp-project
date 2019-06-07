#include "stdafx.h"
#include "VideoMetadata.h"
#include "FFMpeg.h"

namespace MediaInformation
{
	VideoMetadata::VideoMetadata(int index, AVStream *st)
	{
		AVRational display_aspect_ratio;

		av_reduce(&display_aspect_ratio.num, &display_aspect_ratio.den,
				  st->codec->width*st->sample_aspect_ratio.num,
				  st->codec->height*st->sample_aspect_ratio.den,
				  1024 * 1024);

		bitrate=st->codec->bit_rate;

		if(display_aspect_ratio.num > 0 && display_aspect_ratio.den > 0)
		{
			width = (float)st->codec->height*(float)display_aspect_ratio.num / (float)display_aspect_ratio.den;
			if(width % 2 == 1)
				width++;
		}
		//어떤 경우에는 display_aspect_ratio.num>0&&display_aspect_ratio.den>0 조건이 false인데 비율이 깨진다.
		//그건 아래 조건으로 처리한다.
		else if(st->codec->sample_aspect_ratio.num > 0 && st->codec->sample_aspect_ratio.den > 0)
		{
			width = (double)st->codec->width*((double)st->codec->sample_aspect_ratio.num / (double)st->codec->sample_aspect_ratio.den);

			if(width % 2 == 1)
				width++;
		}
		else
			width = st->codec->width;

		this->index = index;
		height = st->codec->height;

		frameRate = (float)st->avg_frame_rate.num / (float)st->avg_frame_rate.den;
		
		if(isnan(frameRate))
			frameRate = 0;

		AVCodec* dec = avcodec_find_decoder(st->codec->codec_id);
		format = String::MultiByteToWideChar(dec->name);

		ReadMetaData(st->metadata, &description);
	}

	Utility::Json::JsonDictionary VideoMetadata::ToJson() const
	{
		Utility::Json::JsonMutableDictionary json;

		json.AddValue(L"index", index);
		json.AddValue(L"width", width);
		json.AddValue(L"height", height);
		json.AddValue(L"bitrate", bitrate);
		json.AddValue(L"frameRate", frameRate);
		json.AddValue(L"format", format);
		json.AddValue(L"description", description);

		return json;
	}
}
