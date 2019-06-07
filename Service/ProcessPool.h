#pragma once

class ProcessPool
{
public:
	static ProcessPool& Instance();

	void AddProcess(HANDLE processId);
	void RemoveProcess(HANDLE processId);
	void TerminateAllProcess();
	void KillAllProcess();

private:
	ProcessPool(void);
	set<HANDLE> processPool;

	Threading::CriticalSection cs;
	
};
