#include "StdAfx.h"
#include "FileWriter.h"

namespace CommonLib
{
	namespace Io
	{
		namespace File
		{
			FileWriter::FileWriter()
			{
				hFile = INVALID_HANDLE_VALUE;
			}

			FileWriter::FileWriter(const tstring& filePath, bool isNewFile)
			{
				hFile = INVALID_HANDLE_VALUE;

				if (isNewFile)
					Create(filePath);
				else
					Open(filePath);
			}

			FileWriter::~FileWriter()
			{
				Close();
			}

			void FileWriter::MoveToEnd()
			{
				::SetFilePointer(hFile, Size(), NULL, FILE_BEGIN);
			}

			HANDLE FileWriter::GetHandle()
			{
				return hFile;
			}

			void FileWriter::WriteLine(const tstring& buffer)
			{
				int len = String::Length(buffer) * sizeof(TCHAR);

				Write(buffer.c_str(), len);

				const tstring& newLine = _T("\r\n");

				Write(newLine.c_str(), String::Length(newLine) *sizeof(TCHAR));
			}

			void FileWriter::Write(LPCVOID buffer, int bufferSize)
			{
				if (0 < bufferSize)
				{
					DWORD writtenSize = 0;

					if (FALSE == WriteFile(hFile, buffer, bufferSize, &writtenSize, NULL))
					{
						THROW_LIBRARY(String::Format(_T("file write fail.(%s)"), Diagnostics::GetLastErrorMessage().c_str()));
					}
				}
			}

			void FileWriter::Write(vector<BYTE> buffer)
			{
				if (0 < buffer.size())
					Write(&buffer[0], buffer.size());
			}

			void FileWriter::Write(string buffer)
			{
				if (0 < buffer.size())
					Write(buffer.c_str(), buffer.length());
			}

			void FileWriter::Create(const tstring& filePath)
			{
				_ASSERT_EXPR(hFile == INVALID_HANDLE_VALUE, L"always opened.");

				hFile = CreateFile(
					filePath.c_str(),                // name of the write
					GENERIC_WRITE,          // open for writing
					FILE_SHARE_READ,
					NULL,                   // default security
					CREATE_ALWAYS,          // overwrite existing
					FILE_ATTRIBUTE_NORMAL,  // normal file
					NULL);                  // no attr. template

				if (hFile == INVALID_HANDLE_VALUE)
				{
					THROW_LIBRARY(String::Format(_T("file open fail.(%s)"), Diagnostics::GetLastErrorMessage().c_str()));
				}
			}

			void FileWriter::Open(const tstring& filePath)
			{
				_ASSERT_EXPR(hFile == INVALID_HANDLE_VALUE, L"always opened.");

				hFile = CreateFile(
					filePath.c_str(),                // name of the write
					GENERIC_WRITE,          // open for writing
					FILE_SHARE_READ,
					NULL,                   // default security
					OPEN_ALWAYS,          // overwrite existing
					FILE_ATTRIBUTE_NORMAL,  // normal file
					NULL);                  // no attr. template

				if (hFile == INVALID_HANDLE_VALUE)
				{
					THROW_LIBRARY(String::Format(_T("file open fail.(%s)"), Diagnostics::GetLastErrorMessage().c_str()));
				}

				MoveToEnd();
			}

			void FileWriter::Close()
			{
				if (hFile != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;
				}
			}

			LONGLONG FileWriter::Size()
			{
				LARGE_INTEGER fileSize;
				GetFileSizeEx(hFile, &fileSize);

				return fileSize.QuadPart;
			}
		}
	}
}