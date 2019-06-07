#include "stdafx.h"
#include "MovieEncoding.h"
#include "ProcessPool.h"
#include "MovieMetadata.h"

namespace Service
{
	using namespace Utility;

	MovieEncoding::~MovieEncoding()
	{
	}

	MovieEncoding::MovieEncoding(const Json::JsonDictionary& json
		, tstring folderName
		, tstring sourceFilePath
		, tstring tempfolderPath
		)
	{
		if (0 == folderName.length())
		{
			this->targetPath = json.Path(L"TargetPath");
		}
		else
		{
			this->targetPath = Path::Combine(json.Path(L"TargetPath"), folderName);
		}

		this->sourceFilePathName = json.IsExistKey(L"SourceFilePathName") ? json.String(L"SourceFilePathName") : sourceFilePath;
		this->tempfolderPath = tempfolderPath;
		this->downloadId = json.IsExistKey(L"DownloadId") ? json.String(L"DownloadId") : CreateDownloadId();
		this->targetBitrate = json.Integer(L"TargetBitrate");
		this->duration = 0;
		this->lastEncodedDuration = 0;

		if (IsEncoded() == false)
		{
			Path::DeleteFile(MovieFilePathName(), true);
			Path::DeleteFile(SubtitlesFilePathName(), true);
			Path::DeleteFile(CfgFilePathName(), true);
		}
	}

	void MovieEncoding::Update()
	{
		movieInfoUpdated = false;

		try {
			auto targetFilePath = IsEncoded() ? MovieFilePathName() : sourceFilePathName;

			//������ �����ϴ��� �̸� Ȯ���Ѵ�.
			//nasó�� ������ ���� ����� ������ ��� Ȯ���ϴ� �� 5�� �̻� �ɸ���.
			//�׷��� ��� ������ ������ �������� Ȯ�� �� ������ �����´�.
			if (false == Path::IsExist(targetFilePath))
				return;

			MediaInformation::MovieMetadata movieInfo(targetFilePath);

			if (movieInfo.isAvailable && movieInfo.videos.size() > 0)
			{
				duration = movieInfo.duration;

				option.Update(targetBitrate, movieInfo.videos[0].width, movieInfo.videos[0].height);

				thumbnail = movieInfo.Thumbnail();

				audioCount = movieInfo.audios.size();

				movieInfoUpdated = true;
			}
			else
			{
				duration = 0;
				targetBitrate = 0;
			}
		}
		catch (...) {}
	}

	tstring MovieEncoding::CreateDownloadId()
	{
		return Path::LastComponent(sourceFilePathName) + Utility::CreateUuid();
	}

	void MovieEncoding::StartEncoding()
	{
		workingContinued = true;

		assert((IsWaiting() || IsFailed()) && IsPrepared());

		controlEncodingThread.Execute(&MovieEncoding::ControlEncodingThread, this, 0);
	}

	void MovieEncoding::StopEncoding()
	{
		workingContinued = false;

		controlEncodingThread.WaitStop(INFINITE);

		process.Stop();
		ProcessPool::Instance().RemoveProcess(process.Handle());
	}

	string MovieEncoding::FFmpegCommand()
	{
		LPCTSTR format = LR"TEXT(%s\FFMpegnGinSoft.exe -y -i "%s" -map 0:v -map 0:a -f mov -c:a aac -strict experimental -ar %d -b:a %dk -ac %d -s %dx%d -c:v libx264 -pix_fmt yuv420p -preset %s -qp 0 -r %f -x264opts "crf=%d:vbv-bufsize=%d:vbv-maxrate=%d:keyint=%d:bframes=0" -deinterlace -threads %d "%s")TEXT";

		tstring ffmpegCommand = String::Format(format
			, Path::ShortPathName(Path::ModuleFilePath()).c_str()
			, Path::ShortPathName(sourceFilePathName).c_str()
			, option.audioSampleRate, option.audioBitrate, option.audioChannels
			, option.outputWidth, option.outputHeight, option.compress
			, option.frameRate, option.crf, option.videoBitrate, option.videoBitrate, option.keyframe
			, int(System::NumberOfCpus() / 2)
			, MovieFilePathName().c_str()
			);

		return String::WideCharToMultiByte(ffmpegCommand);
	}

	bool MovieEncoding::IsEqualDuration(int encodedDuration)
	{
		//���ڵ��� �������� duration�� �������� ª�ٸ� ���ڵ��� ������ ���̴�.
		//NAS�� �������� �����ϸ� ���� ������ �������µ� �� �� ������ ��Ÿ����.
		//5���� ������ ���ش�.

		auto diff = duration - encodedDuration;

		return diff < 5 && -5 < diff;
	}

