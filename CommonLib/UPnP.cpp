#include "stdafx.h"
#include "UPnP.h"

namespace CommonLib
{
	namespace Network
	{
		UPnP::UPnP()
		{
			::ZeroMemory(&urls, sizeof(UPNPUrls));
		}

		UPnP::~UPnP()
		{
			Unload();
		}

		void UPnP::Refresh()
		{
			Unload();

			int error = 0;
			const char * multicastif = 0;
			const char * minissdpdpath = 0;

			devlist = upnpDiscover(15000, multicastif, minissdpdpath, 0, 0, &error);

			char lanaddr[64];	/* my ip address on the LAN */

			UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr));

			UpdateRouterIp();
		}

		void UPnP::Unload()
		{
			FreeUPNPUrls(&urls);

			if (devlist != 0)
			{
				freeUPNPDevlist(devlist);
				devlist = 0;
			}

			routerIp = L"";
		}

		bool UPnP::IsSupported()
		{
			return UPNPIGD_IsConnected(&urls, &data) == 1;
		}

		void UPnP::UpdateRouterIp()
		{
			char externalIPAddress[40];

			if (UPNP_GetExternalIPAddress(urls.controlURL, data.first.servicetype, externalIPAddress) == UPNPCOMMAND_SUCCESS)
			{
				//위에 함수가 결과를 리턴하는 데 시간이 걸린다. 그 동안 Unload되면 routerIp가 에러가 된다. 그래서 체크함
				if (devlist != 0)
					routerIp = String::MultiByteToWideChar(externalIPAddress);
			}
			else
			{
				if (devlist != 0)
					routerIp = L"Unknown";
			}
		}

		wstring UPnP::RouterIp()
		{
			return routerIp;
		}

		bool UPnP::SetPortMapping(wstring name, wstring ip, int port)
		{
			string iaddr = String::WideCharToMultiByte(ip);
			string description = String::WideCharToMultiByte(name);
			string portString = String::WideCharToMultiByte(String::StringFrom(port));

			return UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, portString.c_str()
				, portString.c_str(), iaddr.c_str(), description.c_str(), "TCP", 0, 0) == UPNPCOMMAND_SUCCESS;
		}

		void UPnP::RemoveMapping(int port)
		{
			string portString = String::WideCharToMultiByte(String::StringFrom(port));

			UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, portString.c_str(), "TCP", 0);
		}

		bool UPnP::IsDidMapping(wstring name, wstring ip, int port)
		{
			string portString = String::WideCharToMultiByte(String::StringFrom(port));

			char intClient[40];
			char intPort[6];
			char intDescription[255];

			if (UPNP_GetSpecificPortMappingEntry(
				urls.controlURL
				, data.first.servicetype
				, portString.c_str()
				, "TCP"
				, NULL
				, intClient
				, intPort
				, intDescription/*desc*/
				, NULL/*enabled*/
				, 0) == UPNPCOMMAND_SUCCESS)
			{
				wstring mappedIp = String::MultiByteToWideChar(intClient);
				wstring mappedDesc = String::MultiByteToWideChar(intDescription);

				return String::IsEqual(mappedIp, ip) && String::IsEqual(mappedDesc, name);
			}

			return false;
		}
	}
}
