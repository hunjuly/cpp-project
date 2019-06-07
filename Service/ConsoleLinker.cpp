#include "stdafx.h"
#include "ConsoleLinker.h"
#include <Sddl.h>

using namespace CommonLib::Utility;

extern void StopServer();
extern void StartServer();

namespace ConsoleLinker
{
#define INSTANCES 4 
#define PIPE_TIMEOUT 5000
#define BUFSIZE 64*1024

	enum class State
	{
		Connecting
		, ReadingOrderCode
		, ReadingDataSize
		, ReadingData
		, WritingResultCode
		, WritingDataSize
		, WritingData
		, End
	};

	typedef struct
	{
		OVERLAPPED oOverlap;
		HANDLE hPipeInst;

		DWORD order;
		DWORD dataSize;
		DWORD bufferPos;
		DWORD transferSize;
		BYTE buffer[BUFSIZE];
		State state;
		BOOL fPendingIO;
	} PIPEINST;

	PIPEINST Pipe[INSTANCES];
	HANDLE hEvents[INSTANCES];

	DWORD WINAPI InstanceThread(LPVOID unused);

	HANDLE hThread = NULL;
	static bool isContinue;

	void CreatePipes();

	void Run()
	{
		CreatePipes();

		isContinue = true;
		DWORD  dwThreadId = 0;
		hThread = CreateThread(NULL, 0, InstanceThread, 0, 0, &dwThreadId);
	}

	void Stop()
	{
		if (hThread)
		{
			isContinue = false;

			for (int i = 0;i < INSTANCES; ++i)
			{
				CloseHandle(hEvents[i]);
			}

			CloseHandle(hThread);
			hThread = NULL;
		}
	}

	SECURITY_ATTRIBUTES* GetSecurity()
	{
		static char secDesc[SECURITY_DESCRIPTOR_MIN_LENGTH];
		static SECURITY_ATTRIBUTES sa;
		sa.lpSecurityDescriptor = &secDesc;
		
		if (InitializeSecurityDescriptor(sa.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION))
		{
			// ACL is set as NULL in order to allow all access to the object.
			if (SetSecurityDescriptorDacl(sa.lpSecurityDescriptor, TRUE, NULL, FALSE))
			{
				sa.nLength = sizeof(sa);
				sa.bInheritHandle = TRUE;
			
				return &sa;
			}
		}
		
		return NULL;
	}

	VOID Connect(PIPEINST& Pipe)
	{
		if (ConnectNamedPipe(Pipe.hPipeInst, &Pipe.oOverlap) == 0)
		{
			switch (GetLastError())
			{
			case ERROR_IO_PENDING:
				Pipe.fPendingIO = TRUE;
				break;

			case ERROR_PIPE_CONNECTED:
				if (SetEvent(Pipe.oOverlap.hEvent) == 0)
					throw(L"ConnectToNewClient 1");

				Pipe.fPendingIO = FALSE;
				break;

			default:
				throw(L"ConnectToNewClient 2");
			}
		}
		else
			throw(L"ConnectToNewClient 3");

		Pipe.state = State::Connecting;
	}

	VOID Reconnect(PIPEINST& Pipe)
	{
		if (!DisconnectNamedPipe(Pipe.hPipeInst))
			throw L"Reconnect 1";

		Connect(Pipe);
	}

	void CreatePipes()
	{
		SECURITY_ATTRIBUTES* secAttr = GetSecurity();

		for (int i = 0; i < INSTANCES; i++)
		{
			hEvents[i] = CreateEvent(NULL, TRUE, TRUE, NULL);

			if (hEvents[i] == NULL)
				throw(L"CreatePipes01");

			Pipe[i].oOverlap.hEvent = hEvents[i];
			Pipe[i].hPipeInst = CreateNamedPipe(TEXT("\\\\.\\pipe\\nGinMediaServerPipe3")
				, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
				, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT
				, INSTANCES
				, BUFSIZE*sizeof(TCHAR)
				, BUFSIZE*sizeof(TCHAR)
				, PIPE_TIMEOUT
				, secAttr);

			if (Pipe[i].hPipeInst == INVALID_HANDLE_VALUE)
				throw(L"CreatePipes02");

			Connect(Pipe[i]);
		}
	}

	void Write(PIPEINST& Pipe, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite)
	{
		BOOL fSuccess = WriteFile(Pipe.hPipeInst, lpBuffer, nNumberOfBytesToWrite, &Pipe.transferSize, &Pipe.oOverlap);

		if (fSuccess && nNumberOfBytesToWrite == Pipe.transferSize)
			Pipe.fPendingIO = FALSE;
		else if (!fSuccess && (GetLastError() == ERROR_IO_PENDING))
			Pipe.fPendingIO = TRUE;
		else
			Reconnect(Pipe);
	}

	void Read(PIPEINST& Pipe, LPVOID lpBuffer, DWORD nNumberOfBytesToWrite)
	{
		BOOL fSuccess = ReadFile(Pipe.hPipeInst, lpBuffer, nNumberOfBytesToWrite, &Pipe.transferSize, &Pipe.oOverlap);

		if (fSuccess && Pipe.transferSize != 0)
			Pipe.fPendingIO = FALSE;
		else if (!fSuccess && (GetLastError() == ERROR_IO_PENDING))
			Pipe.fPendingIO = TRUE;
		else
			Reconnect(Pipe);
	}

