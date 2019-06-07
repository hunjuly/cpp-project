#include "StdAfx.h"
#include "MovableHeap.h"

namespace CommonLib
{
	//자세한 설명은 FixedHeap.cpp 참조
	namespace Memory
	{
		MovableHeap::MovableHeap(int size)
		{
			hgbl=GlobalAlloc(GHND, size);
			CHECK_NULL_THROW_LAST_ERROR(hgbl);
		}

		MovableHeap::~MovableHeap(void)
		{
			if(hgbl)
				GlobalFree(hgbl);
		}

		PVOID MovableHeap::Lock()
		{
			PVOID addr= (LPDLGTEMPLATE)GlobalLock(hgbl);
			
			CHECK_NULL_THROW_LAST_ERROR(addr);
		}

		void MovableHeap::Unlock()
		{
			if(!GlobalUnlock(hgbl))
			{
				DWORD errCode=GetLastError();

				if(errCode!=NO_ERROR)
					THROW_LIBRARY(Diagnostics::GetErrorMessage(errCode));
			}
		}
	}
}