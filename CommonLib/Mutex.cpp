#include "stdafx.h"
#include "Mutex.h"

namespace CommonLib
{
	namespace Threading
	{
		Mutex::Mutex()
		{
			handle=CreateMutex(NULL, FALSE, NULL);

			if(NULL==handle)
			{
				throw Exceptions::LibraryException(__WFILE__, __WFUNCTION__, __LINE__,Diagnostics::GetLastErrorMessage().c_str());
			}
		}

		Mutex::~Mutex()
		{
			CloseHandle(handle);

			handle=NULL;
		}

		void Mutex::Lock()
		{
			WaitForSingleObject(handle, INFINITE);;
		}

		void Mutex::Unlock()
		{
			ReleaseMutex(handle);
		}
	}
}
