#include "stdafx.h"
#include "Network.h"

namespace CommonLib
{
	namespace Network
	{
		void Initialize()
		{
			WSADATA wsaData;

			int retCode=WSAStartup(MAKEWORD(2,2), &wsaData);

			switch(retCode)
			{
			case WSASYSNOTREADY:
				THROW_LIBRARY(_T("The underlying network subsystem is not ready for network communication."));
				break;

			case WSAVERNOTSUPPORTED:
				THROW_LIBRARY(_T("The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation."));
				break;

			case WSAEINPROGRESS:
				THROW_LIBRARY(_T("A blocking Windows Sockets 1.1 operation is in progress."));
				break;

			case WSAEPROCLIM:
				THROW_LIBRARY(_T("A limit on the number of tasks supported by the Windows Sockets implementation has been reached."));
				break;

			case WSAEFAULT:
				THROW_LIBRARY(_T("The lpWSAData parameter is not a valid pointer."));
				break;
			}
		}

		void Uninitialize()
		{
			WSACleanup();
		}

		SOCKET CreateTcpSocket()
		{
			SOCKET aSocket=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (aSocket==INVALID_SOCKET) 
			{
				THROW_LIBRARY(GetWSALastErrorMessage());
			}

			return aSocket;
		}

		SOCKET CreateUdpSocket()
		{
			SOCKET aSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

			if (aSocket==INVALID_SOCKET) 
			{
				THROW_LIBRARY(GetWSALastErrorMessage());
			}

			return aSocket;
		}

		vector<AdapterInfo> GetIpAddresses()
		{
			vector<AdapterInfo> ipList;

			ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
			PIP_ADAPTER_INFO pAdapterInfo=(IP_ADAPTER_INFO *)HeapAlloc(GetProcessHeap(), 0, ulOutBufLen);

			if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) 
			{
				HeapFree(GetProcessHeap(), 0, pAdapterInfo);

				pAdapterInfo = (IP_ADAPTER_INFO *)HeapAlloc(GetProcessHeap(), 0, ulOutBufLen);

				if (pAdapterInfo == NULL) {
					printf("Error allocating memory needed to call GetAdaptersinfo\n");
				}
			}

			if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)==NO_ERROR) 
			{
				PIP_ADAPTER_INFO pAdapter= pAdapterInfo;

				while (pAdapter) 
				{
					if(MIB_IF_TYPE_ETHERNET==pAdapter->Type||IF_TYPE_IEEE80211==pAdapter->Type)
					{
						AdapterInfo info;
						info.Description=String::MultiByteToWideChar(pAdapter->Description);
						info.Ip=String::MultiByteToWideChar(pAdapter->IpAddressList.IpAddress.String);

						//블루투스는 0.0.0.0 의 네트워크로 인식한다. 그래서 이걸로 제거
						if(IsAvailableIp(info.Ip))
							ipList.push_back(info);
					}

					pAdapter = pAdapter->Next;
				}
			}

			if(pAdapterInfo)
				HeapFree(GetProcessHeap(), 0, pAdapterInfo);

			return ipList;
		}

		bool IsAvailableIp(const tstring& ip)
		{
			return String::IsEqual(ip, L"0.0.0.0") == false
				&& String::IsEqual(ip, L"127.0.0.1") == false;
		}

		bool IsPrivateIp(const tstring& ip)
		{
			vector<tstring> list = String::Tokenize(ip, '.');

			if (list.size() != 4)
				return false;

			int value0 = StrToInt(list[0].c_str());
			int value1 = StrToInt(list[1].c_str());

			if (value0 == 192 && value1 == 168)
				return true;

			if (value0 == 10)
				return true;

			if (value0 == 172 && 16 <= value1 && value1 <= 31)
				return true;

			return false;
		}

		namespace Converting
		{
			ULONG HostOrder(BYTE* buffer)
			{
				ULONG networkOrderd=0;
				memcpy(&networkOrderd, buffer,sizeof(ULONG));

				return ntohl(networkOrderd);
			}
		}

		tstring GetWSALastErrorMessage()
		{
			return CommonLib::Diagnostics::GetErrorMessage(WSAGetLastError());
		}
	}
}
