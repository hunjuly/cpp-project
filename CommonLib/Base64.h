#pragma once

namespace CommonLib
{
	namespace Utility
	{
		std::vector<BYTE> Base64Decode(const wstring& text);
		std::wstring Base64Encode(const std::vector<BYTE>& buffer);
	}
}