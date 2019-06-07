#pragma once
#include <cpprest\json.h>

namespace CommonLib
{
	namespace Utility
	{
		namespace Json
		{
			class JsonArray
			{
			public:
				JsonArray(const web::json::value& jsonSource);
				JsonArray(const tstring& jsonSource, bool* success);
				virtual ~JsonArray();

				bool Boolean(const int index) const;
				int Integer(const int index) const;
				double Double(const int index) const;
				tstring String(const int index) const;
				web::json::value Object(const int index) const;
				tstring Path(const int index) const;

				int Count() const;

				const web::json::value& Source() const;

				string ToString() const;

			protected:
				void Parse(const tstring& jsonSource, bool* success);
				web::json::value jsonValues;
			};
		}
	}
}
