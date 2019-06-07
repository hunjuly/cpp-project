#include "stdafx.h"
#include "service.h"

namespace CommonLib
{
	namespace System
	{
		Service::Service(tstring serviceName)
		{
			this->serviceName=serviceName;
		}

		void Service::Install(LPCWSTR filePath)
		{  
			SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE); 

			SC_HANDLE schService = CreateService
				( 
				schSCManager,	/* SCManager database      */ 
				serviceName.c_str(),			/* name of service         */ 
				serviceName.c_str(),			/* service name to display */ 
				SERVICE_ALL_ACCESS,        /* desired access          */ 
				SERVICE_WIN32_OWN_PROCESS/*|SERVICE_INTERACTIVE_PROCESS */, /* service type            */ 
				SERVICE_AUTO_START,      /* start type              */ 
				SERVICE_ERROR_NORMAL,      /* error control type      */ 
				filePath,			/* service's binary        */ 
				NULL,                      /* no load ordering group  */ 
				NULL,                      /* no tag identifier       */ 
				NULL,                      /* no dependencies         */ 
				NULL,                      /* LocalSystem account     */ 
				NULL
				);                     /* no password             */ 
			CloseServiceHandle(schService); 
			CloseServiceHandle(schSCManager);

			Start();
		}

		void Service::UnInstall()
		{
			SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS); 
			SC_HANDLE schService = OpenService( schSCManager, serviceName.c_str(), SERVICE_ALL_ACCESS);

			DeleteService(schService);

			CloseServiceHandle(schService); 
			CloseServiceHandle(schSCManager);	
		}

		void Service::Stop() 
		{ 
			SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, /*SC_MANAGER_ALL_ACCESS*/SC_MANAGER_CONNECT); 
			SC_HANDLE schService = OpenService( schSCManager, serviceName.c_str(), SERVICE_STOP);

			SERVICE_STATUS status;
			ControlService(schService, SERVICE_CONTROL_STOP, &status);

			CloseServiceHandle(schService); 
			CloseServiceHandle(schSCManager); 
		}

		void Service::Start() 
		{ 
			// run service with given name
			SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, /*SC_MANAGER_ALL_ACCESS*/SC_MANAGER_CONNECT); 
			SC_HANDLE schService = OpenService( schSCManager, serviceName.c_str(), SERVICE_START);
			// call StartService to run the service
			StartService(schService, 0, NULL);

			CloseServiceHandle(schService); 
			CloseServiceHandle(schSCManager); 
		}

		bool Service::IsExist()
		{
			SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, /*SC_MANAGER_ALL_ACCESS*/SC_MANAGER_CONNECT); 
			SC_HANDLE schService = OpenService( schSCManager, serviceName.c_str(), SERVICE_QUERY_STATUS);
		
			if(schService)
			CloseServiceHandle(schService); 

			if(schSCManager)
			CloseServiceHandle(schSCManager);

			return schService!=NULL;
		}

		bool Service::IsRunning()
		{
			SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, /*SC_MANAGER_ALL_ACCESS*/SC_MANAGER_CONNECT); 
			SC_HANDLE schService = OpenService( schSCManager, serviceName.c_str(), SERVICE_QUERY_STATUS);

			SERVICE_STATUS status;
			QueryServiceStatus(schService, &status);

			CloseServiceHandle(schService); 
			CloseServiceHandle(schSCManager); 

			return status.dwCurrentState==SERVICE_RUNNING?true:false;
		}

	}
}