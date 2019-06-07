#pragma once
#include "IService.h"

namespace Service
{
	class MovieEncodingService;

	class SystemService : public IService
	{
	public:
		virtual bool IsCanProcess(const tstring& serviceName);
		virtual void ProcessRequest(const Http::Receive& receive, Http::Response* response);
		virtual void Initialize();

		SystemService(MovieEncodingService* movieEncodingService);

	private:
		MovieEncodingService* movieDownloadService = nullptr;

		//cloudAccounts�� �޸𸮸����� ����ȴ�. �� �׷���?
		web::json::value cloudAccounts;
		web::json::value uploadProgressList;
		ULONGLONG uploadProgressList_UpdateTime = 0;

		void StatusForClient(const Http::Receive& receive, Http::Response* response);
		void StatusForConsole(const Http::Receive& receive, Http::Response* response);
		void PowerOff(const Http::Receive& receive, Http::Response* response);

		Threading::CriticalSection statusLock;

		wstring CurrentIp();
		void CheckAndSetPortForwarding();
		
		Network::UPnP upnp;
		Threading::Thread<SystemService> upnpInitThread;
		DWORD UPnPInitThread(LPVOID pThis);
	};
}
