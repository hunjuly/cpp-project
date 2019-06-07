#include "stdafx.h"
#include "Config.h"

#include <ppltasks.h>


ServerState  serverState;

using namespace CommonLib::Utility;

Config& Config::Instance()
{
	static Config instance;

	return instance;
}

wstring Config::FilePath()
{
	return Path::Combine(Path::ModuleFilePath(), L"nGinMediaServer.cfg");
}

Config::Config(void)
{
	if (Path::IsExist(FilePath()))
	{
		Io::File::FileReader reader;
		reader.Open(FilePath());
		vector<BYTE> buffer = reader.ReadToEnd();
		reader.Close();

		UpdateConfig((char*)&buffer[0]);
	}
}

Config::~Config(void)
{}

tstring Config::TempFolder()
{
	//반드시 사용자가 지정한 폴더에 nGinTemp폴더를 만들어서 거기서 작업한다.
	//사용자가 지정한 폴더가 c:\와 같은 시스템 폴더라면 이걸 지울 수도 있다.
	if (false == Path::IsExist(tempFolder))
	{
		Path::CreateFolder(tempFolder);
	}

	return tempFolder;
}

tstring Config::StreamingTempFolder()
{
	tstring path = Path::Combine(TempFolder(), L"Streaming");

	if (false == Path::IsExist(path))
	{
		Path::CreateFolder(path);
	}

	return path;
}

tstring Config::EncodingTempFolder()
{
	tstring path = Path::Combine(TempFolder(), L"Encoding");

	if (false == Path::IsExist(path))
	{
		Path::CreateFolder(path);
	}

	return path;
}

Json::JsonArray Config::GetCloudAccountsJson()
{
	Json::JsonMutableArray array;

	for (auto& account : CloudAccounts)
	{
		Json::JsonMutableDictionary dic;
		dic.AddValue(L"UUID", account.UUID);
		dic.AddValue(L"Type", account.Type);
		dic.AddValue(L"Email", account.Email);
		dic.AddValue(L"Data", account.Data);

		array.AddValue(dic);
	}

	return array;
}

void Config::SetCloudAccountsJson(web::json::value value)
{
	Json::JsonArray array(value);

	CloudAccounts.clear();

	for (int i = 0; i < array.Count(); ++i)
	{
		CloudAccount account;

		Json::JsonDictionary dic(array.Object(i));
		account.UUID = dic.String(L"UUID");
		account.Type = dic.String(L"Type");
		account.Email = dic.String(L"Email");

		//나중에 추가된 데이터라서 없을 수 있다.
		if (dic.IsExistKey(L"Data"))
		{
			if (dic.IsNull(L"Data") == false)
				account.Data = dic.String(L"Data");
		}
		else
			account.Data = L"";

		CloudAccounts.push_back(account);
	}
}

Json::JsonArray Config::GetCredentialPathsJson(vector<CredentialPath>& list)
{
	Json::JsonMutableArray array;

	for (auto& path : list)
	{
		Json::JsonMutableDictionary dic;
		dic.AddValue(L"Path", path.Path);
		dic.AddValue(L"Id", path.Id);
		dic.AddValue(L"Password", path.Password);
		dic.AddValue(L"Type", path.Type);

		array.AddValue(dic);
	}

	return array;
}

void Config::SetCredentialPathsJson(web::json::value value, vector<CredentialPath>& list)
{
	Json::JsonArray array(value);

	list.clear();

	for (int i = 0; i < array.Count(); ++i)
	{
		CredentialPath path;

		Json::JsonDictionary dic(array.Object(i));

		path.Path = dic.String(L"Path");
		path.Id = dic.String(L"Id");
		path.Password = dic.String(L"Password");
		path.Type = dic.String(L"Type");

		list.push_back(path);
	}
}

Json::JsonArray Config::GetPublicVideoFoldersJson()
{
	return GetCredentialPathsJson(PublicVideoFolders);
}

void Config::SetPublicVideoFoldersJson(web::json::value value)
{
	SetCredentialPathsJson(value, PublicVideoFolders);
}

