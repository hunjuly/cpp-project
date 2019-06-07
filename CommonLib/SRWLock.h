#pragma once

namespace CommonLib
{
	namespace Threading
	{
		class SRWLock
		{
			SRWLOCK lock; 

		public:
			SRWLock();
			~SRWLock();

			void ReadLock();
			void ReadUnlock();
			void WriteLock();
			void WriteUnlock();
		};
	}
}
