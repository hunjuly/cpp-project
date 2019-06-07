// TestFirewall.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

namespace CommonLib
{
	namespace System
	{
		namespace Firewall
		{
			bool AddAppToFirewall(LPCWSTR filename, LPCWSTR title)
			{
				tstring vistaCmd
					=String::Format(L"advfirewall firewall add rule name=\"%s\" dir=in action=allow program=\"%s\"",title,filename);
				::ShellExecute(NULL, L"open", L"netsh.exe",vistaCmd.c_str(),NULL,SW_HIDE);

				tstring xpCmd=String::Format(L"firewall add allowedprogram \"%s\" \"%s\" ENABLE",filename,title);
				::ShellExecute(NULL, L"open", L"netsh.exe",xpCmd.c_str(),NULL,SW_HIDE);

				return 0;
			}

			bool RemoveAppFromFirewall(LPCWSTR filename, LPCWSTR title)
			{
				tstring vistaCmd=String::Format(L"advfirewall firewall delete rule name=\"%s\"",title);
				::ShellExecute(NULL, L"open", L"netsh.exe",vistaCmd.c_str(),NULL,SW_HIDE);

				tstring xpCmd=String::Format(L"firewall delete allowedprogram program=\"%s\"",filename);
				::ShellExecute(NULL, L"open", L"netsh.exe",xpCmd.c_str(),NULL,SW_HIDE);

				return 0;
			}
		}
	}
}