	DWORD MovieEncoding::ControlEncodingThread(LPVOID pThis)
	{
		assert(IsEncoded() == false);
		assert(movieInfoUpdated);

		try
		{
			isEncoding = true;
			encodingFailed = false;

			process.Start(FFmpegCommand());
			ProcessPool::Instance().AddProcess(process.Handle());

			//Check FFMpeg Alive
			while (process.IsAlive())
			{
				if (workingContinued == false)
					return 0;

				if (encodingBytesPerSecList.size() > 30)
				{
					encodingBytesPerSecList.pop_front();
				}

				encodingBytesPerSecList.push_back(BytePerSec(CurrentSize()));

				Sleep(1000);
			}

			ProcessPool::Instance().RemoveProcess(process.Handle());

			if (process.ReturnCode() == 0)
			{
				auto targetData = MediaInformation::MovieMetadata(MovieFilePathName());

				bool isSuccess = false;

				if (IsEqualDuration(targetData.duration))
				{
					isSuccess = true;
				}
				else if (0 < targetData.duration)
				{
					//�� �� ���� ���� ��ġ���� ���д�. �������� �����Ѵ�.
					if (lastEncodedDuration == targetData.duration)
					{
						isSuccess = true;
					}
					else
					{
						//ù ��° ���д�. Ȥ�� ���� �ٸ� ��ġ���� ���д�. �ٽ� �õ��� ����.
						lastEncodedDuration = targetData.duration;

						encodingFailed = true;
						isCanRetry = true;
						isEncoding = false;
					}
				}
				else
				{
					encodingFailed = true;
					isCanRetry = false;
					isEncoding = false;
				}

				if (isSuccess)
				{
					vector<MediaInformation::SubtitleHeader> subtitleList = MediaInformation::SubtitleList(sourceFilePathName);

					CreateSubtitles(subtitleList);

					CreateCfg(subtitleList);

					encodingFailed = false;
					isCanRetry = false;
					isEncoding = false;
				}

				return 0;
			}
		}
		catch (...)
		{
		}

		Path::DeleteFile(MovieFilePathName());

		encodingFailed = true;
		isCanRetry = false;
		isEncoding = false;

		return 0;
	}

	int MovieEncoding::EncodingSpeed() const
	{
		if (IsEncoding() == false)
		{
			return 0;
		}

		if (encodingBytesPerSecList.size() == 0)
			return 0;

		if (encodingBytesPerSecList.size() == 1)
			return encodingBytesPerSecList.front().size;

		float total = encodingBytesPerSecList.back().size - encodingBytesPerSecList.front().size;
		float time = encodingBytesPerSecList.back().time - encodingBytesPerSecList.front().time;

		return total / (time / 1000.0);
	}

	void MovieEncoding::CreateSubtitles(vector<MediaInformation::SubtitleHeader>& subtitleList)
	{
		Io::File::FileWriter writer(SubtitlesFilePathName());

		int totalSize = 0;

		for (auto& subtitle : subtitleList)
		{
			subtitle.dataPosition = totalSize;

			//�Ϸᰡ �� ������ �ƴ��� ��� �Ƴ�
			auto data = MediaInformation::SubtitleData(sourceFilePathName, subtitle.type, subtitle.isEmbedded, subtitle.embeddedIndex);

			writer.Write(data);

			subtitle.isEmbedded = false;
			subtitle.dataSize = data.size();

			totalSize += data.size();
		}

		writer.Close();
	}

	void MovieEncoding::CreateCfg(const vector<MediaInformation::SubtitleHeader>& subtitleList)
	{
		if (Path::IsExist(MovieFilePathName()) == false)
			THROW_LIBRARY(L"not finished");

		MediaInformation::MovieMetadata movieInfo(MovieFilePathName());

		Utility::Json::JsonMutableDictionary json;

		json.AddValue(L"Filename", Path::RemoveExtension(Path::LastComponent(sourceFilePathName)));
		json.AddValue(L"MovieInfo", movieInfo.ToJson());
		json.AddValue(L"Thumbnail", movieInfo.Thumbnail());
		json.AddValue(L"TargetBitrate", option.videoBitrate);
		json.AddValue(L"SubtitleList", MediaInformation::SubtitleListToJson(subtitleList));

		Io::File::FileWriter writer(CfgFilePathName());
		writer.Write(json.ToString());
		writer.Close();
	}

