#include "StdAfx.h"
#include "HttpResponse.h"

using namespace CommonLib::Utility;

namespace Http
{
	void Response::Reset()
	{
		data.Reset(0);
	}

	void Response::Log()
	{
		//Debug::OutputA(header);
		//Debug::Output(String::Format(L"PostData Size=%d",body.size()));
	}

	void Response::Custom(int code, const char* message)
	{
		char header[1024];

		_snprintf_s(&header[0], 1024, 1024,
			"HTTP/1.1 %d %s\r\n"
			"Server: nGinMediaServer/3.0\r\n"
			"Connection: close\r\n"
			"\r\n"
			, code, message);

		int headerSize = strlen(header);

		this->data.Reset(headerSize);
		this->data.WriteHeader((BYTE*)header, headerSize);
	}

	void Response::IncorrectPassword()
	{
		Custom(600, "Incorrect password");
	}

	void Response::IncorrectVersion()
	{
		Custom(601, "Incorrect Version");
	}

	void Response::RequestedJobFailed()
	{
		Custom(602, "Requested job failed.");
	}

	void Response::Ok()
	{
		Custom(200, "Ok");
	}

	void Response::NotFound()
	{
		Custom(404, "Page Not Found");
	}

	void Response::WithContent(const tstring& body)
	{
		WithContent("text/plain", (const char*)body.c_str(), body.length() * 2);
	}

	void Response::WithContent(const string& body)
	{
		WithContent("text/plain", (const char*)body.c_str(), body.length());
	}

	void Response::WithContent(const Json::JsonDictionary& body)
	{
		WithContent(body.ToString());
	}

	void Response::WithContent(const Json::JsonArray& body)
	{
		WithContent(body.ToString());
	}

	void Response::WithContent(const vector<BYTE>& body)
	{
		if(body.size() > 0)
		{
			WithContent("text/plain", (const char*)&body[0], body.size());
		}
		else
		{
			WithContent("text/plain", (const char*)"", 0);
		}
	}

	void Response::WithContent(const char* type, const vector<BYTE>& body)
	{
		if(body.size() > 0)
			WithContent(type, (const char*)&body[0], body.size());
		else
			WithContent(type, (const char*)"", 0);
	}

	void Response::WithContent(const char* type, const tstring& body)
	{
		WithContent(type, (const char*)body.c_str(), body.length() * 2);
	}

	void Response::WithContent(const char* type, const char* body, int bodySize)
	{
		char header[1024];

		_snprintf_s(&header[0], 1024, 1024,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: %s\r\n"
			"Accept-Ranges: bytes\r\n"
			"Server: nGinMediaServer/1.0\r\n"
			"Connection: close\r\n"
			"Content-Length: %d\r\n"
			"\r\n"
			, type, bodySize);

		int headerSize = strlen(header);

		this->data.Reset(headerSize + bodySize);
		this->data.WriteHeader((BYTE*)header, headerSize);
		this->data.WriteBody((BYTE*)body, bodySize);
	}

	void Response::WithFile(wstring filePathName, long long filePosition)
	{
		int64_t fileSize = Path::FileSize(filePathName);
		int64_t bodySize = fileSize - filePosition;

		char header[1024];

		_snprintf_s(&header[0], 1024, 1024,
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: application/octet-stream\r\n"
					"Accept-Ranges: bytes\r\n"
					"Server: nGinMediaServer/1.0\r\n"
					"Connection: close\r\n"
					"Content-Length: %lld\r\n"
					"\r\n"
					, bodySize);

		int headerSize = strlen(header);

		this->data.Reset(headerSize);
		this->data.OpenBodyFile(filePathName);
		this->data.postDataSize = bodySize;
		this->data.bodyFilePosition = filePosition;
		this->data.WriteHeader((BYTE*)header, headerSize);
	}
}
