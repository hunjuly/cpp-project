#include "stdafx.h"
#include "String.h"

namespace CommonLib
{
	namespace String
	{
		// ... 을 인자로 받는 경우 첫 번째 인자의 크기는 반드시 4바이트(32bit 기준)가 와야 한다.
		//그렇지 않을 경우 가변 인자의 포인터를 제대로 얻지 못하고 결국 죽는 문제가 발생함
		tstring Format(LPCTSTR format, ...)
		{
			tstring message;

			va_list argPtr;
			va_start(argPtr, format);

			message = Format(format, argPtr);

			va_end(argPtr);

			return message;
		}

		tstring Format(LPCTSTR format, const va_list argPtr)
		{
			int formatMessageLength = _vsctprintf(format, argPtr);

			formatMessageLength += 1; // _vscprintf doesn't count terminating '\0'

			vector<TCHAR> buffer(formatMessageLength);

			_vstprintf_s( &buffer[0], formatMessageLength, format, argPtr);

			return &buffer[0];
		}

		bool IsEqual(const tstring& a, const tstring& b, bool isIgnoreCase)
		{
			if(isIgnoreCase)
				return _tcsicmp(a.c_str(), b.c_str())==0;
			else
				return _tcscmp(a.c_str(), b.c_str())==0;
		}

		string WideCharToMultiByte(const wstring& source)
		{
			return WideCharToMultiByte(CP_ACP, source);
		}

		string WideCharToMultiByte(UINT codePage, const wstring& source)
		{
			DWORD convertedLength=
				::WideCharToMultiByte(codePage, 0, source.c_str(), -1, NULL, 0, NULL, NULL);

			vector<char> convertedString(convertedLength);

			::WideCharToMultiByte(
				codePage
				, 0
				, source.c_str()
				, -1
				, &convertedString[0]
			, convertedString.size()
				, NULL
				, NULL
				);

			return &convertedString[0];
		}

		wstring MultiByteToWideChar(const string& source)
		{
			return MultiByteToWideChar(CP_ACP, source);
		}

		wstring MultiByteToWideChar(UINT codePage, const string& source)
		{
			DWORD convertedLength=
				::MultiByteToWideChar(codePage, 0, source.c_str(), -1, NULL, 0);

			vector<wchar_t> convertedString(convertedLength);

			::MultiByteToWideChar(
				codePage
				, 0
				, source.c_str()
				, -1
				, &convertedString[0]
			, convertedString.size()
				);

			return &convertedString[0];
		}

		vector<tstring> Tokenize(const tstring& source, const tstring& delimeter)
		{
			vector<tstring> tokenizedList;

			tstring::const_iterator currentIter = source.begin();

			tstring::const_iterator foundIter = search(currentIter, source.end(), delimeter.begin(),delimeter.end());

			tstring token;

			while(foundIter != source.end())
			{
				token=source.substr(currentIter-source.begin(), foundIter-currentIter);

				tokenizedList.push_back(token);

				currentIter=foundIter+delimeter.length();

				foundIter = search(currentIter, source.end(), delimeter.begin(),delimeter.end());
			}

			token=source.substr(currentIter-source.begin(), foundIter-currentIter);

			tokenizedList.push_back(token);

			return tokenizedList;
		}


		vector<tstring> Tokenize(const tstring& source, const WCHAR delimeter)
		{
			vector<tstring> tokenizedList;

			tstring::const_iterator currentIter = source.begin();

			tstring::const_iterator foundIter = find(currentIter, source.end(), delimeter);

			tstring token;

			while(foundIter != source.end())
			{
				token=source.substr(currentIter-source.begin(), foundIter-currentIter);

				tokenizedList.push_back(token);

				currentIter=foundIter+1;

				foundIter = find(currentIter, source.end(), delimeter);
			}

			token=source.substr(currentIter-source.begin(), foundIter-currentIter);

			tokenizedList.push_back(token);

			return tokenizedList;
		}

		vector<tstring> Tokenize(const TCHAR* unicodeSource, const int length, const WCHAR delimeter)
		{
			vector<tstring> tokenizedList;

			const TCHAR* currentIter = unicodeSource;

			const TCHAR* foundIter = find(currentIter, &unicodeSource[length], delimeter);

			tstring token;

			while(foundIter != &unicodeSource[length])
			{
				token.clear();
				copy(currentIter, foundIter, back_inserter(token));

				tokenizedList.push_back(token);

				currentIter=foundIter+1;

				foundIter = find(currentIter, &unicodeSource[length], delimeter);
			}

			token.clear();
			copy(currentIter, foundIter, back_inserter(token));

			tokenizedList.push_back(token);

			return tokenizedList;
		}

