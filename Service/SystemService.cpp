#include "stdafx.h"
#include "HttpReceive.h"
#include "HttpResponse.h"
#include "MovieEncodingService.h"
#include "SystemService.h"

using namespace CommonLib::Utility;

namespace Service
{
	SystemService::SystemService(MovieEncodingService* movieDownloadService)
	{
		this->movieDownloadService = movieDownloadService;
	}

	void SystemService::Initialize()
	{
		if (upnpInitThread.IsRunning() == false)
			upnpInitThread.Execute(&SystemService::UPnPInitThread, this, 0);
	}

	DWORD SystemService::UPnPInitThread(LPVOID pThis)
	{
		try 
		{
			upnp.Refresh();

			CheckAndSetPortForwarding();
		}
		catch (...) {}

		return 0;
	}

	void SystemService::ProcessRequest(const Http::Receive& receive, Http::Response* response)
	{
		if (String::IsEqual(receive.urlPath.back(), L"StatusForClient.htm"))
			StatusForClient(receive, response);
		else if (String::IsEqual(receive.urlPath.back(), L"StatusForConsole.htm"))
			StatusForConsole(receive, response);
		else if (String::IsEqual(receive.urlPath.back(), L"PowerOff.htm"))
			PowerOff(receive, response);
		else
			response->NotFound();
	}

	bool SystemService::IsCanProcess(const tstring& serviceName)
	{
		return String::IsEqual(L"System", serviceName, true);
	}

	void SystemService::StatusForClient(const Http::Receive& receive, Http::Response* response)
	{
		statusLock.SafeLock([&]
		{
			Json::JsonArray array = movieDownloadService->CompleteDownload(receive);
			movieDownloadService->CancelDownload(receive);
			movieDownloadService->MoveDownload(receive);
			movieDownloadService->PrepareDownload(receive);

			Json::JsonMutableDictionary sendData;
			{
				sendData.AddValue(L"CompleteDownloadIds", array);

				//DownloadList
				sendData.AddValue(L"DownloadList", movieDownloadService->EncodingProgressList(receive.JsonData().String(L"UUID")));
				sendData.AddValue(L"UploadList", movieDownloadService->EncodingProgressList(L"CloudClientId"));

				//Etc
				sendData.AddValue(L"CloudAccounts", cloudAccounts);
				sendData.AddValue(L"TotalMemory", String::StringFrom(System::TotalMemory()));
				sendData.AddValue(L"MemoryUsage", String::StringFrom(System::MemoryUsage()));
				sendData.AddValue(L"CpuUsage", System::CpuUsage());
				sendData.AddValue(L"DiskUsedSpace", String::StringFrom(System::DiskUsedSpace(Config::Instance().EncodingTempFolder())));
				sendData.AddValue(L"DiskTotalSpace", String::StringFrom(System::DiskTotalSpace(Config::Instance().EncodingTempFolder())));
				sendData.AddValue(L"ServerName", System::ComputerName());

				//UploadProgress, 업데이트 된지 10초가 넘었다면 무시한다. 콘솔이 죽은 것이다.
				if (::GetTickCount64() - uploadProgressList_UpdateTime < 10 * 1000)
				{
					sendData.AddValue(L"UploadProgressList", uploadProgressList);
				}
			}

			response->WithContent(sendData);
		});
	}

	void SystemService::StatusForConsole(const Http::Receive& receive, Http::Response* response)
	{
		statusLock.SafeLock([&]
		{
			//from console
			movieDownloadService->CancelDownload(receive);

			cloudAccounts = receive.JsonData().Object(L"CloudAccounts");

			uploadProgressList = receive.JsonData().Object(L"UploadProgressList");
			uploadProgressList_UpdateTime = ::GetTickCount64();

			//to console
			Json::JsonArray array = movieDownloadService->CompleteDownload(receive);

			Json::JsonMutableDictionary sendData;
			{
				sendData.AddValue(L"CompleteDownloadIds", array);
				sendData.AddValue(L"EncodingStatusLists", movieDownloadService->AllEncodingStatus());

				wstring currentIp = CurrentIp();

				sendData.AddValue(L"ServerIp", currentIp);
				sendData.AddValue(L"Port", String::StringFrom(Config::Instance().Port));
				sendData.AddValue(L"RouterIp", upnp.RouterIp());

				if (Network::IsAvailableIp(currentIp)
					&& Network::IsPrivateIp(currentIp)
					&& String::IsEqual(upnp.RouterIp(), L"Unknown"))
					sendData.AddValue(L"NeedPortForwarding", 1);
				else
					sendData.AddValue(L"NeedPortForwarding", 0);
			}

			response->WithContent(sendData);
		});
	}

	wstring SystemService::CurrentIp()
	{
		wstring currentIp = L"";
		{
			vector<Network::AdapterInfo> networks = Network::GetIpAddresses();

			int selectedNetwork = Config::Instance().SelectedNetwork;

			if (0 <= selectedNetwork && selectedNetwork < networks.size())
			{
				currentIp = networks[selectedNetwork].Ip;
			}
		}

		return currentIp;
	}

	void SystemService::CheckAndSetPortForwarding()
	{
		if (upnp.IsSupported())
		{
			wstring ip = CurrentIp();

			if (false == upnp.IsDidMapping(L"nGinMediaServer", ip, Config::Instance().Port))
			{
				upnp.RemoveMapping(Config::Instance().Port);
				upnp.SetPortMapping(L"nGinMediaServer", ip, Config::Instance().Port);
			}
		}
	}

	void SystemService::PowerOff(const Http::Receive& receive, Http::Response* response)
	{
		if (System::PowerOff())
			response->Ok();
		else
			response->NotFound();
	}

}
