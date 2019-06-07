#pragma once
#include "Socket.h"

namespace CommonLib
{
	namespace Network
	{
		namespace Sync
		{
			class Transfer
			{
			public: 
				Transfer(SOCKET socket);

				vector<BYTE> Recv();
				vector<BYTE> Recv(int maxSize);
				vector<BYTE> RecvUnit();
				ULONG RecvLong();

				void Send(const BYTE* buffer, int size);
				void Send(const vector<BYTE>& buffer);
				void SendUnit(const BYTE* buffer, int size);
				void SendUnit(const vector<BYTE>& buffer);
				void Send(ULONG hostOrderLong);

			private:
				SOCKET socket;
			};
		}
	}
}
