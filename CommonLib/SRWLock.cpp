#include "stdafx.h"
#include "SRWLock.h"

namespace CommonLib
{
	namespace Threading
	{
		SRWLock::SRWLock()
		{
			InitializeSRWLock(&lock);
		}

		SRWLock::~SRWLock(){}

		void SRWLock::ReadLock()
		{
			AcquireSRWLockShared(&lock);
		}

		void SRWLock::ReadUnlock()
		{
			ReleaseSRWLockShared(&lock);
		}

		void SRWLock::WriteLock()
		{
			AcquireSRWLockExclusive(&lock);
		}

		void SRWLock::WriteUnlock()
		{
			ReleaseSRWLockExclusive(&lock);
		}
	}
}
