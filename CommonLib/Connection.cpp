#include "stdafx.h"

namespace CommonLib
{
	namespace Network
	{
		namespace Sync
		{
			unique_ptr<Socket> Listen(u_short port)
			{
				return Listen(port, 100);
			}

			unique_ptr<Socket> Listen(u_short port, int backLog)
			{
				SOCKET hServSock;
				SOCKADDR_IN servAddr;

				hServSock=WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				servAddr.sin_family=AF_INET;
				servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
				servAddr.sin_port=htons(port);

				if(::bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr))==SOCKET_ERROR) 
				{
					THROW_LIBRARY((Network::GetWSALastErrorMessage()));
				}

				if(::listen(hServSock, backLog) == SOCKET_ERROR) 
				{
					THROW_LIBRARY((Network::GetWSALastErrorMessage()));
				}

				return unique_ptr<Socket>(new Socket(hServSock));
			}

			SOCKET  Connect(const tstring& addressWithPort)
			{
				vector<tstring> addressAndPort=String::Tokenize(addressWithPort, _T(':'));

				if(2!=addressAndPort.size())
				{
					THROW_LIBRARY(String::Format(_T("wrong address.(%s)"), addressWithPort.c_str()));
				}

				return Connect(addressAndPort[0], addressAndPort[1]);
			}

			SOCKET Connect(const tstring& address, const tstring& port)
			{
				ADDRINFOT* addrInfo;
				{
					ADDRINFOT hints;
					{
						ZeroMemory(&hints, sizeof(hints));

						hints.ai_family = AF_UNSPEC;
						hints.ai_socktype = SOCK_STREAM;
						hints.ai_protocol = IPPROTO_TCP;
					}

					int code=GetAddrInfo(address.c_str(), port.c_str(), &hints, &addrInfo);

					if(code!=0)
						THROW_LIBRARY(Diagnostics::GetErrorMessage(code));
				}

				SOCKET socket=::socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);

				if(socket==INVALID_SOCKET)
				{
					::FreeAddrInfo(addrInfo);

					THROW_LIBRARY(_T("can't create socket."));
				}

				int iResult=connect(socket, addrInfo->ai_addr, (int)addrInfo->ai_addrlen);

				FreeAddrInfo(addrInfo);

				if (SOCKET_ERROR==iResult) 
				{
					tstring msg=Network::GetWSALastErrorMessage();
					closesocket(socket);
					THROW_LIBRARY(msg);
				}

				return socket;
			}
		}
	}
}
