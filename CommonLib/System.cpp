#include "stdafx.h"
#include "System.h"

#include <pdh.h>
#pragma comment(lib, "pdh.lib")

namespace CommonLib
{
	namespace System
	{
		bool PowerOff()
		{
			HANDLE hToken;
			TOKEN_PRIVILEGES tkp;

			// Get a token for this process. 
			if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
				return false;

			// Get the LUID for the shutdown privilege. 
			LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

			tkp.PrivilegeCount = 1;  // one privilege to set    
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			// Get the shutdown privilege for this process. 
			AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

			if(GetLastError() != ERROR_SUCCESS)
				return false;

			// Shut down the system and force all applications to close. 

			if(!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE
				, SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_UPGRADE | SHTDN_REASON_FLAG_PLANNED))
				return false;

			//shutdown was successful
			return true;
		}

		long long TotalMemory()
		{
			MEMORYSTATUSEX memInfo;
			memInfo.dwLength = sizeof(MEMORYSTATUSEX);
			GlobalMemoryStatusEx(&memInfo);

			return memInfo.ullTotalPhys;
		}

		long long MemoryUsage()
		{
			MEMORYSTATUSEX memInfo;
			memInfo.dwLength = sizeof(MEMORYSTATUSEX);
			GlobalMemoryStatusEx(&memInfo);

			return memInfo.ullTotalPhys - memInfo.ullAvailPhys;
		}

		int CpuUsage()
		{
			static PDH_HQUERY cpuQuery;
			static PDH_HCOUNTER cpuTotal;
			static bool init = false;

			if(init == false)
			{
				PdhOpenQuery(NULL, NULL, &cpuQuery);
				PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
				PdhCollectQueryData(cpuQuery);

				init = true;
			}

			PDH_FMT_COUNTERVALUE counterVal;

			PdhCollectQueryData(cpuQuery);
			PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);

			return counterVal.doubleValue;
		}

		long long DiskUsedSpace(tstring driveName)
		{
			ULARGE_INTEGER uliFreeBytesAvailableToCaller;
			ULARGE_INTEGER uliTotalNumberOfBytes;
			ULARGE_INTEGER uliTotalNumberOfFreeBytes;

			if(::GetDiskFreeSpaceEx(driveName.c_str(), &uliFreeBytesAvailableToCaller
				, &uliTotalNumberOfBytes, &uliTotalNumberOfFreeBytes) == TRUE)
			{
				return uliTotalNumberOfBytes.QuadPart - uliTotalNumberOfFreeBytes.QuadPart;
			}

			return 0;
		}

		long long DiskTotalSpace(tstring driveName)
		{
			ULARGE_INTEGER uliFreeBytesAvailableToCaller;
			ULARGE_INTEGER uliTotalNumberOfBytes;
			ULARGE_INTEGER uliTotalNumberOfFreeBytes;

			if(::GetDiskFreeSpaceEx(driveName.c_str(), &uliFreeBytesAvailableToCaller
				, &uliTotalNumberOfBytes, &uliTotalNumberOfFreeBytes) == TRUE)
			{
				return uliTotalNumberOfBytes.QuadPart;
			}

			return 0;
		}

		wstring ComputerName()
		{
			DWORD size = 1024;
			WCHAR name[1024];

			GetComputerName(name, &size);

			return name;
		}

		int NumberOfCpus()
		{
			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);

			return sysinfo.dwNumberOfProcessors;
		}
	}
}