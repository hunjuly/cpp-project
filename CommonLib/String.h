#pragma once

namespace CommonLib
{
	namespace String
	{
		tstring Format(LPCTSTR format, ...);
		tstring Format(LPCTSTR format, const va_list argPtr);

		string WideCharToMultiByte(const wstring& source);
		string WideCharToMultiByte(UINT codePage, const wstring& source);

		wstring MultiByteToWideChar(const string& source);
		wstring MultiByteToWideChar(UINT codePage, const string& source);

		vector<tstring> Tokenize(const tstring& source, const WCHAR delimeter);
		vector<tstring> Tokenize(const TCHAR* unicodeSource, const int length, const WCHAR delimeter);
		vector<tstring> Tokenize(const vector<TCHAR>& source, const TCHAR delimeter);
		vector<tstring> Tokenize(const tstring& source, const tstring& delimeter);

		tstring GetModuleMessage(const int messageId);
		tstring GetModuleMessage(LPCTSTR moduleName, const int messageId);

		bool IsEqual(const tstring& a, const tstring& b,bool isIgnoreCase=false);

		tstring EraseFrontBlank(const tstring& text);

		int Length(const tstring& text);
		int Length(const string& text);
		int Find(LPCWSTR source, const tstring& target);
		int Find(const tstring& source, const tstring& target);
		int FindIgnoreCase(LPCWSTR source, const tstring& target);
		int FindIgnoreCase(const tstring& source, const tstring& target);
		tstring ReplaceIgnoreCase(const tstring& source,const tstring& target, const tstring& replace);
		tstring ReplaceIgnoreCase(LPCWSTR source, int sourceSize, const tstring& target, const tstring& replace);
		tstring Replace(tstring source, const tstring& target, const tstring& replaceString);

		int MakeInt(const tstring& text);
		long long MakeInt64(const tstring& text);
		tstring StringFrom(int64_t value);
	}
}
