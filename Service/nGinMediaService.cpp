// nGinMediaServer.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//
#include "stdafx.h"
#include "HttpTransfer.h"
#include "ServiceHandler.h"
#include "ProcessPool.h"
#include "Segmenter.h"
#include "MediaInformation.h"
#include "MovieMetadata.h"
#include "ConsoleLinker.h"

CommonLib::Diagnostics::MiniDump minidump;

bool IsContinueMiniDump(void)
{
	ProcessPool::Instance().TerminateAllProcess();

	if (Path::IsExist(Config::Instance().StreamingTempFolder()))
	{
		Path::DeleteFolder(Config::Instance().StreamingTempFolder(), true);
	}

	return true;
}

using namespace Network;
using namespace Network::Async;
using namespace Io::Iocp;

std::unique_ptr<TransferPool<Http::HttpTransfer>> transferPool;
std::unique_ptr<ConnectionListener> listener;

void StopServer()
{
	serverState = ServerState::Wating;

	//SubtitleExtractThread�� MovieMetaData ������ �� ��� FFMpeg�� �������� �� �ִ�.
	//�̰��� �����ؾ� �ش� �ڵ尡 ����ȴ�.
	ProcessPool::Instance().KillAllProcess();

	//���񽺿� ��û�� ó���� ��� ó���� �Ŀ�
	Service::ServiceHandler::Instance().Uninitialize();

	//Ŭ���̾�Ʈ�� ������ �����ϰ�
	if (listener.get())
		listener->Stop();

	listener.reset();

	//Ŭ���̾�Ʈ�� ����� ���ϵ� ��� ���´�.
	transferPool.reset();

	Network::Uninitialize();

	serverState = ServerState::Stop;
}

void StartServer()
{
	const int NumberOfThreads = 16;
	const int NumberOfMaxConnections = 100;
	const int Backlog = 5;

	try
	{
		serverState = ServerState::Wating;

		ProcessPool::Instance().KillAllProcess();

		Network::Initialize();

		Service::ServiceHandler::Instance().Initialize();

		const int idleTimeSec = 120;
		transferPool.reset(new TransferPool<Http::HttpTransfer>(NumberOfThreads, NumberOfMaxConnections, idleTimeSec));

		listener.reset(new ConnectionListener(*transferPool, NumberOfThreads));

		bool res = listener->Start(Config::Instance().Port, Backlog);

		serverState = res ? ServerState::Running : ServerState::Error;
	}
	catch (Exceptions::LibraryException& ex)
	{
		StopServer();

		serverState = ServerState::Error;
	}
}

DWORD WINAPI InstanceThread(LPVOID unused)
{
	int count = 0;

	if (false == Config::Instance().IsAvailable())
		return 0;

	//30�� ��ٷ� ����.
	while (count++ < 10)
	{
		Sleep(3000);

		if (Config::Instance().IsNetworkAvailable())
		{
			StartServer();

			break;
		}
	}

	return 0;
}

void StartServerInThread()
{
	DWORD  dwThreadId = 0;
	HANDLE hThread = CreateThread(NULL, 0, InstanceThread, 0, 0, &dwThreadId);
}

#ifdef _DEBUG
int _tmain(int argc, _TCHAR* argv[])
{
	//SystemService�� cloudAccounts�� �޸𸮸����� ����ȴ�. �� �׷���?
	serverState = ServerState::Stop;

	CommonLib::Diagnostics::MiniDump::IsContinueMiniDump = IsContinueMiniDump;

	ProcessPool::Instance().KillAllProcess();

	ConsoleLinker::Run();

	StartServerInThread();

	printf("Ready\n");

	getchar();

	printf("Ending\r\n");
	ConsoleLinker::Stop();

	if (serverState == ServerState::Running)
		StopServer();

	HWND guiHwnd = ::FindWindow(NULL, L"nGinMediaServer 3.0");
	PostMessage(guiHwnd, WM_DESTROY, 0, 0);

	return 0;
}

#else
LPWSTR pServiceName = L"nGinMediaServerServiceVer3";

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
VOID WINAPI ServiceHandler(DWORD fdwControl);

SERVICE_STATUS_HANDLE   hServiceStatusHandle;
SERVICE_STATUS          ServiceStatus;

int _tmain(int argc, _TCHAR* argv[])
{
	//ConsoleLinker::Run(); ���� ffmpeg.exe ���μ����� �׿��� �Ѵ�.
	//���� �Ӹ� �ƴ϶� �������� ���� �޴´�.
	ProcessPool::Instance().KillAllProcess();

	System::Service service(pServiceName);

	if (argc == 1) //���� ���� ����ƴ�.
	{
		serverState = ServerState::Stop;

		CommonLib::Diagnostics::MiniDump::IsContinueMiniDump = IsContinueMiniDump;

		ConsoleLinker::Run();

		StartServerInThread();

		SERVICE_TABLE_ENTRY	lpServiceStartTable[] = {
			{ pServiceName, ServiceMain }
			,{ NULL, NULL }
		};

		StartServiceCtrlDispatcher(lpServiceStartTable);
	}
	else if (argc == 2)
	{
		if (String::IsEqual(argv[1], L"-i"))
		{
			WCHAR pModuleFile[1024 + 1];
			DWORD dwSize = GetModuleFileName(NULL, pModuleFile, 1024);

			service.Install(pModuleFile);
			System::Firewall::AddAppToFirewall(pModuleFile, pServiceName);
		}
		else if (String::IsEqual(argv[1], L"-u"))
		{
			HWND guiHwnd = ::FindWindow(NULL, L"nGinMediaServer 3.0");
			PostMessage(guiHwnd, WM_DESTROY, 0, 0);

			service.Stop();
			service.UnInstall();

			WCHAR pModuleFile[1024 + 1];
			DWORD dwSize = GetModuleFileName(NULL, pModuleFile, 1024);

			System::Firewall::RemoveAppFromFirewall(pModuleFile, pServiceName);
		}
		else if (String::IsEqual(argv[1], L"-restart"))
		{
			WCHAR pModuleFile[1024 + 1];
			DWORD dwSize = GetModuleFileName(NULL, pModuleFile, 1024);

			service.Stop();
			service.UnInstall();
			service.Install(pModuleFile);
		}
	}

	return 0;
}

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	DWORD   status = 0;
	DWORD   specificError = 0xfffffff;

	ServiceStatus.dwServiceType = SERVICE_WIN32;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;

	hServiceStatusHandle = RegisterServiceCtrlHandler(pServiceName, ServiceHandler);
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;

	SetServiceStatus(hServiceStatusHandle, &ServiceStatus);
}

VOID WINAPI ServiceHandler(DWORD fdwControl)
{
	switch (fdwControl)
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		ConsoleLinker::Stop();

		if (serverState == ServerState::Running)
			StopServer();

		ServiceStatus.dwWin32ExitCode = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwCheckPoint = 0;
		ServiceStatus.dwWaitHint = 0;
		break;
	default:
		break;
	};

	SetServiceStatus(hServiceStatusHandle, &ServiceStatus);
}
#endif
