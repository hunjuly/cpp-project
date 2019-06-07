#pragma once
#include "IService.h"

namespace Service
{
	class ContentsInfoService :public IService
	{
	public:
		virtual bool IsCanProcess(const tstring& serviceName);
		virtual void ProcessRequest(const Http::Receive& receive, Http::Response* response);

	private:
		void ContentsList(const Http::Receive& receive, Http::Response* response);
		void FolderInfo(const Http::Receive& receive, Http::Response* response);
		void MovieInfo(const Http::Receive& receive, Http::Response* response);
		vector<BYTE> FolderThumbail(const tstring& folderName);
		//void SubtitleList(const Http::Receive& receive, Http::Response* response);
		void DownloadSubtitle(const Http::Receive& receive, Http::Response* response);
	};
}