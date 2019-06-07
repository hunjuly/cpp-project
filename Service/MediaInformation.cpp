#include "stdafx.h"
#include "MediaInformation.h"
#include "FFMpeg.h"

using namespace CommonLib::Utility;

namespace MediaInformation
{
	vector<tstring> MovieFileList(const tstring& requestedFolder)
	{
		LPCWSTR videoFormatList[] = {
			L"avi", L"wmv", L"wmp", L"wm", L"asf", L"mpg", L"mpeg", L"mpe"
			, L"m1v", L"m2v", L"mpv2", L"mp2v", L"ts", L"tp", L"tpr", L"trp", L"ifo", L"ogm"
			, L"ogv", L"mp4", L"m4v", L"m4p", L"m4b", L"3gp", L"3gpp", L"3g2", L"3gp2", L"mkv", L"rm", L"ram"
			, L"rmvb", L"rpm", L"flv", L"swf", L"mov", L"qt", L"amr", L"nsv", L"dpg", L"m2ts", L"m2t"
			, L"mts", L"dvr-ms", L"k3g", L"skm", L"evo", L"nsr", L"amv", L"divx", L"webm", L"axv", L"dv", L"f4v" };

		return Path::FileList(requestedFolder, videoFormatList, _countof(videoFormatList));
	}

	vector<BYTE> MovieThumbnail(const tstring& fileName, float time)
	{
		tstring command = String::Format(L"-ss %f -i \"%s\" -f image2 -vcodec mjpeg -vframes 1 -s 140x100 -"
			, time, Path::ShortPathName(fileName).c_str());

		return CommandResult(command);
	}


	//ffmpeg에 종속된다. 위치 바꿔라
	//text 타입은 ffmpeg에서 모두 srt로 변환해서 출력한다.
	bool IsImageSubtitleType(int code)
	{
		//text 타입은 ffmpeg에서 모두 srt로 변환해서 출력한다.
		switch (code)
		{
		case AV_CODEC_ID_DVD_SUBTITLE:
		case AV_CODEC_ID_DVB_SUBTITLE:
		case AV_CODEC_ID_HDMV_PGS_SUBTITLE:
			return true;
		}

		return false;
	}

	vector<tstring> SubtitleExtensions(tstring type)
	{
		if (String::IsEqual(type, L"VOBSUB"))
			return{ L"IDX", L"SUB" };

		return{ type };
	}

	void ReadFileSubtitleHeader(vector<SubtitleHeader>& subtitleList, tstring movieFile, tstring type)
	{
		auto extensions = SubtitleExtensions(type);

		for (const auto& extension : extensions)
		{
			if (Path::IsExist(Path::ReplaceExtension(movieFile, extension)) == false)
				return;
		}

		int dataSize = 0;

		for (const auto& extension : extensions)
		{
			const auto filename = Path::ReplaceExtension(movieFile, extension);

			dataSize += Path::FileSize(filename);
		}

		//파일이 2개 이상이면 제일 앞에 각 파일의 크기를 적어준다.
		if (1 < extensions.size())
		{
			dataSize += extensions.size() * 4;
		}

		SubtitleHeader header;
		header.type = type;
		header.isEmbedded = false;
		header.embeddedIndex = -1;
		header.dataPosition = 0;
		header.dataSize = dataSize;

		subtitleList.push_back(header);
	}

	void ReadEmbeddedSubtitleHeader(vector<SubtitleHeader>& subtitleList, const MovieMetadata& movieData)
	{
		for (const auto& subtitle : movieData.subtitles)
		{
			SubtitleHeader header;
			header.isEmbedded = true;
			header.embeddedIndex = subtitle.index;
			header.type = IsImageSubtitleType(subtitle.type) ? L"DVDSUB" : L"SRT";
			header.dataPosition = 0;
			header.dataSize = -1;

			subtitleList.push_back(header);
		}
	}

	vector<SubtitleHeader> SubtitleList(const tstring& movieFile, const MovieMetadata& movieData)
	{
		vector<SubtitleHeader> subtitleList;

		ReadFileSubtitleHeader(subtitleList, movieFile, L"SMI");
		ReadFileSubtitleHeader(subtitleList, movieFile, L"SRT");
		ReadFileSubtitleHeader(subtitleList, movieFile, L"ASS");
		ReadFileSubtitleHeader(subtitleList, movieFile, L"SSA");
		ReadFileSubtitleHeader(subtitleList, movieFile, L"VOBSUB");

		//네트워크 파일은 자막을 추출하는 데 너무 많은 시간이 걸린다. 사용자에게 오류로 인식될 수 있다.
		//if (movieFile[0] != '/' && movieFile[0] != '\\')
		//	ReadEmbeddedSubtitleHeader(subtitleList, movieData);

		return move(subtitleList);
	}

	vector<SubtitleHeader> SubtitleList(const tstring& movieFile)
	{
		auto metadata = MovieMetadata(movieFile);

		return move(SubtitleList(movieFile, metadata));
	}


	struct EmbeddedSubtitle
	{
		tstring fileName;
		int index;
		bool isComplete;
		tstring type;
		vector<BYTE> result;
	};

