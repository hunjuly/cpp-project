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

		//�Ϻ� �������� �߰��� ������ �־ ������ ���ڵ��� ���� ���Ѵ�.
		//�׷��� duration�� ���̰� ����� �ݺ��ؼ� ���� �������� ���ڵ� �ϰ� �ȴ�.
		//�׷��� �� �� ���� ���ڵ� ���̰� ���ٸ� �׳� ��� ��Ų��.
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