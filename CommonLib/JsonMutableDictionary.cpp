#include "stdafx.h"
#include "JsonMutableDictionary.h"
#include "Exception.h"

using namespace web::json;

namespace CommonLib
{
	namespace Utility
	{
		namespace Json
		{
			JsonMutableDictionary::JsonMutableDictionary()
			{}
			JsonMutableDictionary::~JsonMutableDictionary()
			{}
			JsonMutableDictionary::JsonMutableDictionary(const tstring& jsonSource, bool* success)
				: JsonDictionary(jsonSource, success)
			{}

			void JsonMutableDictionary::Parse(const tstring& jsonSource, bool* success)
			{
				JsonDictionary::Parse(jsonSource, success);
			}

			void JsonMutableDictionary::AddValue(const tstring& key, bool value)
			{
				if(IsExistKey(key))
					THROW_LIBRARY(L"exist Key");

				jsonValues[key] = value::boolean(value);
			}

			void JsonMutableDictionary::AddValue(const tstring& key, int value)
			{
				if(IsExistKey(key))
					THROW_LIBRARY(L"exist Key");

				jsonValues[key] = value::number(value);
			}

			void JsonMutableDictionary::AddValue(const tstring& key, double value)
			{
				if(IsExistKey(key))
					THROW_LIBRARY(L"exist Key");

				jsonValues[key] = value::number(value);
			}

			void JsonMutableDictionary::AddValue(const tstring& key, WCHAR* value)
			{
				if(IsExistKey(key))
					THROW_LIBRARY(L"exist Key");

				jsonValues[key] = value::string(value);
			}

			void JsonMutableDictionary::AddValue(const tstring& key, const tstring& value)
			{
				if(IsExistKey(key))
					THROW_LIBRARY(L"exist Key");

				jsonValues[key] = value::string(value);
			}

			void JsonMutableDictionary::AddValue(const tstring& key, const vector<BYTE>& value)
			{
				if(IsExistKey(key))
					THROW_LIBRARY(L"exist Key");

				jsonValues[key] = value::string(Base64Encode(value));
			}

			void JsonMutableDictionary::AddPath(const tstring& key, const tstring& value)
			{
				if(IsExistKey(key))
					THROW_LIBRARY(L"exist Key");

				JsonMutableArray path;

				auto folderList = String::Tokenize(value, '\\');

				for(auto folder : folderList)
				{
					if(0 < folder.length())
						path.AddValue(folder);
				}

				jsonValues[key] = path.Source();
			}

			void JsonMutableDictionary::AddValue(const tstring& key, const JsonArray& value)
			{
				if(value.Count() == 0)
					return;

				if(IsExistKey(key))
					THROW_LIBRARY(L"exist Key");

				jsonValues[key] = value.Source();
			}

			void JsonMutableDictionary::AddValue(const tstring& key, const JsonDictionary& value)
			{
				if(value.Count() == 0)
					return;

				if(IsExistKey(key))
					THROW_LIBRARY(L"exist Key");

				jsonValues[key] = value.Source();
			}

			void JsonMutableDictionary::AddValue(const tstring& key, const web::json::value& value)
			{
				if (IsExistKey(key))
					THROW_LIBRARY(L"exist Key");

				jsonValues[key] = value;
			}
		}
	}
}