	bool MovieEncoding::IsPrepared() const
	{
		return movieInfoUpdated;
	}

	bool MovieEncoding::IsEncoding() const
	{
		return isEncoding && movieInfoUpdated;
	}

	bool MovieEncoding::IsEncoded() const
	{
		return Path::IsExist(CfgFilePathName()) && Path::IsExist(MovieFilePathName()) && Path::IsExist(SubtitlesFilePathName());
	}

	bool MovieEncoding::IsWaiting() const
	{
		return IsEncoding() == false && IsEncoded() == false && movieInfoUpdated 
			&& ((IsFailed() && isCanRetry) || IsFailed() == false);
	}

	bool MovieEncoding::IsFailed() const
	{
		return movieInfoUpdated && encodingFailed;
	}

	bool MovieEncoding::IsEqual(tstring downloadId)
	{
		return this->downloadId == downloadId;
	}

	void MovieEncoding::Delete()
	{
		if (IsEncoding())
		{
			StopEncoding();
			return;
		}

		Path::DeleteFile(MovieFilePathName(), true);
		Path::DeleteFile(SubtitlesFilePathName(), true);
		Path::DeleteFile(CfgFilePathName(), true);
	}

	tstring MovieEncoding::Status() const
	{
		if (movieInfoUpdated == false)
		{
			return L"Initializing";
		}
		else if (encodingFailed)
		{
			return L"EncodingFail";
		}
		else if (IsEncoded())
		{
			return L"Encoded";
		}
		else if (IsEncoding())
		{
			return L"Encoding";
		}
		else if (IsWaiting())
		{
			return L"Waiting";
		}

		return L"Unknown";
	}

	long long MovieEncoding::TargetSize() const
	{
		if (IsEncoded())
			return Path::FileSize(MovieFilePathName());

		auto expectedSize = (long long)(((option.videoBitrate + audioCount * option.audioBitrate) / 8) * 1024) * duration;

		return max(Path::FileSize(MovieFilePathName()), expectedSize);
	}

	long long MovieEncoding::CurrentSize() const
	{
		if (Path::IsExist(MovieFilePathName()))
			return Path::FileSize(MovieFilePathName());

		return 0;
	}
	//���ڵ� �Ϸ��� ���� ����
	Json::JsonDictionary MovieEncoding::EncodingMovieInfo() const
	{
		Json::JsonMutableDictionary dic;

		dic.AddValue(L"ExpectedSize", String::StringFrom(TargetSize()));
		dic.AddValue(L"Thumbnail", thumbnail);
		dic.AddValue(L"VideoBitrate", option.videoBitrate);
		dic.AddValue(L"VideoWidth", option.outputWidth);
		dic.AddValue(L"VideoHeight", option.outputHeight);
		dic.AddValue(L"Duration", duration);
		dic.AddValue(L"Filename", Path::LastComponent(sourceFilePathName));

		return dic;
	}

	tstring MovieEncoding::MovieFilePathName() const
	{
		return Path::Combine(tempfolderPath, downloadId + L".mov");
	}
	tstring MovieEncoding::CfgFilePathName() const
	{
		return Path::Combine(tempfolderPath, downloadId + L".cfg");
	}
	tstring MovieEncoding::SubtitlesFilePathName() const
	{
		return Path::Combine(tempfolderPath, downloadId + L".sbs");
	}

	Json::JsonDictionary MovieEncoding::JsonForSave() const
	{
		Json::JsonMutableDictionary json;

		json.AddValue(L"DownloadId", downloadId);
		json.AddValue(L"TargetBitrate", targetBitrate);
		json.AddPath(L"TargetPath", targetPath);
		json.AddValue(L"SourceFilePathName", sourceFilePathName);

		return json;
	}

	Json::JsonDictionary MovieEncoding::EncodingProgress()
	{
		Json::JsonMutableDictionary json;

		json.AddValue(L"DownloadId", downloadId);
		json.AddPath(L"TargetPath", targetPath);
		json.AddValue(L"Status", Status());
		json.AddValue(L"ExpectedSize", String::StringFrom(TargetSize()));
		json.AddValue(L"EncodingSize", String::StringFrom(CurrentSize()));
		json.AddValue(L"EncodingBytesPerSec", EncodingSpeed());

		return json;
	}

	void MovieEncoding::ChangeTargetPath(wstring targetPath)
	{
		this->targetPath = targetPath;
	}
}
