#pragma once
#include "Socket.h"

namespace CommonLib
{
	namespace Network
	{
		namespace Sync
		{
			unique_ptr<Socket> Listen(u_short port);
			unique_ptr<Socket> Listen(u_short port, int backLog);

			SOCKET  Connect(const tstring& address, const tstring& port);
			SOCKET  Connect(const tstring& addressWithPort);
		}
	}
}
