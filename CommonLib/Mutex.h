#pragma once

namespace CommonLib
{
	namespace Threading
	{
		class Mutex
		{
			HANDLE handle;

		public:
			Mutex();
			~Mutex();

			void Lock();
			void Unlock();
		};
	}
}
