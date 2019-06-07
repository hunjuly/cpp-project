#pragma once

namespace CommonLib
{
	namespace Network
	{
		struct AdapterInfo
		{
			tstring Description;
			tstring Ip;
		};

		void Initialize();
		void Uninitialize();
		SOCKET CreateTcpSocket();
		SOCKET CreateUdpSocket();
		vector<AdapterInfo> GetIpAddresses();
		bool IsPrivateIp(const tstring& ip);
		bool IsAvailableIp(const tstring& ip);

		namespace Converting
		{
			ULONG HostOrder(BYTE* buffer);
		}

		tstring GetWSALastErrorMessage();
	}
}
