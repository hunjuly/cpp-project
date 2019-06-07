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

			private: //복사,대입 연산자는 막아놓자 누구한테 대입하면 안 된다.
				RequestHandler(const RequestHandler&); 
				RequestHandler& operator=(const RequestHandler&); 
			};
		}
	}
}