#include "stdafx.h"
#include "Process.h"
#include <tlhelp32.h>

namespace CommonLib
{
	namespace System
	{
		Process::Process()
		{
			ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

			//SECURITY_ATTRIBUTES sa;
			//ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
			//{
			//	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			//	sa.bInheritHandle = TRUE;
			//}
		}

		Process::~Process()
		{
		}

		void Process::Start(const string& command)
		{
			STARTUPINFOA si;
			ZeroMemory(&si, sizeof(STARTUPINFOA));
			{
				si.cb = sizeof(STARTUPINFOA);
				si.dwFlags = STARTF_USESTDHANDLES;
				si.hStdOutput = NULL;
				si.hStdInput = NULL;
				si.hStdError = NULL;
			}

			if(CreateProcessA(NULL, (LPSTR)command.c_str(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi) == false)
				throw Exceptions::Exception(L"Process create Fail");
		}

		void Process::Start(const wstring& command)
		{
			STARTUPINFOW si;
			ZeroMemory(&si, sizeof(STARTUPINFOW));
			{
				si.cb = sizeof(STARTUPINFOW);
				si.dwFlags = STARTF_USESTDHANDLES;
				si.hStdOutput = NULL;
				si.hStdInput = NULL;
				si.hStdError = NULL;
			}

			if(CreateProcessW(NULL, (LPWSTR)command.c_str(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi) == false)
				throw Exceptions::Exception(L"Process create Fail");
		}

		void Process::Stop()
		{
			BOOL result  = ::TerminateProcess(pi.hProcess, 0);
		}

		void Process::Pause()
		{
			SuspendThread(pi.hThread);
		}

		void Process::Resume()
		{
			ResumeThread(pi.hThread);
		}

		int Process::ReturnCode()
		{
			DWORD dwExitCode = -1;

			GetExitCodeProcess(pi.hProcess, &dwExitCode);

			return dwExitCode;
		}

		bool Process::IsAlive() const
		{
			if(pi.hProcess == NULL)
				return false;

			DWORD dwExitCode;

			GetExitCodeProcess(pi.hProcess, &dwExitCode);

			return STILL_ACTIVE == dwExitCode;
		}

		HANDLE Process::Handle()
		{
			return pi.hProcess;
		}
	
		void Process::KillProcess(wstring filename)
		{
			PROCESSENTRY32 entry;
			entry.dwSize = sizeof(PROCESSENTRY32);

			HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

			if (Process32First(snapshot, &entry) == TRUE)
			{
				while (Process32Next(snapshot, &entry) == TRUE)
				{
					if (_wcsicmp(entry.szExeFile, filename.c_str()) == 0)
					{
						HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

						auto result = ::TerminateProcess(hProcess, -3);

						CloseHandle(hProcess);
					}
				}
			}

			CloseHandle(snapshot);
		}
	}
}
