#include "StdAfx.h"
#include "MiniDump.h"

namespace CommonLib
{
	namespace Diagnostics
	{
		void(*MiniDump::PostDump)(TCHAR*);
		bool(*MiniDump::IsContinueMiniDump)(void);
		MINIDUMP_TYPE MiniDump::Type;

		typedef BOOL (WINAPI* MINIDUMPWRITEDUMP)(HANDLE hProcess
			, DWORD dwPid
			, HANDLE hFile
			, MINIDUMP_TYPE dumpType
			, CONST PMINIDUMP_EXCEPTION_INFORMATION exceptionParam
			, CONST PMINIDUMP_USER_STREAM_INFORMATION userStreamParam
			, CONST PMINIDUMP_CALLBACK_INFORMATION callbackParam
			);

		void _PostDump(TCHAR* dumpFileName){dumpFileName;}
		bool _IsContinueMiniDump(void){return true;}

		MiniDump::MiniDump()
		{
			SetUnhandledExceptionFilter(ProcessUnhandledException);
#ifdef _DEBUG
			Type=MiniDumpWithFullMemory;
#else
			Type=MiniDumpNormal;
#endif
			PostDump=_PostDump;
			IsContinueMiniDump=_IsContinueMiniDump;
		}

		LONG __stdcall MiniDump::ProcessUnhandledException(LPEXCEPTION_POINTERS params)
		{
			HMODULE hDll=::LoadLibrary(_T("DbgHelp.dll"));

			if(hDll==NULL)
			{
				return EXCEPTION_CONTINUE_SEARCH;
			}

			if(IsContinueMiniDump()==false)
			{
				return EXCEPTION_CONTINUE_SEARCH;
			}

			HANDLE hProcess=GetCurrentProcess();
			DWORD processId=GetCurrentProcessId();
			DWORD threadId=GetCurrentThreadId();

			MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
			exceptionInfo.ThreadId=threadId;
			exceptionInfo.ExceptionPointers=params;
			exceptionInfo.ClientPointers=FALSE;

			TCHAR currentPath[MAX_PATH];
			ZeroMemory(currentPath, MAX_PATH);

			GetModuleFileName(NULL, &currentPath[0], MAX_PATH);

			TCHAR dumpFilePath[MAX_PATH];
			ZeroMemory(dumpFilePath, MAX_PATH);

			_tcscpy_s(dumpFilePath, MAX_PATH, currentPath);
			_tcscat_s(dumpFilePath, MAX_PATH, _T(".dmp"));

			HANDLE hFile=CreateFile(
				dumpFilePath
				, GENERIC_READ|GENERIC_WRITE
				, FILE_SHARE_READ
				, NULL
				, CREATE_ALWAYS
				, FILE_ATTRIBUTE_NORMAL,NULL
				);

			MINIDUMPWRITEDUMP pMiniDumpWriteDump=(MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");

			if(pMiniDumpWriteDump != NULL)
				pMiniDumpWriteDump(hProcess, processId, hFile, Type, &exceptionInfo, NULL, NULL);

			CloseHandle(hFile);

			PostDump(dumpFilePath);

			return EXCEPTION_EXECUTE_HANDLER;
		}
	}
}
