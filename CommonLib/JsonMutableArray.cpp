#include "stdafx.h"
#include "JsonMutableArray.h"

using namespace web::json;

namespace CommonLib
{
	namespace Utility
	{
		namespace Json
		{
			JsonMutableArray::JsonMutableArray() :JsonMutableArray(web::json::value::array())
			{
			}

			JsonMutableArray::JsonMutableArray(const web::json::value& jsonSource) 
				: JsonArray(jsonSource)
			{}

			JsonMutableArray::~JsonMutableArray()
			{
			}

			void JsonMutableArray::AddValue(const bool value)
			{
				jsonValues[Count()] = value::boolean(value);
			}
			
			void JsonMutableArray::AddValue(const int value)
			{
				jsonValues[Count()] = value::number(value);
			}

			void JsonMutableArray::AddValue(const double value)
			{
				jsonValues[Count()] = value::number(value);
			}

			void JsonMutableArray::AddValue(LPCWSTR value)
			{
				jsonValues[Count()] = value::string(value);
			}

			void JsonMutableArray::AddValue(const tstring& value)
			{
				jsonValues[Count()] = value::string(value);
			}

			void JsonMutableArray::AddValue(const string& value)
			{
				jsonValues[Count()] = value::string(String::MultiByteToWideChar(value));
			}

			void JsonMutableArray::AddValue(const std::vector<tstring>& valueList)
			{
				for (const tstring& value:valueList)
				{
					jsonValues[Count()] = value::string(value);
				}
			}

			void JsonMutableArray::AddValue(const vector<BYTE>& value)
			{
				AddValue(Base64Encode(value));
			}

			void JsonMutableArray::AddValue(const JsonArray& value)
			{
				if (value.Count() == 0)
					return;

				jsonValues[Count()] = value.Source();
			}

			void JsonMutableArray::AddValue(const JsonDictionary& value)
			{
				if (value.Count() == 0)
					return;

				jsonValues[Count()] = value.Source();
			}
		}
	}
}