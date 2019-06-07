#pragma once

namespace CommonLib
{
	namespace System
	{
		class Service
		{
		public:
			Service(tstring serviceName);

			void Install(LPCWSTR filePath);
			void UnInstall();
			void Stop();
			void Start();
			bool IsRunning();
			bool IsExist();

		private: 
			tstring serviceName;
		};
	}
}