#include "stdafx.h"
#include "JsonDictionary.h"
#include "Exception.h"

using namespace web::json;

namespace CommonLib
{
	namespace Utility
	{
		namespace Json
		{
			JsonDictionary::JsonDictionary(const tstring& jsonSource, bool* success) :JsonDictionary()
			{
				Parse(jsonSource, success);
			}

			JsonDictionary::JsonDictionary(const web::json::value& jsonSource)
			{
				this->jsonValues = jsonSource;
			}

			void JsonDictionary::Parse(const tstring& jsonSource, bool* success)
			{
				try
				{
					jsonValues = value::parse(jsonSource);
					*success = true;
				}
				catch (json_exception& ex)
				{
					*success = false;
				}
			}

			JsonDictionary::JsonDictionary(){}
			JsonDictionary::~JsonDictionary(){}

			bool JsonDictionary::Boolean(const tstring& key) const
			{
				try
				{
					return jsonValues.at(key).as_bool();
				}
				catch (json_exception& ex)
				{
					THROW_LIBRARY(L"key not found or not bool");
				}
			}

			int JsonDictionary::Integer(const tstring& key) const
			{
				try
				{
					return jsonValues.at(key).as_integer();
				}
				catch (json_exception& ex)
				{
					THROW_LIBRARY(L"key not found or not bool");
				}
			}

			double JsonDictionary::Double(const tstring& key) const
			{
				try
				{
					return jsonValues.at(key).as_double();
				}
				catch (json_exception& ex)
				{
					THROW_LIBRARY(L"key not found or not bool");
				}
			}

			tstring JsonDictionary::String(const tstring& key) const
			{
				try
				{
					return jsonValues.at(key).as_string();
				}
				catch (json_exception& ex)
				{
					THROW_LIBRARY(L"key not found or not bool");
				}
			}

			tstring JsonDictionary::Path(const tstring& key) const
			{
				try
				{
					tstring path;

					web::json::value pathList = jsonValues.at(key);

					for(int i = 0; i < pathList.size();++i)
					{
						path = Path::Combine(path, pathList[i].as_string());
					}

					return path;
				}
				catch(json_exception& ex)
				{
					THROW_LIBRARY(L"key not found or not bool");
				}
			}

			web::json::value JsonDictionary::Object(const tstring& key) const
			{
				try
				{
					return jsonValues.at(key);
				}
				catch (json_exception& ex)
				{
				}

				return web::json::value();
			}

			bool JsonDictionary::IsExistKey(const tstring& key) const
			{
				bool r = jsonValues.has_field(key);
				return r;
			}

			bool JsonDictionary::IsNull(const tstring& key) const
			{
				return jsonValues.at(key).is_null();
			}

			const web::json::value& JsonDictionary::Source() const
			{
				return jsonValues;
			}

			int JsonDictionary::Count() const
			{
				return jsonValues.size();
			}

			string JsonDictionary::ToString() const
			{	
				if(Count() == 0)
				{
					return "";
				}

				//stringstream str;
				wstring wstr = jsonValues.serialize();
				
				//jsonValues.serialize(str);
				string str = String::WideCharToMultiByte(CP_UTF8, wstr);

				return str;
				//return str.str();
			}
		}
	}
}