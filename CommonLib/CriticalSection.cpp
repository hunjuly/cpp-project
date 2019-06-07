#include "stdafx.h"
#include "CriticalSection.h"

namespace CommonLib
{
	namespace Threading
	{
		CriticalSection::CriticalSection()
		{
			lockCount=0;

			if(FALSE==InitializeCriticalSectionAndSpinCount(&section, 0x80000400))
			{
				throw Exceptions::LibraryException(__WFILE__, __WFUNCTION__, __LINE__,Diagnostics::GetLastErrorMessage().c_str());
			}
		}

		CriticalSection::~CriticalSection()
		{
			_ASSERT(lockCount==0);

			DeleteCriticalSection(&section);
		}

		void CriticalSection::Lock()
		{
			EnterCriticalSection(&section);
			lockCount++;
		}

		void CriticalSection::Unlock()
		{
			lockCount--;
			LeaveCriticalSection(&section);

			_ASSERT(lockCount>=0);
		}

		void CriticalSection::Relock(int milisecDelta)
		{
			Unlock();
			Sleep(milisecDelta);
			Lock();
		}

		void CriticalSection::SafeLock(const function<void()>& task)
		{
			Lock();

			try
			{
				task();
			}
			catch(...)
			{
				Unlock();

				throw;
			}

			Unlock();
		}
	}
}
