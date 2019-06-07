#include "stdafx.h"
#include "ProcessPool.h"

ProcessPool& ProcessPool::Instance()
{
	static ProcessPool instance;

	return instance;
}

ProcessPool::ProcessPool(void)
{
}

void ProcessPool::AddProcess(HANDLE processId)
{
	cs.SafeLock([&]()
	{
		processPool.insert(processId);
	});
}

void ProcessPool::RemoveProcess(HANDLE processId)
{
	cs.SafeLock([&]()
	{
		processPool.erase(processId);
	});
}

void ProcessPool::TerminateAllProcess()
{
	cs.SafeLock([&]()
	{
		for(HANDLE handle : processPool)
		{
			::TerminateProcess(handle, -2);
		}

		processPool.clear();
	});
}

void ProcessPool::KillAllProcess()
{
	System::Process::KillProcess(L"FFMpegnGinSoft.exe");
}