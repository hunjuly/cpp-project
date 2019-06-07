#include "stdafx.h"
#include "ErrorMessage.h"

namespace CommonLib
{
	namespace Diagnostics
	{
		tstring GetLastErrorMessage()
		{
			return GetErrorMessage(GetLastError());
		}

		tstring GetErrorMessage(const int errorCode)
		{
			const int bufferSize=2048;
			vector<TCHAR> messageBuffer;
			messageBuffer.resize(bufferSize);

			FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
				, NULL
				, errorCode
				, 0
				, &messageBuffer[0]
			, bufferSize
				, NULL);

			return &messageBuffer[0];
		}
	}
}
