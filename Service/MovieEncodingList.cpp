#include "stdafx.h"
#include "MovieEncodingList.h"
#include "MediaInformation.h"
#include "Config.h"

namespace Service
{
	using namespace Utility;

	MovieEncodingList::MovieEncodingList(tstring clientId)
	{
		this->clientId = clientId;

		if (Path::IsExist(StoragePath()) == false)
			Path::CreateFolder(StoragePath(), true);

		LoadList();

		workingContinued = true;
		encodingMovieThread.Execute(&MovieEncodingList::EncodingMovieThread, this, 0);
	}

	MovieEncodingList::~MovieEncodingList()
	{}

	void MovieEncodingList::Uninitialize()
	{
		workingContinued = false;

		SafeLock([&]()
		{
			for (auto& download : downloads)
			{
				download->StopEncoding();
			}

			if (downloads.size() == 0)
			{
				Path::DeleteFolder(StoragePath(), true);
			}

			downloads.clear();
		});

		encodingMovieThread.WaitStop(10000);
	}

	tstring MovieEncodingList::StoragePath()
	{
		tstring path = Path::Combine(Config::Instance().EncodingTempFolder(), clientId);

		if (false == Path::IsExist(path))
		{
			Path::CreateFolder(path);
		}

		return path;
	}

	tstring MovieEncodingList::DownloadListFilePath()
	{
		return Path::Combine(StoragePath(), L"DownloadList.cfg");
	}

	bool MovieEncodingList::IsEqual(tstring clientId)
	{
		return this->clientId == clientId;
	}

	void MovieEncodingList::Add(const Json::JsonDictionary& json)
	{
		SafeLock([&]()
		{
			vector<tstring> targetList;

			Json::JsonArray folderList = Json::JsonArray(json.Object(L"FolderList"));

			for (int i = 0; i < folderList.Count(); ++i)
			{
				tstring folderPath = folderList.Path(i);
				tstring folderName = Path::LastComponent(folderPath);

				vector<tstring> fileList = MediaInformation::MovieFileList(folderPath);

				for (const tstring& filename : fileList)
				{
					downloads.push_back(
						unique_ptr<MovieEncoding>(
							new MovieEncoding(json, folderName, Path::Combine(folderPath, filename), StoragePath())
							)
						);
				}
			}

			Json::JsonArray movieList = Json::JsonArray(json.Object(L"MovieList"));

			for (int i = 0; i < movieList.Count(); ++i)
			{
				downloads.push_back(unique_ptr<MovieEncoding>(new MovieEncoding(json, L"", movieList.Path(i), StoragePath())));
			}
		});

		SaveList();
	}

	void MovieEncodingList::MoveDownload(const vector<tstring>& downloadIds, const vector<tstring>& targetPaths)
	{
		SafeLock([&]()
		{
			for (int i = 0; i < downloadIds.size(); ++i)
			{
				if (IsExist(downloadIds[i]))
				{
					Item(downloadIds[i])->ChangeTargetPath(targetPaths[i]);
				}
			}
		});

		SaveList();
	}

	void MovieEncodingList::LoadList()
	{
		SafeLock([&]()
		{
			downloads.clear();

			if (Path::IsExist(DownloadListFilePath()) == false)
				return;

			Io::File::FileReader file(DownloadListFilePath());

			vector<BYTE> buffer = file.ReadToEnd();
			buffer.push_back(0);

			bool success = false;

			Json::JsonDictionary dic(String::MultiByteToWideChar(CP_UTF8, (char*)&buffer[0]), &success);

			if (success)
			{
				clientInfo = dic.String(L"ClientInfo");

				if (dic.IsExistKey(L"List"))
				{
					Json::JsonArray array(dic.Object(L"List"));

					for (int i = 0; i < array.Count(); ++i)
					{
						Json::JsonDictionary json(array.Object(i));

						downloads.push_back(unique_ptr<MovieEncoding>(new MovieEncoding(json, L"", L"", StoragePath())));
					}
				}
			}
			else
			{
				Path::DeleteFile(DownloadListFilePath());
			}
		});
	}

	void MovieEncodingList::SaveList()
	{
		SafeLock([&]()
		{
			Utility::Json::JsonMutableArray array;

			for (auto& movie : downloads)
			{
				array.AddValue(movie->JsonForSave());
			}

			Io::File::FileWriter writer(DownloadListFilePath());

			Json::JsonMutableDictionary dic;
			dic.AddValue(L"List", array);
			dic.AddValue(L"ClientInfo", clientInfo);

			writer.Write(dic.ToString());
		});
	}

	DWORD MovieEncodingList::EncodingMovieThread(LPVOID pThis)
	{
		while (workingContinued)
		{
			SafeLock([&]()
			{
				bool found = false;

				for (auto& download : downloads)
				{
					if (download->IsEncoding())
					{
						found = true;
						break;
					}
				}

				if (found == false)
				{
					for (auto& download : downloads)
					{
						if (download->IsWaiting() && download->IsPrepared())
						{
							download->StartEncoding();
							found = true;
							break;
						}
					}
				}

				if (found == false)
				{
					for (auto& download : downloads)
					{
						//실패는 했지만 혹시라도 복구가 됐을지도 모른다.
						//일시적인 네트워크 장애로 파일을 읽지 못하면 여기서 복구 할 수 있다.
						if (download->IsFailed())
						{
							download->StartEncoding();
							found = true;
							break;
						}
					}
				}

				//인코딩 무비 업데이트가 안 되었으면 여기서 해준다. 1초에 하나씩 업데이트 된다.
				for (auto& download : downloads)
				{
					if (download->IsPrepared() == false)
					{
						download->Update();
						//한 번에 너무 많이Update를 하면 응답을 하지 못해서 서버가 멈추게 된다.
						break;
					}
				}
			});

			Sleep(1000);
		}

		return 0;
	}

	void MovieEncodingList::Delete(vector<tstring> deleteIds)
	{
		SafeLock([&]()
		{
			for (auto& deleteId : deleteIds)
			{
				for (auto download = downloads.begin(); download != downloads.end(); ++download)
				{
					if ((*download)->IsEqual(deleteId))
					{
						(*download)->Delete();
						downloads.erase(download);
						break;
					}
				}
			}

			SaveList();
		});
	}

	Json::JsonArray MovieEncodingList::EncodingProgressList()
	{
		Utility::Json::JsonMutableArray array;

		auto hasUploadStatus = String::IsEqual(clientId, L"CloudClientId");

		SafeLock([&]()
		{
			for (auto& movie : downloads)
			{
				array.AddValue(movie->EncodingProgress());
			}
		});

		return array;
	}

	MovieEncoding* MovieEncodingList::Item(tstring downloadId)
	{
		MovieEncoding* item = nullptr;

		SafeLock([&]()
		{
			for (auto& download : downloads)
			{
				if (download->IsEqual(downloadId))
				{
					item = download.get();
					break;
				}
			}
		});

		return item;
		//THROW_LIBRARY(L"Not Found");
	}

	bool MovieEncodingList::IsExist(tstring downloadId)
	{
		bool result = false;

		SafeLock([&]()
		{
			for (auto& download : downloads)
			{
				if (download->IsEqual(downloadId))
				{
					result = true;
					break;
				}
			}
		});

		return result;
	}

	void MovieEncodingList::SafeLock(const function<void()>& task)
	{
		cs.SafeLock(task);
	}
}
