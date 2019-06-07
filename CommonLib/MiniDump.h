#pragma once

#include <dbghelp.h>

namespace CommonLib
{
	namespace Diagnostics
	{
		class MiniDump
		{
		private:
			static LONG __stdcall ProcessUnhandledException(LPEXCEPTION_POINTERS params);

		public:
			static MINIDUMP_TYPE Type;
			static void(*PostDump)(TCHAR*);
			static bool(*IsContinueMiniDump)(void);

			MiniDump();
		};
	}
}