		vector<tstring> Tokenize(const vector<TCHAR>& source, const TCHAR delimeter)
		{
			return Tokenize(&source[0],source.size(),delimeter);
		}

		tstring GetModuleMessage(const int messageId)
		{
			return GetModuleMessage(NULL, messageId);
		}

		tstring GetModuleMessage(LPCTSTR moduleName, const int messageId)
		{
			const int bufferSize=2048;
			vector<TCHAR> messageBuffer;
			messageBuffer.resize(bufferSize);

			if(0==FormatMessage(
				FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS
				, ::GetModuleHandle(moduleName)
				, messageId
				, 0
				, &messageBuffer[0]
			, bufferSize
				, NULL)
				)
			{
				THROW_LIBRARY(Diagnostics::GetLastErrorMessage());
			}

			return &messageBuffer[0];
		}

		tstring EraseFrontBlank(const tstring& text)
		{
			tstring erasedText=text;

			while(String::Length(erasedText)>0)
			{
				if(erasedText[0]==' ')
					erasedText.erase(erasedText.begin() ,erasedText.begin()+1);
				else
					break;
			}

			return erasedText;
		}

		int Length(const tstring& text)
		{
			return _tcsnlen(text.c_str(), text.size());
		}

		int Length(const string& text)
		{
			return strnlen(text.c_str(), text.size());
		}

		tstring  MakeUpper(const tstring& text)
		{
			int length=String::Length(text);

			vector<WCHAR> buffer(length + 1);
			wcscpy_s(&buffer[0], buffer.size(), text.c_str());

			_wcsupr_s(&buffer[0], buffer.size());

			return &buffer[0];
		}

		int Find(LPCWSTR source, const tstring& target)
		{
			const TCHAR* pos=_tcsstr(source, target.c_str());

			return pos?pos-source:-1;
		}

		int Find(const tstring& source, const tstring& target)
		{
			return Find(source.c_str(),target);
		}

		WCHAR UpperCase(WCHAR aChar)
		{
			return (aChar>='a'&&'z'>=aChar)?aChar+'A'-'a':aChar;
		}

		int FindIgnoreCase(LPCWSTR source, const tstring& target)
		{
			if(String::Length(target)==0)
				return -1;

			tstring upperTarget=MakeUpper(target);

			LPCWSTR current=source;

			while(*current!=0)
			{
				if(upperTarget.at(0)==UpperCase(*current))
				{
					current++;

					bool found=true;

					for(int i=1;i<String::Length(upperTarget);++i)
					{
						if(upperTarget.at(i)!=UpperCase(*current))
						{
							found=false;
							break;
						}
						current++;
					}

					if(found)
						return current-source-String::Length(upperTarget);
				}
				else
					current++;
			}

			return -1;
		}

		int FindIgnoreCase(const tstring& source, const tstring& target)
		{
			return FindIgnoreCase(source.c_str(),target);
		}

		tstring ReplaceIgnoreCase(LPCWSTR source, int sourceSize, const tstring& target, const tstring& replace)
		{
			tstring buffer;
			int targetLength=String::Length(target);

			LPCWSTR current=source;

			int foundPos=FindIgnoreCase(current, target);

			while(foundPos!=-1)
			{
				buffer.append(current, foundPos);
				current+=foundPos;

				buffer.append(replace);
				current+=targetLength;

				foundPos=FindIgnoreCase(current, target);
			}

			buffer.append(current);

			return buffer;
		}

		tstring ReplaceIgnoreCase(const tstring& source,const tstring& target, const tstring& replace)
		{
			return ReplaceIgnoreCase(source.c_str(),String::Length(source),target,replace);
		}

		tstring Replace(tstring source, const tstring& target, const tstring& replaceString)
		{
			std::string::size_type offset = 0;     
			while( true )    
			{        
				// 검색.        
				offset = source.find( target, offset );                   
				if( tstring::npos == offset )                       
					break;                 
				else                   
					// 찾은 문자열을 바꿀 문자열로 바꿈.            
					source.replace( offset, target.length(), replaceString );
			} 

			return source;
		}

		int MakeInt(const tstring& text)
		{
			return StrToInt(text.c_str());
		}

		long long MakeInt64(const tstring& text)
		{
			long long value = 0;

			StrToInt64Ex(text.c_str(), STIF_DEFAULT, &value);

			return value;
		}
		
		tstring StringFrom(int64_t value)
		{
			return Format(L"%lld", value);
		}
	}
}