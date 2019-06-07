#include "stdafx.h"
#include "HttpReceive.h"

namespace Http
{
	void Receive::AddHeader(const tstring& header)
	{
		headers.push_back(header);

		if(HasHeader())
			FinishAddHeader();
	}

	void Receive::FinishAddHeader()
	{
		SetContentLength();
		SetServiceCategoryAndName();
	}

	void Receive::Reset()
	{
		contentLength = 0;
		urlPath.clear();
		serviceType = L"";
		headers.clear();
		postData = L"";
	}

	void Receive::Log()
	{
		for_each(headers.begin(), headers.end(), [&](tstring header)
		{
			Debug::Output(header);
		});

		Debug::Output(postData);
	}

	void Receive::SetContentLength()
	{
		tstring contentLength = ValueInHeaders(L"Content-Length");

		this->contentLength = (contentLength.length() > 0) ? StrToInt(contentLength.c_str()) : 0;
	}

	void Receive::SetServiceCategoryAndName()
	{
		urlPath = String::Tokenize(RequestedResource(), '/');

		serviceType = ::PathFindExtension(RequestedResource().c_str());
	}

	tstring Receive::RequestedResource() const
	{
		//POST /Video/GetFileList.htm HTTP/1.1\r\n
		//GET /stream.m3u8 HTTP/1.1
		//GET /stream-2.ts HTTP/1.1
		vector<tstring> wordList = String::Tokenize(headers[0], L' ');

		if(wordList.size() < 2)
			return L"";

		wordList[1].erase(0, 1); // /stream에서 /을 지운다.

		return wordList[1];
	}

	tstring Receive::ValueInHeaders(const tstring& targetHeader) const
	{
		for(size_t i = 0; i<headers.size(); ++i)
		{
			tstring header = headers[i];

			if(String::Length(header)>String::Length(targetHeader))
			{
				tstring subHeader = header.substr(0, String::Length(targetHeader));

				if(String::IsEqual(subHeader, targetHeader, true))
				{
					int pos = header.find(':');

					tstring value = header.substr(pos + 1);

					return String::EraseFrontBlank(value);
				}
			}
		}

		return L"";
	}

	tstring Receive::StringFromPostData(const vector<BYTE>& postDataBuffer)
	{
		vector<BYTE> newData(postDataBuffer.size() + 2);
		copy(postDataBuffer.begin(), postDataBuffer.end(), newData.begin());
		newData[newData.size() - 1] = 0;
		newData[newData.size() - 2] = 0;

		return String::MultiByteToWideChar(CP_UTF8, (LPCSTR)&newData[0]);
	}

	void Receive::SetPostData(const vector<BYTE>& postDataBuffer)
	{
		if(contentLength > 0)
		{
			this->postData = StringFromPostData(postDataBuffer);

			bool success;
			jsonData.reset(new Utility::Json::JsonDictionary(postData, &success));
		}
	}

	void Receive::SetPostData(const tstring& postData)
	{
		this->postData = postData;

		bool success;
		jsonData.reset(new Utility::Json::JsonDictionary(postData, &success));
	}

	bool Receive::HasHeader() const
	{
		if(headers.size() == 0)
			return false;

		return headers.back().size() == 0;
	}

	const Utility::Json::JsonDictionary& Receive::JsonData() const
	{
		if(jsonData.get() == nullptr)
			THROW_LIBRARY(L"empty data");

		return *jsonData;
	}
}
