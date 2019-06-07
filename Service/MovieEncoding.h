#pragma once
#include "FFMpegQualityOption.h"
#include "MediaInformation.h"

namespace Service
{
	class MovieEncoding
	{
	public:
		~MovieEncoding();

		MovieEncoding(const Utility::Json::JsonDictionary& json
			, tstring folderName
			, tstring sourceFilePath
			, tstring tempfolderPath
			);

		Utility::Json::JsonDictionary JsonForSave() const;
		Utility::Json::JsonDictionary EncodingProgress();
		Utility::Json::JsonDictionary EncodingMovieInfo() const;

		tstring MovieFilePathName() const;
		tstring CfgFilePathName() const;
		tstring SubtitlesFilePathName() const;

		void ChangeTargetPath(wstring targetPath);

		void StartEncoding();
		void StopEncoding();
		void Delete();
		bool IsEqual(tstring downloadId);
		bool IsEncoding() const;
		bool IsEncoded() const;
		bool IsWaiting() const;
		bool IsPrepared() const;
		bool IsFailed() const;

		void Update();

		tstring downloadId;
		tstring sourceFilePathName;

	private:
		FFMpeg::QualityOption option;

		int targetBitrate;
		tstring targetPath;
		vector<BYTE> thumbnail;

		int audioCount = 0;
		int duration;
		tstring tempfolderPath;
		bool encodingFailed = false;
		bool isCanRetry = false;

		bool workingContinued = false;
		Threading::Thread<MovieEncoding> controlEncodingThread;
		DWORD ControlEncodingThread(LPVOID pThis);

		bool movieInfoUpdated = false;

		bool isEncoding = false;
		System::Process process;

		string FFmpegCommand();

		long long TargetSize() const;
		long long CurrentSize() const;
		tstring Status() const;

		void CreateSubtitles(vector<MediaInformation::SubtitleHeader>& subtitleList);
		void CreateCfg(const vector<MediaInformation::SubtitleHeader>& subtitleList);
		tstring CreateDownloadId();

		bool IsEqualDuration(int encodedDuration);

		//일부 동영상은 중간에 오류가 있어서 끝까지 인코딩을 하지 못한다.
		//그러면 duration에 차이가 생기고 반복해서 오류 동영상을 인코딩 하게 된다.
		//그래서 두 번 연속 인코딩 길이가 같다면 그냥 통과 시킨다.
		int lastEncodedDuration;

		class BytePerSec
		{
		public:
			long long size;
			DWORD time;

			BytePerSec(long long size)
			{
				this->size = size;
				time = ::GetTickCount();
			}
		};

		int EncodingSpeed() const;
		deque<BytePerSec> encodingBytesPerSecList;
	};
}