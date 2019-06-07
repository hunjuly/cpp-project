#pragma once

namespace CommonLib
{
	namespace Threading
	{
		class CriticalSection
		{
			LONG lockCount;
			CRITICAL_SECTION section; 

			void Lock();
			void Unlock();

		public:
			CriticalSection();
			~CriticalSection();

			void SafeLock(const function<void()>& task);
			void Relock(int milisecDelta);
		};
	}
}
