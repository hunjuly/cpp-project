#include "StdAfx.h"
#include "DebugInfo.h"

namespace Debug
{
	void Output(tstring message)
	{
#ifdef _DEBUG
		::OutputDebugString(message.c_str());
		::OutputDebugString(L"\r\n");
#endif
	}

	void OutputA(string message)
	{
#ifdef _DEBUG
		::OutputDebugStringA(message.c_str());
		::OutputDebugString(L"\r\n");
#endif
	}
}
