#pragma once
#include "Thread.h"

namespace CommonLib
{
	namespace Io
	{
		namespace Iocp
		{
			class IRequestable;

			class RequestHandler
			{
			public:
				RequestHandler(int numberOfThreads, int numberOfConcurrentThreads);
				~RequestHandler(void);
				
				void RegisterHandle(HANDLE handle, IRequestable* requestable);
				void Stop();
				void PostMessage(IRequestable* receiver, DWORD transferSize);
				int NumberOfThreads() const;

				//For Unit Test
				HANDLE Handle() const;
				const vector<HANDLE>& ThreadHandles() const;

			private:
				DWORD TaskThread(LPVOID NotUsed);
				HANDLE CreateNewCompletionPort(int numberOfConcurrentThreads);
				void CreateWorkingThread(int numberOfThreads);

				void PostQuitMessageToThreads();
				void WaitUntilThreadsEnding();

				HANDLE hCompPort;
				vector<Threading::Thread<RequestHandler> > workerThreads;
				vector<HANDLE> threadHandles;

			private: //����,���� �����ڴ� ���Ƴ��� �������� �����ϸ� �� �ȴ�.
				RequestHandler(const RequestHandler&); 
				RequestHandler& operator=(const RequestHandler&); 
			};
		}
	}
}