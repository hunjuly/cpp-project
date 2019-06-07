// nGinMediaServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
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

	//SubtitleExtractThread나 MovieMetaData 가져올 때 등등 FFMpeg이 실행중일 수 있다.
	//이것을 종료해야 해당 코드가 종료된다.
	ProcessPool::Instance().KillAllProcess();

	//서비스에 요청된 처리는 모두 처리한 후에
	Service::ServiceHandler::Instance().Uninitialize();

	//클라이언트의 접속을 차단하고
	if (listener.get())
		listener->Stop();

	listener.reset();

	//클라이언트와 연결된 소켓도 모두 끊는다.
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

	//30초 기다려 본다.
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
	//SystemService의 cloudAccounts이 메모리릭으로 보고된다. 왜 그렇지?
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
	//ConsoleLinker::Run(); 전에 ffmpeg.exe 프로세스를 죽여야 한다.
	//소켓 뿐만 아니라 파이프도 영향 받는다.
	ProcessPool::Instance().KillAllProcess();

	System::Service service(pServiceName);

	if (argc == 1) //인자 없이 실행됐다.
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
