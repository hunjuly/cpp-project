#include "stdafx.h"
#include "MessageWriter.h"

namespace CommonLib
{
	namespace Diagnostics
	{
		MessageWriter::MessageWriter(void){}
		MessageWriter::~MessageWriter(void){}

		MessageWriter& MessageWriter::Instance()
		{
			static MessageWriter messageWriterPtr;

			return messageWriterPtr;
		}

		void MessageWriter::SetFile(const tstring& messageFilePath)
		{
			this->messageFilePath = messageFilePath;
		}

		void MessageWriter::ToFile(const tstring& message)
		{
			ToFile(messageFilePath.c_str(), message);
		}

		void MessageWriter::ToFile(const tstring& messageFilePath, const tstring& message)
		{
			Io::File::FileWriter logFile(messageFilePath);

			int logFileSize = logFile.Size();

			bool isNeedUnicodeHeader = (logFileSize == 0 && sizeof(TCHAR) >= 2)?true:false;

			if(isNeedUnicodeHeader)
			{
				unsigned char unicodeHeader[]={0xFF,0xFE,0};

				logFile.Write(unicodeHeader, strlen((char*)unicodeHeader));
			}

			logFile.WriteLine(message);			
		}
	}
}
