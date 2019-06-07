#include "stdafx.h"
#include "HttpBuffer.h"

namespace Http
{
	HttpBuffer::HttpBuffer()
	{
		Reset();
	}

	void HttpBuffer::Reset(int size)
	{
		writePos = 0;
		readPos = 0;

		bodyFilePosition = 0;
		postDataSize = 0;
		readPostDataPos = 0;

		data.swap(vector<BYTE>(size));
		currentBodyBuffer.swap(vector<BYTE>(size));

		if (data.size() > 0)
			ZeroMemory(&data[0], data.size());
	}

	const vector<BYTE>& HttpBuffer::GetData() const
	{
		//종종 헤더만 오고 PostData는 안 오는 경우가 있다.
		if (data.size() != writePos)
			return vector<BYTE>();

		return data;
	}

	vector<BYTE>::iterator HttpBuffer::First()
	{
		return data.begin();
	}

	vector<BYTE>::iterator HttpBuffer::Last()
	{
		return data.begin() + writePos;
	}

	void HttpBuffer::AddBuffer(HttpBuffer& sourceBuffer)
	{
		copy(sourceBuffer.First(), sourceBuffer.Last(), Last());

		IncreaseWritePos(sourceBuffer.writePos);
	}

	void HttpBuffer::WriteToCurrentPos(BYTE* buffer, int size)
	{
		if (size > 0)
		{
			memcpy(CurrentWritePosition(), buffer, size);

			IncreaseWritePos(size);
		}
	}

	void HttpBuffer::WriteHeader(BYTE* buffer, int size)
	{
		WriteToCurrentPos(buffer, size);
		headerSize = size;
	}

	void HttpBuffer::WriteBody(BYTE* buffer, int size)
	{
		WriteToCurrentPos(buffer, size);
	}

	BYTE* HttpBuffer::CurrentWritePosition()
	{
		return &data[writePos];
	}

	int HttpBuffer::RemainedWriteSize()
	{
		return data.size() - writePos;
	}

	void HttpBuffer::IncreaseWritePos(int size)
	{
		writePos += size;

		_ASSERT(writePos <= data.size());
	}

	BYTE* HttpBuffer::CurrentReadPosition()
	{
		return &data[readPos];
	}

	int HttpBuffer::RemainedReadSize()
	{
		return data.size() - readPos;
	}

	void HttpBuffer::IncreaseReadPos(int size)
	{
		readPos += size;

		_ASSERT(readPos <= data.size());
	}

	const vector<BYTE>& HttpBuffer::CurrentReadPostDataPosition()
	{
		currentBodyBuffer = bodyReader.Read(bodyFilePosition + readPostDataPos, min(RemainedReadPostDataSize(), 64 * 1024));

		return currentBodyBuffer;
	}

	void HttpBuffer::OpenBodyFile(wstring bodyFilePathName)
	{
		bodyReader.Open(bodyFilePathName);
	}

	void HttpBuffer::Close()
	{
		bodyReader.Close();

		//이걸 안 하면 버퍼가 그대로 남아서 메모리를 차지한다.
		Reset();
	}

	long long HttpBuffer::RemainedReadPostDataSize()
	{
		return postDataSize - readPostDataPos;
	}

	void HttpBuffer::IncreaseReadPostDataPos(int size)
	{
		readPostDataPos += size;
	}

	bool HttpBuffer::HasLine()
	{
		return find(First(), Last(), '\n') != Last();
	}

	vector<BYTE> HttpBuffer::GetBody()
	{
		vector<BYTE> body;

		copy(data.begin() + headerSize, data.end(), back_inserter(body));

		return body;
	}

	//\r\n으로 끝나는 한 문장을 리턴하고 삭제한다.
	tstring HttpBuffer::GetLineAndRemove()
	{
		tstring line(L"");

		auto lineEndPos = find(First(), Last(), '\n');

		if (lineEndPos != Last())
		{
			//\r\n에서 \r을 0으로 만든다.
			*(lineEndPos - 1) = 0;

			line = String::MultiByteToWideChar((LPCSTR)&data[0]);

			auto nextLineBeginPos = lineEndPos + 1;

			writePos = distance(nextLineBeginPos, Last());

			data.erase(First(), nextLineBeginPos);
		}

		return line;
	}
}