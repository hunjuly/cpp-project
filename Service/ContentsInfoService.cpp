#include "stdafx.h"
#include "ContentsInfoService.h"
#include "HttpReceive.h"
#include "HttpResponse.h"
#include "MediaInformation.h"
#include "MovieMetadata.h"

using namespace CommonLib::Utility;

namespace Service
{
	bool ContentsInfoService::IsCanProcess(const tstring& serviceName)
	{
		return String::IsEqual(L"ContentsInfo", serviceName, true);
	}

	void ContentsInfoService::ProcessRequest(const Http::Receive& receive, Http::Response* response)
	{
		//������� ���� ����
		if (Config::Instance().IgnorePowerSaveMode)
			::SetThreadExecutionState(ES_SYSTEM_REQUIRED);

		if(String::IsEqual(receive.urlPath.back(), L"ContentList.htm"))
			ContentsList(receive, response);
		else if(String::IsEqual(receive.urlPath.back(), L"MovieInfo.htm"))
			MovieInfo(receive, response);
		else if(String::IsEqual(receive.urlPath.back(), L"FolderInfo.htm"))
			FolderInfo(receive, response);
		//else if(String::IsEqual(receive.urlPath.back(), L"SubtitleList.htm"))
		//	SubtitleList(receive, response);
		else if(String::IsEqual(receive.urlPath.back(), L"DownloadSubtitle.htm"))
			DownloadSubtitle(receive, response);
	}

	void ContentsInfoService::ContentsList(const Http::Receive& receive, Http::Response* response)
	{
		Json::JsonMutableDictionary sendData;

		auto targetPath = receive.JsonData().Path(L"TargetPath");

		if(String::IsEqual(targetPath, L""))
		{
			Json::JsonMutableArray jsonFolderList;
			
			if(String::IsEqual(receive.JsonData().String(L"TargetPathType"), L"Public"))
			{
				for (auto& path : Config::Instance().PublicVideoFolders)
				{
					jsonFolderList.AddValue(path.Path);
				}
			}
			else if(String::IsEqual(receive.JsonData().String(L"TargetPathType"), L"Private"))
			{
				if(String::IsEqual(receive.JsonData().String(L"PrivateFolderPassword"), Config::Instance().PrivateFolderPassword))
				{
					for (auto& path : Config::Instance().PrivateVideoFolders)
					{
						jsonFolderList.AddValue(path.Path);
					}
				}
			}

			sendData.AddValue(L"FolderList", jsonFolderList);
		}
		else
		{
			Json::JsonMutableArray jsonFolderList;
			jsonFolderList.AddValue(Path::FolderList(targetPath));
			sendData.AddValue(L"FolderList", jsonFolderList);

			Json::JsonMutableArray jsonFileList;
			jsonFileList.AddValue(MediaInformation::MovieFileList(targetPath));
			sendData.AddValue(L"MovieList", jsonFileList);
		}

		response->WithContent(sendData);
	}

	void ContentsInfoService::FolderInfo(const Http::Receive& receive, Http::Response* response)
	{
		//���� ��,������ ���� ��, ������ ����� �ϳ�
		tstring targetFolder = receive.JsonData().Path(L"TargetPath");

		if(false == Path::IsExist(targetFolder))
			return response->NotFound();

		Json::JsonMutableDictionary sendData;

		tstring title = Path::LastComponent(targetFolder);

		for(auto& rootFolder : Config::Instance().PublicVideoFolders)
		{
			if(targetFolder == rootFolder.Path)
			{
				title = targetFolder;
				break;
			}
		}

		sendData.AddValue(L"Title", title);
		sendData.AddValue(L"FolderCount", Path::FolderCount(targetFolder));
		sendData.AddValue(L"MovieCount", (int)MediaInformation::MovieFileList(targetFolder).size());
		sendData.AddValue(L"Thumbnail", FolderThumbail(targetFolder));

		response->WithContent(sendData);
	}

	vector<BYTE> ContentsInfoService::FolderThumbail(const tstring& folderName)
	{
		vector<tstring> fileList = MediaInformation::MovieFileList(folderName);

		if(fileList.size() == 0)
			return vector< BYTE >();

		try
		{
			MediaInformation::MovieMetadata& movieData = MediaInformation::MovieMetadata(Path::Combine(folderName, fileList[0]));

			return movieData.Thumbnail();
		}
		catch(const Exceptions::Exception& ex)
		{
			//����Ͽ��� ������ �߻��ϴ��� ����/���� ���� ������ �˷���� �ϴϱ�
			//�� �κп��� ���ܴ� �׳� �����ϵ��� �Ѵ�.
		}

		return vector<BYTE>();
	}

	void ContentsInfoService::MovieInfo(const Http::Receive& receive, Http::Response* response)
	{
		tstring requestedFile = receive.JsonData().Path(L"TargetPath");

		if(!Path::IsExist(requestedFile))
			return response->NotFound();

		Json::JsonMutableDictionary sendData;

		MediaInformation::MovieMetadata& movieData = MediaInformation::MovieMetadata(requestedFile);

		auto subtitleList = MediaInformation::SubtitleList(requestedFile, movieData);

		sendData.AddValue(L"MovieInfo", movieData.ToJson());
		sendData.AddValue(L"Thumbnail", movieData.Thumbnail());
		sendData.AddValue(L"Filename", Path::RemoveExtension(Path::LastComponent(requestedFile)));
		sendData.AddValue(L"SubtitleList", MediaInformation::SubtitleListToJson(subtitleList));

		response->WithContent(sendData);
	}

	void ContentsInfoService::DownloadSubtitle(const Http::Receive& receive, Http::Response* response)
	{
		tstring movieFilePath = receive.JsonData().Path(L"TargetPath");
		tstring type = receive.JsonData().String(L"type");
		bool isEmbedded = receive.JsonData().Boolean(L"isEmbedded");
		int embeddedIndex = receive.JsonData().Integer(L"embeddedIndex");

		response->WithContent(MediaInformation::SubtitleData(movieFilePath, type, isEmbedded, embeddedIndex));
	}
}
