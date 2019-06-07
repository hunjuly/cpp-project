#pragma once

namespace CommonLib
{
	namespace Io
	{
		namespace File
		{
			class FileReader
			{
			private:
				HANDLE hFile; 

			public:
				tstring fileName;
				FileReader();
				FileReader(const tstring& filePath);
				~FileReader();

				tstring ReadLine();
				vector<BYTE> Read(LONGLONG pos, int size);
				void Read(LONGLONG pos, int size, vector<BYTE>& buffer);
				vector<BYTE> ReadToEnd();
				BYTE Read();
				bool hasMore();

				void Open(const tstring& filePath);
				void Close();
				LONGLONG Size();
				long long CurrentPosition();
			};
		}
	}
}