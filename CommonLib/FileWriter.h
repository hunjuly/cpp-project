#pragma once

namespace CommonLib
{
	namespace Io
	{
		namespace File
		{
			class FileWriter
			{
			private:
				HANDLE hFile; 

				void MoveToEnd();

			public:
				FileWriter();
				FileWriter(const tstring& filePath, bool isNewFile=true);
				~FileWriter();

				void Open(const tstring& filePath);
				void Create(const tstring& filePath);
				void Close();

				LONGLONG Size();

				HANDLE GetHandle();

				void WriteLine(const tstring& buffer);
				void Write(LPCVOID buffer, int bufferSize);
				void Write(vector<BYTE> buffer);				
				void Write(string buffer);
			};
		}
	}
}