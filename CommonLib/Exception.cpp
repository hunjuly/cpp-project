#include "StdAfx.h"
#include "Exception.h"

namespace CommonLib
{
	namespace Exceptions
	{
		Exception::Exception(const tstring& message)
		{
			this->message = message;
		}

		Exception::~Exception(void)	{}

		tstring Exception::Message() const
		{
			return message;
		}

		LibraryException::LibraryException(LPCTSTR file, LPCTSTR func, int line) : Exception(L"")
		{
			this->file=file;
			this->func=func;
			this->line=line;
		}
		LibraryException::LibraryException(LPCTSTR file, LPCTSTR func, int line, const tstring& message) : Exception(message)
		{
			this->file=file;
			this->func=func;
			this->line=line;
		}
		LibraryException::~LibraryException(void){}

		tstring LibraryException::Message() const
		{
			return String::Format(_T("%s(%s(), %d line, %s)"), message.c_str(), func.c_str(), line, file.c_str());
		}

		UserOutputException::UserOutputException(const tstring& message) : Exception(message){}
		UserOutputException::~UserOutputException(void){}

	}
}
