#pragma once

#include "MovieEncoding.h"

namespace Service
{
	class MovieEncodingList
	{
	public:
		MovieEncodingList(tstring clientId);
		~MovieEncodingList();
		void Uninitialize();

		bool IsEqual(tstring clientId);
		void Add(const Utility::Json::JsonDictionary& json);
		void Delete(vector<tstring> deleteIds);
		void MoveDownload(const vector<tstring>& downloadIds, const vector<tstring>& targetPaths);

		Utility::Json::JsonArray EncodingProgressList();
		bool IsExist(tstring downloadId);

		void SafeLock(const function<void()>& task);

		tstring clientId;
		tstring clientInfo;
		MovieEncoding* Item(tstring downloadId);

		void SaveList();

	private:
		CommonLib::Threading::CriticalSection cs;

		tstring StoragePath();
		tstring DownloadListFilePath();

		vector<unique_ptr<MovieEncoding>> downloads;

		void LoadList();

		bool workingContinued = false;
		Threading::Thread<MovieEncodingList> encodingMovieThread;
		DWORD EncodingMovieThread(LPVOID pThis);
	};
}