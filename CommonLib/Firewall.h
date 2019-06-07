#pragma once

namespace CommonLib
{
	namespace System
	{
		namespace Firewall
		{
			bool AddAppToFirewall(LPCWSTR filename, LPCWSTR title);
			bool RemoveAppFromFirewall(LPCWSTR filename, LPCWSTR title);
		}
	}
}