	void SendServerState(PIPEINST& Pipe)
	{
		DWORD value = (DWORD)serverState;

		memcpy(&Pipe.buffer[0], &value, sizeof(DWORD));

		Pipe.state = State::End;
		Write(Pipe, &Pipe.buffer[0], sizeof(DWORD));
	}

	Json::JsonArray GetNetworksJson();
	void GetTask(PIPEINST& Pipe)
	{
		if (Pipe.fPendingIO)
		{
			DWORD fSuccess = GetOverlappedResult(Pipe.hPipeInst, &Pipe.oOverlap, &Pipe.transferSize, FALSE);

			if (!fSuccess || (Pipe.state != State::Connecting &&Pipe.transferSize == 0))
			{
				Reconnect(Pipe);
				return;
			}
		}

		switch (Pipe.state)
		{
		case State::Connecting:
			Pipe.state = State::ReadingOrderCode;
			Read(Pipe, &Pipe.order, sizeof(DWORD));

			break;

		case State::ReadingOrderCode:
			/* order
			0 = GetState , 0 = stop 1 = start
			1 = Start ,  0 = fail 1 = success
			2 = Stop ,  0 = fail 1 = success
			3 = Write Config, size+content
			4 = Read config , size+content
			5 = Write Network List, size+content
			*/

			if (Pipe.order == 0)
			{
				SendServerState(Pipe);
			}
			else if (Pipe.order == 1)
			{
				StartServer();

				SendServerState(Pipe);
			}
			else if (Pipe.order == 2)
			{
				StopServer();

				SendServerState(Pipe);
			}
			else if (Pipe.order == 3)
			{
				string value = Config::Instance().GetJson();

				Pipe.dataSize = value.length();
				memcpy(&Pipe.buffer[0], value.c_str(), value.length());
				Pipe.bufferPos = 0;

				Pipe.state = State::WritingDataSize;
				Write(Pipe, &Pipe.dataSize, sizeof(DWORD));
			}
			else if (Pipe.order == 4)
			{
				Pipe.state = State::ReadingDataSize;
				Read(Pipe, &Pipe.dataSize, sizeof(DWORD));
			}
			else if (Pipe.order == 5)
			{
				string value = GetNetworksJson().ToString();

				Pipe.dataSize = value.length();
				memcpy(&Pipe.buffer[0], value.c_str(), value.length());
				Pipe.bufferPos = 0;

				Pipe.state = State::WritingDataSize;
				Write(Pipe, &Pipe.dataSize, sizeof(DWORD));
			}
			break;

		case State::ReadingDataSize:
			Pipe.bufferPos = 0;

			Pipe.state = State::ReadingData;
			Read(Pipe, &Pipe.buffer[Pipe.bufferPos], Pipe.dataSize - Pipe.bufferPos);

			break;

		case State::ReadingData:
			Pipe.bufferPos += Pipe.transferSize;

			if (0 < Pipe.dataSize - Pipe.bufferPos)
				Read(Pipe, &Pipe.buffer[Pipe.bufferPos], Pipe.dataSize - Pipe.bufferPos);
			else if (0 == Pipe.dataSize - Pipe.bufferPos)
			{
				Pipe.buffer[Pipe.dataSize] = 0;
				Config::Instance().SaveJson((char*)Pipe.buffer, Pipe.dataSize + 1);
				Pipe.order = 1;
				Pipe.state = State::End;
				Write(Pipe, &Pipe.order, sizeof(DWORD));
			}
			else
				Reconnect(Pipe);

			break;

		case State::WritingDataSize:
			Pipe.state = State::WritingData;
			Write(Pipe, &Pipe.buffer[Pipe.bufferPos], Pipe.dataSize - Pipe.bufferPos);

			break;

		case State::WritingData:
			Pipe.bufferPos += Pipe.transferSize;

			if (0 < Pipe.dataSize - Pipe.bufferPos)
				Write(Pipe, &Pipe.buffer[Pipe.bufferPos], Pipe.dataSize - Pipe.bufferPos);
			else
			{
				Pipe.state = State::End;
				Read(Pipe, &Pipe.order, sizeof(DWORD));
			}

			break;

		case State::End:
			Pipe.state = State::ReadingOrderCode;
			Read(Pipe, &Pipe.order, sizeof(DWORD));
			break;

		default:
			throw(L"ProcessTask 2");
		}
	}

	Json::JsonArray GetNetworksJson()
	{
		vector<Network::AdapterInfo> networks = Network::GetIpAddresses();
		Json::JsonMutableArray array;

		for (auto network : networks)
		{
			array.AddValue(String::Format(L"%s(%s)", network.Description.c_str(), network.Ip.c_str()));
		}

		return array;
	}

	DWORD WINAPI InstanceThread(LPVOID unused)
	{
		while (isContinue)
		{
			DWORD dwWait = WaitForMultipleObjects(INSTANCES, hEvents, FALSE, INFINITE);

			if (isContinue == false)
				break;

			int i = dwWait - WAIT_OBJECT_0;  // determines which pipe 

			if (0 <= i && i < INSTANCES)
			{
				GetTask(Pipe[i]);
			}
		}

		return 0;
	}
}