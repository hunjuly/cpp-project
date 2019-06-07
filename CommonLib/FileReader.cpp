#include "StdAfx.h"
#include "FileReader.h"

namespace CommonLib
{
	namespace Io
	{
		namespace File
		{
			FileReader::FileReader()
			{
				hFile = INVALID_HANDLE_VALUE;
			}

			FileReader::FileReader(const tstring& filePath)
			{
				hFile = INVALID_HANDLE_VALUE;

				Open(filePath);
			}

			FileReader::~FileReader()
			{
				Close();
			}

			tstring FileReader::ReadLine()
			{
				vector<BYTE> buffer;

				if(hasMore()==false)
					return L"";

				while(hasMore())
				{
					BYTE data=Read();

					if(data=='\r')
					{
						//\r이면 이 뒤로 0 \n 0이 더 있는 거다.
						Read();Read();Read();
						buffer.push_back(0);
						buffer.push_back(0);
						break;
					}
					else if(data=='\n')
					{
						//\n이면 이 뒤로 0 더 있는 거다.
						Read();
						buffer.push_back(0);
						buffer.push_back(0);
						break;
					}
					else
						buffer.push_back(data);
				}

				return (LPCTSTR)&buffer[0];
			}

			bool FileReader::hasMore()
			{
				return CurrentPosition()<Size();
			}

			vector<BYTE> FileReader::ReadToEnd()
			{
				return move(Read(0, Size()));
			}

			void FileReader::Read(LONGLONG pos, int size, vector<BYTE>& buffer)
			{
				buffer.resize(size);

				if (size == 0)
					return;

				LARGE_INTEGER newPointer;
				LARGE_INTEGER posPointer;
				posPointer.QuadPart=pos;

				SetFilePointerEx(hFile, posPointer, &newPointer, FILE_BEGIN);

				DWORD readSize=0;

				DWORD ret=ReadFile(hFile, &buffer[0], size, &readSize, NULL);

				buffer.resize(readSize);
			}

			BYTE FileReader::Read()
			{
				BYTE buffer;

				DWORD readSize=0;

				DWORD ret=ReadFile(hFile, &buffer, 1, &readSize, NULL);

				return buffer;
			}

			vector<BYTE> FileReader::Read(LONGLONG pos, int size)
			{
				vector<BYTE> buffer;

				Read(pos, size, buffer);

				return move(buffer);
			}

			void FileReader::Open(const tstring& filePath)
			{
				if (hFile!=INVALID_HANDLE_VALUE) 
				{ 
					THROW_LIBRARY(String::Format(_T("the file(%s) is always opened."), filePath.c_str()));
				}

				hFile = CreateFile(
					filePath.c_str(),                // name of the write
					GENERIC_READ,          // open for writing
					FILE_SHARE_READ|FILE_SHARE_WRITE,
					NULL,                   // default security
					OPEN_EXISTING,          // overwrite existing
					FILE_ATTRIBUTE_NORMAL,  // normal file
					NULL);                  // no attr. template

				if (hFile == INVALID_HANDLE_VALUE) 
				{ 
					THROW_LIBRARY(String::Format(_T("file open fail.(%s)"), Diagnostics::GetLastErrorMessage().c_str()));
				}

				fileName=filePath;
			}

			void FileReader::Close()
			{
				//TODO System::Handle 클래스를 사용해야 한다.
				if(hFile != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;
				}
			}

			LONGLONG FileReader::Size()
			{
				LARGE_INTEGER fileSize;
				GetFileSizeEx(hFile,&fileSize);

				return fileSize.QuadPart;
			}

			long long FileReader::CurrentPosition()
			{
				LARGE_INTEGER ret;

				LARGE_INTEGER pos;
				pos.QuadPart=0;

				SetFilePointerEx(hFile,pos,&ret,FILE_CURRENT);

				return ret.QuadPart;
			}
		}
	}
}