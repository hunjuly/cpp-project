#include "StdAfx.h"
#include "Socket.h"

namespace CommonLib
{
	namespace Network
	{
		namespace Sync
		{
			Socket::Socket(SOCKET socket)
			{
				this->socket=socket;
			}

			Socket::~Socket(void)
			{
				Close();
			}

			void Socket::Close()
			{
				if(INVALID_SOCKET!=socket)
				{
					shutdown(socket, SD_SEND);
					closesocket(socket);
					socket=INVALID_SOCKET;
				}
			}

			SOCKET Socket::Get()
			{
				return socket;
			}

			Socket::operator SOCKET()
			{
				return socket;
			}

			bool Socket::IsConnected()
			{
				return socket!=INVALID_SOCKET;
			}
		}
	}
}
