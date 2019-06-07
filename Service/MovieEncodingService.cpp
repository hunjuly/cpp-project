#include "stdafx.h"
#include "MovieEncodingService.h"
#include "HttpReceive.h"
#include "HttpResponse.h"

using namespace CommonLib::Utility;

namespace Service
{
	void MovieEncodingService::Initialize()
	{
		vector<tstring> encodingList = Path::FolderList(Config::Instance().EncodingTempFolder());

		for (tstring clientId : encodingList)
		{
			try
			{
				GetEncodingList(clientId);
			}
			catch (std::exception& ex)
			{
				Path::DeleteFolder(Path::Combine(Config::Instance().EncodingTempFolder(), clientId));
			}
		}
	}

	void MovieEncodingService::Uninitialize()
	{
		for (auto& encodingList : downloadLists)
		{
			encodingList->Uninitialize();
		}

		downloadLists.clear();
	}

	bool MovieEncodingService::IsCanProcess(const tstring& serviceName)
	{
		return String::IsEqual(L"MovieEncoding", serviceName, true);
	}

	void MovieEncodingService::ProcessRequest(const Http::Receive& receive, Http::Response* response)
	{
		//절전모드 진입 금지
		if (Config::Instance().IgnorePowerSaveMode)
			::SetThreadExecutionState(ES_SYSTEM_REQUIRED);

		if (String::IsEqual(receive.urlPath[1], L"EncodingMovieInfo.htm"))
			EncodingMovieInfo(receive, response);
		else if (String::IsEqual(receive.urlPath[1], L"DownloadFile"))
			DownloadFile(receive, response);
		else
			response->NotFound();
	}

	Json::JsonArray  MovieEncodingService::AllEncodingStatus()
	{
		Json::JsonMutableArray array;

		for (auto& encodingList : downloadLists)
		{
			Json::JsonMutableDictionary dic;
			dic.AddValue(L"ClientId", encodingList->clientId);
			dic.AddValue(L"ClientInfo", encodingList->clientInfo);
			dic.AddValue(L"StatusList", encodingList->EncodingProgressList());

			array.AddValue(dic);
		}

		return array;
	}

	MovieEncodingList& MovieEncodingService::GetEncodingList(const tstring& clientId)
	{
		for (auto& encodingList : downloadLists)
		{
			if (encodingList->IsEqual(clientId))
				return *encodingList;
		}

		downloadLists.push_back(unique_ptr<MovieEncodingList>(new MovieEncodingList(clientId)));

		return *downloadLists.back();
	}

	Json::JsonArray MovieEncodingService::EncodingProgressList(const tstring& clientId)
	{
		MovieEncodingList&  encodingList = GetEncodingList(clientId);

		return encodingList.EncodingProgressList();
	}

	Json::JsonArray MovieEncodingService::CompleteDownload(const Http::Receive& receive)
	{
		try
		{
			if (receive.JsonData().IsExistKey(L"CompleteDownloadIds") == false)
				return Json::JsonMutableArray();

			Json::JsonArray array = receive.JsonData().Object(L"CompleteDownloadIds");

			vector<tstring> completeDownloadList;

			for (int i = 0; i < array.Count(); ++i)
			{
				completeDownloadList.push_back(array.String(i));
			}

			if (0 < completeDownloadList.size())
			{
				MovieEncodingList& encodingList = GetEncodingList(receive.JsonData().String(L"UUID"));
				encodingList.Delete(completeDownloadList);
			}

			return array;
		}
		catch (...) {}

		return Json::JsonMutableArray();
	}

	void MovieEncodingService::PrepareDownload(const Http::Receive& receive)
	{
		try
		{
			if (receive.JsonData().IsExistKey(L"PrepareDownload") == false)
				return;

			Json::JsonArray array = receive.JsonData().Object(L"PrepareDownload");

			for (int i = 0; i < array.Count();++i)
			{
				Json::JsonDictionary dic = array.Object(i);

				MovieEncodingList& encodingList = GetEncodingList(dic.String(L"UUID"));
				encodingList.clientInfo = dic.String(L"ClientInfo");
				encodingList.Add(dic); //직접 전달하지 말고 뽑아서 줘야 한다.

			}
		}
		catch (...) {}
	}

