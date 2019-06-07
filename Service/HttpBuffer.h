#pragma once

namespace Http
{
	class HttpBuffer
	{
	public:
		HttpBuffer();

		void Reset(int size=4096);
		const vector<BYTE>& GetData() const;
		vector<BYTE> GetBody();

		tstring GetLineAndRemove();

		bool HasLine();
		vector<BYTE>::iterator First();
		vector<BYTE>::iterator Last();

		void AddBuffer(HttpBuffer& sourceBuffer);
		void WriteHeader(BYTE* buffer, int size);
		void WriteBody(BYTE* buffer, int size);


		BYTE* CurrentWritePosition();
		int RemainedWriteSize();
		void IncreaseWritePos(int size=1);

		BYTE* CurrentReadPosition();
		int RemainedReadSize();
		void IncreaseReadPos(int size);

		const vector<BYTE>& CurrentReadPostDataPosition();
		long long RemainedReadPostDataSize();
		void IncreaseReadPostDataPos(int size);

		long long postDataSize;
		long long bodyFilePosition;

		void OpenBodyFile(wstring bodyFilePathName);
		void Close();

	private:
		vector<BYTE> currentBodyBuffer;
		long long readPostDataPos;
		CommonLib::Io::File::FileReader bodyReader;

		vector<BYTE> data;
		int writePos;
		int readPos;
		int headerSize;

		void WriteToCurrentPos(BYTE* buffer, int size);
	};
}