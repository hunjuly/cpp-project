#pragma once
#include <cpprest\json.h>

namespace CommonLib
{
	namespace Utility
	{
		namespace Json
		{
			class JsonDictionary
			{
			public:
				JsonDictionary(const tstring& jsonSource, bool* success);
				JsonDictionary::JsonDictionary(const web::json::value& jsonSource);
				JsonDictionary();
				virtual ~JsonDictionary();

				bool Boolean(const tstring& key) const;
				int Integer(const tstring& key) const;
				double Double(const tstring& key) const;
				tstring String(const tstring& key) const;
				tstring JsonDictionary::Path(const tstring& key) const;
				web::json::value Object(const tstring& key) const;

				bool IsExistKey(const tstring& key) const;
				bool IsNull(const tstring& key) const;
				int Count() const;

				const web::json::value& Source() const;
				string ToString() const;

			protected:
				web::json::value jsonValues;
				void Parse(const tstring& jsonSource, bool* success);
			};
		}
	}
}