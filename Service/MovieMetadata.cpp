#include "stdafx.h"
#include "MovieMetadata.h"
#include "FFMpeg.h"
#include "MediaInformation.h"

namespace MediaInformation
{
	MovieMetadata::MovieMetadata(const tstring& filename) :filename(filename)
	{
		static bool uninit = true;

		if(uninit == true)
		{
			av_register_all();
			uninit = false;
		}

		string shortName = String::WideCharToMultiByte(Utility::Path::ShortPathName(filename));

		AVFormatContext *ic = avformat_alloc_context();

		if(ic)
		{//nas 접속이 시원찮으면 여기서 멈춘다. 그러면 서버가 응답을 못하는 상황이 된다.
			if(avformat_open_input(&ic, shortName.c_str(), 0, NULL) >= 0)
			{
				if(avformat_find_stream_info(ic, NULL) >= 0)
				{
					UpdateMetadata(ic);
				}

				avformat_close_input(&ic);
				this->fileSize = Utility::Path::FileSize(filename);
			}
		}

		avformat_free_context(ic);
	}

	std::vector<BYTE> MovieMetadata::Thumbnail()
	{
		if(this->duration < 200)
			return MediaInformation::MovieThumbnail(filename, this->duration / 2);
		else
			return MediaInformation::MovieThumbnail(filename, 100);
	}

	void MovieMetadata::UpdateMetadata(AVFormatContext *ic)
	{
		this->bitrate = ic->bit_rate / 1024;
		this->duration = ic->duration / AV_TIME_BASE;
		ReadMetaData(ic->metadata, &description);

		for(int i = 0; i < ic->nb_streams; ++i)
		{
			if(ic->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				VideoMetadata video(i, ic->streams[i]);

				videos.push_back(video);
			}
		}

		for(int i = 0; i < ic->nb_streams; ++i)
		{
			if(ic->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				AudioMetadata audio(i, ic->streams[i]);

				audios.push_back(audio);
			}
		}

		for(int i = 0; i < ic->nb_streams; ++i)
		{
			if(ic->streams[i]->codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
			{
				SubtitleMetadata subtitle(i, ic->streams[i]);
				
				subtitles.push_back(subtitle);
			}
		}

		for(int i = 0; i < ic->nb_chapters; i++)
		{
			ChapterMetadata chapter(ic->chapters[i]);

			chapters.push_back(chapter);
		}

		if(videos.size() > 0)
			isAvailable = true;
	}

	Json::JsonDictionary MovieMetadata::ToJson()
	{
		Json::JsonMutableDictionary movieData;

		movieData.AddValue(L"isAvailable", isAvailable);
		movieData.AddValue(L"fileSize", String::StringFrom(fileSize));
		movieData.AddValue(L"bitrate", bitrate);
		movieData.AddValue(L"duration", duration);
		movieData.AddValue(L"description", description);

		Json::JsonMutableArray videoJson;

		for(const VideoMetadata& video : videos)
		{
			videoJson.AddValue(video.ToJson());
		}

		movieData.AddValue(L"videos", videoJson);


		Json::JsonMutableArray audioJson;

		for(const AudioMetadata& audio : audios)
		{
			audioJson.AddValue(audio.ToJson());
		}

		movieData.AddValue(L"audios", audioJson);


		Json::JsonMutableArray chapterJson;

		for(const ChapterMetadata& chapter : chapters)
		{
			chapterJson.AddValue(chapter.ToJson());
		}

		movieData.AddValue(L"chapters", chapterJson);

		return movieData;
	}
}