	void MovieEncodingService::CancelDownload(const Http::Receive& receive)
	{
		try
		{
			if (receive.JsonData().IsExistKey(L"CancelDownload") == false)
				return;

			Json::JsonArray array = receive.JsonData().Object(L"CancelDownload");

			for (int i = 0; i < array.Count();++i)
			{
				Json::JsonDictionary dic = array.Object(i);

				Json::JsonArray idArray = dic.Object(L"DownloadIds");

				vector<tstring> downloadIds;

				for (int i = 0; i < idArray.Count(); ++i)
				{
					downloadIds.push_back(idArray.String(i));
				}

				MovieEncodingList& encodingList = GetEncodingList(dic.String(L"UUID"));
				encodingList.Delete(downloadIds);
			}
		}
		catch (...) {}
	}

	void MovieEncodingService::MoveDownload(const Http::Receive& receive)
	{
		try
		{
			if (receive.JsonData().IsExistKey(L"MoveDownload") == false)
				return;

			Json::JsonArray array = receive.JsonData().Object(L"MoveDownload");

			for (int i = 0; i < array.Count();++i)
			{
				Json::JsonDictionary dic = array.Object(i);

				Json::JsonArray idArray = dic.Object(L"DownloadIds");
				Json::JsonArray pathArray = dic.Object(L"TargetPaths");

				if (false == (idArray.Count() == pathArray.Count() && 0 < pathArray.Count()))
					break;

				vector<tstring> downloadIds;

				for (int i = 0; i < idArray.Count(); ++i)
				{
					downloadIds.push_back(idArray.String(i));
				}

				vector<tstring> targetPaths;

				for (int i = 0; i < pathArray.Count(); ++i)
				{
					targetPaths.push_back(pathArray.Path(i));
				}

				MovieEncodingList& encodingList = GetEncodingList(dic.String(L"UUID"));
				encodingList.MoveDownload(downloadIds, targetPaths);
			}
		}
		catch (...) {}
	}

	void MovieEncodingService::EncodingMovieInfo(const Http::Receive& receive, Http::Response* response)
	{
		tstring downloadId = receive.JsonData().String(L"DownloadId");

		MovieEncodingList& encodingList = GetEncodingList(receive.JsonData().String(L"UUID"));

		encodingList.SafeLock([&]()
		{
			if (encodingList.IsExist(downloadId))
			{
				response->WithContent(encodingList.Item(downloadId)->EncodingMovieInfo());
			}
			else
			{
				response->NotFound();
			}
		});
	}

	wstring UrlDecode(string SRC)
	{
		string ret;
		char ch;
		int i, ii;
		for (i = 0; i < SRC.length(); i++)
		{
			if (int(SRC[i]) == 37)
			{
				sscanf_s(SRC.substr(i + 1, 2).c_str(), "%x", &ii);
				ch = static_cast<char>(ii);
				ret += ch;
				i = i + 2;
			}
			else
			{
				ret += SRC[i];
			}
		}

		return String::MultiByteToWideChar(CP_UTF8, ret);
	}

	void MovieEncodingService::DownloadFile(const Http::Receive& receive, Http::Response* response)
	{
		wstring uuid = receive.urlPath[2];
		wstring downloadId = UrlDecode(String::WideCharToMultiByte(receive.urlPath[3]));

		MovieEncodingList& encodingList = GetEncodingList(uuid);

		encodingList.SafeLock([&]()
		{
			if (encodingList.IsExist(downloadId))
			{
				if (String::IsEqual(receive.serviceType, L".mov", true))
					response->WithFile(encodingList.Item(downloadId)->MovieFilePathName(), 0);
				else if (String::IsEqual(receive.serviceType, L".sbs", true))
					response->WithFile(encodingList.Item(downloadId)->SubtitlesFilePathName(), 0);
				else if (String::IsEqual(receive.serviceType, L".cfg", true))
					response->WithFile(encodingList.Item(downloadId)->CfgFilePathName(), 0);
			}
			else
			{
				response->NotFound();
			}
		});
	}
}
