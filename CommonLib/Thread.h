#pragma once
#include <process.h>

typedef unsigned int(__stdcall *PTHREAD_START)(void*);

namespace CommonLib
{
	namespace Threading
	{
		template<typename ClassName>
		class Thread
		{
			typedef DWORD(ClassName::*MemberPtr)(LPVOID);

		private:
			static DWORD WINAPI _Execute(LPVOID pThis)
			{
				Thread* thisPtr = (Thread*)pThis;

				ClassName* classPtr = thisPtr->classPtr;
				MemberPtr memberAddress = thisPtr->memberAddress;
				LPVOID parameter = thisPtr->parameter;

				return (classPtr->*memberAddress)(parameter);
			}

			MemberPtr memberAddress;
			ClassName* classPtr;
			LPVOID parameter;

			HANDLE threadHandle;
			unsigned int threadId;

		public:
			Thread()
			{
				threadHandle = INVALID_HANDLE_VALUE;
			}

			HANDLE Execute(MemberPtr memberAddress, ClassName* classPtr, LPVOID parameter)
			{
				_ASSERT(WAIT_OBJECT_0 == WaitStop(0));

				if(threadHandle != INVALID_HANDLE_VALUE)
					CloseHandle(threadHandle);

				this->memberAddress = memberAddress;
				this->classPtr = classPtr;
				this->parameter = parameter;

				threadHandle = (HANDLE)_beginthreadex(NULL, 0, (PTHREAD_START)_Execute, this, 0, &threadId);

				return threadHandle;
			}

			int WaitStop(DWORD time = INFINITE)
			{
				if(INVALID_HANDLE_VALUE == threadHandle)
					return WAIT_OBJECT_0;

				int result = WaitForSingleObject(threadHandle, time);

				if(result != WAIT_TIMEOUT)
				{
					CloseHandle(threadHandle);
					threadHandle = INVALID_HANDLE_VALUE;
				}

				return result;

			}

			//Terminate로 종료하면 스레드 종료할 때 반드시 잘못된 포인터 문제가 생긴다.
			//절대 사용하지 말자
			//void Terminate()
			//{
			//	if(INVALID_HANDLE_VALUE == threadHandle)
			//		return;

			//	TerminateThread(threadHandle, 0);

			//	CloseHandle(threadHandle);
			//	threadHandle = INVALID_HANDLE_VALUE;
			//}

			bool IsRunning() const
			{
				if(threadHandle == INVALID_HANDLE_VALUE)
					return false;

				return WAIT_TIMEOUT == WaitForSingleObject(threadHandle, 0);
			}
		};
	}
}
