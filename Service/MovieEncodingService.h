#pragma once
#include "IService.h"
#include "MovieEncodingList.h"

namespace Service
{
	class MovieEncodingService : public IService
	{
	public:
		virtual bool IsCanProcess(const tstring& serviceName);
		virtual void ProcessRequest(const Http::Receive& receive, Http::Response* response);
		virtual void Initialize();
		virtual void Uninitialize();

		Json::JsonArray  MovieEncodingService::AllEncodingStatus();
		Json::JsonArray EncodingProgressList(const tstring& clientId);
		Json::JsonArray CompleteDownload(const Http::Receive& receive);
		void PrepareDownload(const Http::Receive& receive);
		void CancelDownload(const Http::Receive& receive);
		void MoveDownload(const Http::Receive& receive);

	private:
		MovieEncodingList& GetEncodingList(const tstring& clientId);

		void EncodingMovieInfo(const Http::Receive& receive, Http::Response* response);
		void DownloadFile(const Http::Receive& receive, Http::Response* response);

	private:
		vector<std::unique_ptr<MovieEncodingList>> downloadLists;
	};
}