	static vector<EmbeddedSubtitle> cachedSubtitles;

	DWORD WINAPI SubtitleExtractThread(LPVOID pThis);

	vector<BYTE> ReadEmbeddedSubtitle(const tstring& movieFile, tstring type, int embeddedIndex)
	{
		for (auto& subtitle : cachedSubtitles)
		{
			if (subtitle.fileName == movieFile && subtitle.index == embeddedIndex)
			{
				if (subtitle.isComplete)
				{
					return subtitle.result;
				}
				else
				{
					return vector<BYTE>();
				}
			}
		}

		if (5 < cachedSubtitles.size())
		{
			cachedSubtitles.erase(cachedSubtitles.begin());
		}

		//발견하지 못했다면 새로 등록해야지
		EmbeddedSubtitle newSubtitle;
		newSubtitle.fileName = movieFile;
		newSubtitle.index = embeddedIndex;
		newSubtitle.type = type;
		newSubtitle.isComplete = false;

		//동기화 문제 가능성이 있다. 높지는 않을 듯
		cachedSubtitles.push_back(newSubtitle);

		DWORD  dwThreadId = 0;
		HANDLE hThread = CreateThread(NULL, 0, SubtitleExtractThread, 0, 0, &dwThreadId);

		return vector<BYTE>();
	}

	DWORD WINAPI SubtitleExtractThread(LPVOID pThis)
	{
		try {
			EmbeddedSubtitle& newSubtitle = cachedSubtitles.back();

			tstring outputFile = Config::Instance().StreamingTempFolder() + L"\\" + Utility::CreateUuid() +
				(String::IsEqual(newSubtitle.type, L"SRT") ? L".srt" : L".vob");

			Io::File::FileWriter writer(outputFile);
			writer.Close();

			tstring command = String::Format(L"-y -i \"%s\" -an -vn -map 0:%d -c:s:0 %s %s"
				, Path::ShortPathName(newSubtitle.fileName).c_str()
				, newSubtitle.index
				, (String::IsEqual(newSubtitle.type, L"SRT") ? L"srt" : L"dvd_subtitle")
				, Path::ShortPathName(outputFile).c_str());

			bool success = ExecuteFFMpeg(command);

			vector<BYTE> data;

			auto metadata = MovieMetadata(newSubtitle.fileName);

			for (const auto& subtitle : metadata.subtitles)
			{
				if (subtitle.index == newSubtitle.index)
				{
					if (0 < subtitle.header.size())
					{
						data.resize(2 * 4); //header하고 data하고 2개 넣을 거니까

						copy(subtitle.header.begin(), subtitle.header.end(), back_inserter(data));
						*((int*)(&data[0])) = subtitle.header.size();
					}

					Io::File::FileReader reader(outputFile);

					auto fileData = reader.ReadToEnd();

					reader.Close();

					copy(fileData.begin(), fileData.end(), back_inserter(data));

					if (0 < subtitle.header.size())
					{
						*((int*)(&data[4])) = fileData.size();
					}

					newSubtitle.isComplete = true;
					newSubtitle.result = move(data);
				}
			}

			Path::DeleteFile(outputFile);
		}
		catch (...) {}

		return 0;
	}

	vector<BYTE> ReadFileSubtitle(const tstring& movieFile, tstring type)
	{
		vector<BYTE> data;

		auto extensions = SubtitleExtensions(type);

		if (1 < extensions.size())
		{
			data.resize(extensions.size() * 4);
		}

		for (int i = 0; i < extensions.size(); ++i)
		{
			const auto& extension = extensions[i];

			const auto filename = Path::ReplaceExtension(movieFile, extension);

			Io::File::FileReader reader(filename);

			auto fileData = reader.ReadToEnd();

			reader.Close();

			int dataLastPosition = data.size();

			data.resize(data.size() + fileData.size());

			memcpy(&data[dataLastPosition], &fileData[0], fileData.size());

			if (1 < extensions.size())
			{
				*((int*)(&data[i * 4])) = fileData.size();
			}
		}

		return data;
	}

	vector<BYTE> SubtitleData(const tstring& movieFile, tstring type, bool isEmbedded, int embeddedIndex)
	{
		return move(isEmbedded ? ReadEmbeddedSubtitle(movieFile, type, embeddedIndex) : ReadFileSubtitle(movieFile, type));
	}

	Utility::Json::JsonArray SubtitleListToJson(const vector<MediaInformation::SubtitleHeader>& subtitleList)
	{
		Json::JsonMutableArray jsonArray;

		for (const auto& subtitle : subtitleList)
		{
			Json::JsonMutableDictionary dic;
			dic.AddValue(L"type", subtitle.type);
			dic.AddValue(L"isEmbedded", subtitle.isEmbedded);
			dic.AddValue(L"embeddedIndex", subtitle.embeddedIndex);
			dic.AddValue(L"dataPosition", subtitle.dataPosition);
			dic.AddValue(L"dataSize", subtitle.dataSize);
			jsonArray.AddValue(dic);
		}

		return jsonArray;
	}
}
