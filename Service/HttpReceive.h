#pragma once

namespace Http
{
	class Receive
	{
	public:
		void AddHeader(const tstring& header);
		void FinishAddHeader();
		void Reset();
		void SetPostData(const vector<BYTE>& postDataBuffer);
		void SetPostData(const tstring& postData);
		bool HasHeader() const;
		void Log();
		tstring ValueInHeaders(const tstring& targetHeader) const;

		int contentLength;
		tstring postData;
		vector<tstring> urlPath;
		tstring serviceType;
		const Utility::Json::JsonDictionary& JsonData() const;

	private:
		std::unique_ptr<Utility::Json::JsonDictionary> jsonData;
		tstring RequestedResource() const;
		void SetContentLength();
		void SetServiceCategoryAndName();
		tstring StringFromPostData(const vector<BYTE>& postDataBuffer);

	private:
		vector<tstring> headers;
	};
}