#include "stdafx.h"
#include "JsonArray.h"

using namespace web::json;

namespace CommonLib
{
	namespace Utility
	{
		namespace Json
		{
			JsonArray::JsonArray(const web::json::value& jsonSource)
			{
				this->jsonValues = jsonSource;
			}

			JsonArray::JsonArray(const tstring& jsonSource, bool* success)
			{
				Parse(jsonSource, success);
			}

			void JsonArray::Parse(const tstring& jsonSource, bool* success)
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

			JsonArray::~JsonArray(){}

			int JsonArray::Count() const
			{
				return jsonValues.size();
			}

			bool JsonArray::Boolean(const int index) const
			{
				try
				{
					return jsonValues.at(index).as_bool();
				}
				catch (json_exception& ex)
				{
					THROW_LIBRARY(L"index/type mismatch");
				}
			}

			int JsonArray::Integer(const int index) const
			{
				try
				{
					return jsonValues.at(index).as_integer();
				}
				catch (json_exception& ex)
				{
					THROW_LIBRARY(L"index/type mismatch");
				}
			}

			double JsonArray::Double(const int index) const
			{
				try
				{
					return jsonValues.at(index).as_double();
				}
				catch (json_exception& ex)
				{
					THROW_LIBRARY(L"index/type mismatch");
				}
			}

			tstring JsonArray::String(const int index) const
			{
				try
				{
					return jsonValues.at(index).as_string();
				}
				catch (json_exception& ex)
				{
					THROW_LIBRARY(L"index/type mismatch");
				}
			}

			tstring JsonArray::Path(const int index) const
			{
				try
				{
					tstring path;

					web::json::value pathList = jsonValues.at(index);

					for(int i = 0; i < pathList.size(); ++i)
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

			web::json::value JsonArray::Object(const int index) const
			{
				try
				{
					return jsonValues.at(index);
				}
				catch (json_exception& ex)
				{
					THROW_LIBRARY(L"index/type mismatch");
				}
			}

			const web::json::value& JsonArray::Source() const
			{
				return jsonValues;
			}

			string JsonArray::ToString() const
			{
				//크기가 0이면 array.ToString()이 ']'을 리턴한다. 그러면 에러다.
				//그래서 강제로 ""을 리턴함
				if (Count() == 0)
				{
					return "";
				}

				wstring wstr = jsonValues.serialize();
				string str = String::WideCharToMultiByte(CP_UTF8, wstr);

				return str;

				//stringstream str;
				//jsonValues.serialize(str);

				//return str.str();
			}
		}
	}
}
