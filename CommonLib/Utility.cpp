#include "stdafx.h"
#include "Utility.h"
#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")

namespace CommonLib
{
	namespace Utility
	{
		tstring CreateUuid()
		{
			UUID uuid;
			::ZeroMemory(&uuid, sizeof(UUID));

			::UuidCreate(&uuid);

			WCHAR* wszUuid = NULL;
			::UuidToStringW(&uuid, (RPC_WSTR*)&wszUuid);

			tstring uuidString = wszUuid;

			if(wszUuid != NULL)
			{
				::RpcStringFree((RPC_WSTR*)&wszUuid);
				wszUuid = NULL;
			}

			return uuidString;
		}
	}
}
