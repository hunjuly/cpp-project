#include "StdAfx.h"
#include "RequestHandler.h"
#include "IRequestable.h"

namespace CommonLib
{
	namespace Io
	{
		namespace Iocp
		{
			RequestHandler::RequestHandler(int numberOfThreads, int numberOfConcurrentThreads)
			{
				hCompPort=CreateNewCompletionPort(numberOfConcurrentThreads);

				CreateWorkingThread(numberOfThreads);
			}

			RequestHandler::~RequestHandler(void)
			{
				CloseHandle(Handle());
			}

			HANDLE RequestHandler::CreateNewCompletionPort(int numberOfConcurrentThreads)
			{
				return CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, numberOfConcurrentThreads);
			}

			void RequestHandler::CreateWorkingThread(int numberOfThreads)
			{
				if (numberOfThreads>MAXIMUM_WAIT_OBJECTS)
					THROW_LIBRARY(L"스레드 수는 MAXIMUM_WAIT_OBJECTS를 초과할 수 없습니다.");

				workerThreads.resize(numberOfThreads);
				threadHandles.resize(numberOfThreads);

				for(int i=0; i<numberOfThreads; ++i)
				{
					threadHandles[i]=workerThreads[i].Execute(&RequestHandler::TaskThread, this, NULL);
				}
			}

			void RequestHandler::RegisterHandle(HANDLE handle, IRequestable* requestable)
			{
				//AssociateDevice
				HANDLE ret=CreateIoCompletionPort(handle, hCompPort, (ULONG_PTR)requestable, NULL);

				_ASSERT(ret!=NULL);
			}

			void RequestHandler::Stop()
			{
				PostQuitMessageToThreads();

				WaitUntilThreadsEnding();
			}

			int RequestHandler::NumberOfThreads() const
			{
				return workerThreads.size();
			}

			void RequestHandler::WaitUntilThreadsEnding()
			{
				::WaitForMultipleObjects(threadHandles.size(), &threadHandles[0], TRUE, INFINITE);

				for(int i=0;i<threadHandles.size();++i)
				{
					CloseHandle(threadHandles[i]);
				}
			}

			void RequestHandler::PostMessage(IRequestable* receiver, DWORD transferSize)
			{
				PostQueuedCompletionStatus(hCompPort, transferSize, (ULONG_PTR)receiver, receiver);
			}

			void RequestHandler::PostQuitMessageToThreads()
			{
				for(size_t i=0; i<workerThreads.size(); ++i)
				{
					OVERLAPPED aOVERLAPPED;
					ZeroMemory(&aOVERLAPPED,sizeof(OVERLAPPED));

					//completionKey를 NULL로 던지기 때문에
					//TaskThread에서 completionKey==NULL면 종료하게 만든거다.
					PostQueuedCompletionStatus(hCompPort, 0, NULL, &aOVERLAPPED);
				}
			}

			DWORD RequestHandler::TaskThread(LPVOID NotUsed)
			{
				NotUsed;

				for(;;)
				{
					DWORD receivedBytes=0;
					ULONG_PTR completionKey=NULL;
					LPOVERLAPPED pOverlapped=nullptr;

					if(TRUE==GetQueuedCompletionStatus(
						hCompPort
						, &receivedBytes
						, &completionKey
						, (LPOVERLAPPED*)&pOverlapped
						, INFINITE)
						)
					{
						//GetQueuedCompletionStatus()가 TRUE인데 completionKey==NULL인 경우는 오직
						//PostQueuedCompletionStatus()를 실행했을 때이다.
						//이것은 즉, 스레드를 종료하라는 이야기
						if(completionKey==NULL)
						{
							break;
						}
					}

					//GetQueuedCompletionStatus의 성공/실패 여부에 상관없이 
					//completionKey가 유효하면 처리하도록 한다. 
					if(completionKey!=NULL)
						((IRequestable*)completionKey)->RequestProcessed(receivedBytes, pOverlapped);
				}

				return 0;
			}

			HANDLE RequestHandler::Handle() const
			{
				return hCompPort;
			}

			const vector<HANDLE>& RequestHandler::ThreadHandles() const
			{
				return threadHandles;
			}
		}
	}
}
