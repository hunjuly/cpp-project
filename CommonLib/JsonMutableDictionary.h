#pragma once
#include <cpprest\json.h>
#include "JsonArray.h"
#include "JsonDictionary.h"

namespace CommonLib
{
	namespace Utility
	{
		namespace Json
		{
			class JsonMutableDictionary : public JsonDictionary
			{
			public:
				JsonMutableDictionary();
				JsonMutableDictionary(const tstring& jsonSource, bool* success);
				virtual ~JsonMutableDictionary();

				void Parse(const tstring& jsonSource, bool* success);

				void AddValue(const tstring& key, bool value);
				void AddValue(const tstring& key, int value);
				void AddValue(const tstring& key, double value);
				void AddValue(const tstring& key, WCHAR* value);
				void AddValue(const tstring& key, const tstring& value);
				void AddValue(const tstring& key, const vector<BYTE>& value);
				void AddValue(const tstring& key, const JsonArray& value);
				void AddValue(const tstring& key, const JsonDictionary& value);
				void AddValue(const tstring& key, const web::json::value& value);
				void AddPath(const tstring& key, const tstring& value);
			};
		}
	}
}