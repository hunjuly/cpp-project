#pragma once

namespace CommonLib
{
	namespace System
	{
		bool PowerOff();
		long long TotalMemory();
		long long MemoryUsage();
		int CpuUsage();
		long long DiskUsedSpace(tstring driveName);
		long long DiskTotalSpace(tstring driveName);
		wstring ComputerName();
		int NumberOfCpus();
	}
}