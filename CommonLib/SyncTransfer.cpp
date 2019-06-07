#include "StdAfx.h"
#include "SyncTransfer.h"

namespace CommonLib
{
	namespace Network
	{
		namespace Sync
		{

			Transfer::Transfer(SOCKET socket) : socket(socket)
			{
			}

			ULONG Transfer::RecvLong()
			{
				ULONG networkOrderLong=0;

				int iResult=recv(socket, (char*)&networkOrderLong, sizeof(ULONG), 0);

				if(iResult==sizeof(ULONG))
					return ntohl(networkOrderLong);
				else if(iResult==0)
					return 0;
				else
					THROW_LIBRARY(Network::GetWSALastErrorMessage());			
			}

			vector<BYTE> Transfer::Recv()
			{
				vector<BYTE> buffer(1024);

				int iResult=recv(socket, (char*)&buffer[0], buffer.size(), 0);

				if(iResult>=0)
					buffer.resize(iResult);

				return buffer;
			}

			vector<BYTE> Transfer::Recv(int maxSize)
			{
				_ASSERT(maxSize>0);

				vector<BYTE> buffer(maxSize);

				int currentPosition=0;

				while(currentPosition!=buffer.size()) 
				{
					int iResult=recv(socket, (char*)&buffer[currentPosition], buffer.size()-currentPosition, 0);

					if(iResult>0)
						currentPosition+=iResult;
					else if(iResult==0)
					{
						break;
					}
					else
						THROW_LIBRARY(Network::GetWSALastErrorMessage());
				}

				return buffer;
			}

			vector<BYTE> Transfer::RecvUnit()
			{
				ULONG size=RecvLong();

				return Recv(size);
			}

			void Transfer::SendUnit(const BYTE* buffer, int size)
			{
				Send(size);
				Send(buffer,size);
			}

			void Transfer::SendUnit(const vector<BYTE>& buffer)
			{
				Send(buffer.size());
				Send(buffer);
			}

			void Transfer::Send(ULONG hostOrderLong)
			{
				ULONG networkOrderLong=htonl(hostOrderLong);

				send(socket, (const char*)&networkOrderLong, sizeof(ULONG), 0);
			}

			void Transfer::Send(const BYTE* buffer, int size)
			{
				send(socket, (const char*)buffer, size, 0);
			}

			void Transfer::Send(const vector<BYTE>& buffer)
			{
				Send(&buffer[0], buffer.size());
			}
		}
	}
}
