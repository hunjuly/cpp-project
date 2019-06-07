#pragma once

namespace CommonLib
{
	namespace Network
	{
		class UPnP
		{
		public:
			void Unload();
			void Refresh();
			bool IsSupported();
			bool IsDidMapping(wstring name, wstring ip, int port);
			bool SetPortMapping(wstring name, wstring ip, int port);
			void RemoveMapping(int port);
			wstring RouterIp();

			UPnP();
			~UPnP();

		private:
			UPNPUrls urls;
			IGDdatas data;
			UPNPDev * devlist = 0;

			wstring routerIp;
			void UpdateRouterIp();
		};
	}
}
