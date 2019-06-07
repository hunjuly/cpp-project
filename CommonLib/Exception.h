#pragma once

#include <stdio.h>

namespace CommonLib
{
	namespace Exceptions
	{
		class Exception
		{
		protected:
			tstring message;

		public:
			Exception(const tstring& message);
			virtual ~Exception(void);

			virtual tstring Message() const;
		};

		class LibraryException : public Exception
		{
		public:
			LibraryException(LPCTSTR file, LPCTSTR func, const int line);
			LibraryException(LPCTSTR file, LPCTSTR func, const int line, const tstring& message);
			virtual ~LibraryException(void);

			virtual tstring Message() const;

		private:
			tstring file;
			tstring func;
			int line;
		};

		class UserOutputException : public Exception
		{
		public:
			UserOutputException(const tstring& message);
			virtual ~UserOutputException(void);
		};
	}
}
