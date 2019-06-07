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
					THROW_LIBRARY(L"������ ���� MAXIMUM_WAIT_OBJECTS�� �ʰ��� �� �����ϴ�.");

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

					//completionKey�� NULL�� ������ ������
					//TaskThread���� completionKey==NULL�� �����ϰ� ����Ŵ�.
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
						//GetQueuedCompletionStatus()�� TRUE�ε� completionKey==NULL�� ���� ����
						//PostQueuedCompletionStatus()�� �������� ���̴�.
						//�̰��� ��, �����带 �����϶�� �̾߱�
						if(completionKey==NULL)
						{
							break;
						}
					}

					//GetQueuedCompletionStatus�� ����/���� ���ο� ������� 
					//completionKey�� ��ȿ�ϸ� ó���ϵ��� �Ѵ�. 
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
