#pragma once


enum class ServerState
{
	Error = 0,
	Running = 1,
	Stop = 2,
	Wating = 3

};

extern ServerState serverState;

class Config
{
private:
	class CloudAccount
	{
	public:
		tstring UUID;
		tstring Email;
		tstring Type;
		tstring Data;
	};

	class CredentialPath
	{
	public :
		wstring Path;
		wstring Id;
		wstring Password;
		wstring Type = L"Normal";
	};

public:
	static Config& Instance();
	~Config(void);

	tstring StreamingTempFolder();
	tstring EncodingTempFolder();
	string GetJson();
	void SaveJson(char* data, int size);
	bool IsAvailable();
	bool IsNetworkAvailable();

public:
	int SelectedNetwork = 0;
	int Port = 4978;
	tstring Password = L"";
	tstring PrivateFolderPassword = L"";
	vector<CloudAccount> CloudAccounts;
	vector<CredentialPath> PublicVideoFolders;
	vector<CredentialPath> PrivateVideoFolders;
	bool IgnorePowerSaveMode = true;

private:
	//TempFolder�� ���� �������� �ʴ´�.
	//�׷��� ��� ������ �� ������ ���� ������ ����ϰ� �Ѵ�.
	//�׷��� ��� �� ������ ������ ������ �پ���.
	tstring tempFolder = L"";
	tstring TempFolder();
	Config(void);
	wstring FilePath();
	void ConnectNetworkFolder(const CredentialPath& pathInfo);

	bool UpdateConfig(const char* data);
	Utility::Json::JsonArray GetCloudAccountsJson();
	void SetCloudAccountsJson(web::json::value value);
	Utility::Json::JsonArray GetPublicVideoFoldersJson();
	void SetPublicVideoFoldersJson(web::json::value value);
	Utility::Json::JsonArray GetPrivateVideoFoldersJson();
	void SetPrivateVideoFoldersJson(web::json::value value);
	Utility::Json::JsonArray GetCredentialPathsJson(vector<CredentialPath>& list);
	void SetCredentialPathsJson(web::json::value value, vector<CredentialPath>& list);
};
