#pragma once

#include "HttpBuffer.h"

namespace Http
{
	class Response
	{
	public:
		void Reset();
		void Log();

		HttpBuffer data;

		void Ok();
		void IncorrectVersion();
		void IncorrectPassword();
		void RequestedJobFailed();
		void NotFound();
		void WithContent(const tstring& body);
		void WithContent(const string& body);
		void WithContent(const Utility::Json::JsonDictionary& body);
		void WithContent(const Utility::Json::JsonArray& body);
		void WithContent(const vector<BYTE>& body);
		void WithContent(const char* type, const vector<BYTE>& body);
		void WithContent(const char* type, const tstring& body);
		void WithContent(const char* type, const char* body, int bodySize);
		void WithFile(wstring filePathName, long long filePosition);

	private:
		void Custom(int code, const char* message);
	};

}