Json::JsonArray Config::GetPrivateVideoFoldersJson()
{
	return GetCredentialPathsJson(PrivateVideoFolders);
}

void Config::SetPrivateVideoFoldersJson(web::json::value value)
{
	SetCredentialPathsJson(value, PrivateVideoFolders);
}

string Config::GetJson()
{
	Json::JsonMutableDictionary dic;
	dic.AddValue(L"SelectedNetwork", SelectedNetwork);
	dic.AddValue(L"Port", Port);
	dic.AddValue(L"Password", Password);
	dic.AddValue(L"PrivateFolderPassword", PrivateFolderPassword);
	dic.AddValue(L"TempFolder", tempFolder);
	dic.AddValue(L"CloudAccounts", GetCloudAccountsJson());
	dic.AddValue(L"PublicVideoFolders", GetPublicVideoFoldersJson());
	dic.AddValue(L"PrivateVideoFolders", GetPrivateVideoFoldersJson());
	dic.AddValue(L"IgnorePowerSaveMode", IgnorePowerSaveMode ? 1 : 0);

	return dic.ToString();
}

bool Config::UpdateConfig(const char* data)
{
	bool success;
	Json::JsonDictionary dic(String::MultiByteToWideChar(CP_UTF8, data), &success);

	if (success == false)
		return false;

	SelectedNetwork = dic.Integer(L"SelectedNetwork");
	Port = dic.Integer(L"Port");
	Password = dic.String(L"Password");
	PrivateFolderPassword = dic.String(L"PrivateFolderPassword");
	tempFolder = dic.String(L"TempFolder");
	SetCloudAccountsJson(dic.Object(L"CloudAccounts"));
	SetPublicVideoFoldersJson(dic.Object(L"PublicVideoFolders"));
	SetPrivateVideoFoldersJson(dic.Object(L"PrivateVideoFolders"));
	IgnorePowerSaveMode = dic.Integer(L"IgnorePowerSaveMode") == 1;

	for (auto& value : PublicVideoFolders)
		ConnectNetworkFolder(value);

	for (auto& value : PrivateVideoFolders)
		ConnectNetworkFolder(value);

	return true;
}


void Config::ConnectNetworkFolder(const CredentialPath& pathInfo)
{
	if (String::IsEqual(pathInfo.Type, L"Network") == false)
		return;

	NETRESOURCE rc;
	rc.dwScope = RESOURCE_GLOBALNET;
	rc.dwType = RESOURCETYPE_ANY;
	rc.dwDisplayType = RESOURCEDISPLAYTYPE_DIRECTORY;
	rc.dwUsage = RESOURCEUSAGE_CONNECTABLE;
	rc.lpComment = NULL;
	rc.lpLocalName = NULL;
	rc.lpProvider = NULL;
	rc.lpRemoteName = (LPWSTR)pathInfo.Path.c_str();

	concurrency::create_task([pathInfo, rc]
	{
		WNetAddConnection2((LPNETRESOURCEW)&rc, pathInfo.Password.c_str(), pathInfo.Id.c_str(), CONNECT_UPDATE_RECENT);
	});
}

void Config::SaveJson(char* data, int size)
{
	if (UpdateConfig(data))
	{
		Io::File::FileWriter writer;
		writer.Create(FilePath());
		writer.Write(data, size);
		writer.Close();
	}
}

bool Config::IsAvailable()
{
	if (Port < 1024 || 49151 < Port)
		return false;

	if (Password.length() <= 0)
		return false;

	if (PublicVideoFolders.size() <= 0)
		return false;

	if (0 < PrivateVideoFolders.size() && PrivateFolderPassword.length() <= 0)
		return false;

	return true;
}

bool Config::IsNetworkAvailable()
{
	vector<Network::AdapterInfo> networks = Network::GetIpAddresses();

	if (SelectedNetwork < 0 || networks.size() <= SelectedNetwork)
		return false;

	if (Network::IsAvailableIp(networks[SelectedNetwork].Ip) == false)
		return false;

	return true;
}