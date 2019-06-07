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
			class JsonMutableArray : public JsonArray
			{
			public:
				JsonMutableArray();
				JsonMutableArray(const web::json::value& jsonSource);
				virtual ~JsonMutableArray();

				void AddValue(const bool value);
				void AddValue(const int value);
				void AddValue(const double value);
				void AddValue(LPCWSTR value);
				void AddValue(const tstring& value);
				void AddValue(const string& value);
				void AddValue(const vector<BYTE>& value);
				void AddValue(const std::vector<tstring>& valueList);
				void AddValue(const JsonArray& value);
				void AddValue(const JsonDictionary& value);
			};
		}
	}
}