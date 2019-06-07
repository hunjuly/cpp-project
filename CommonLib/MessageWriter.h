#pragma once

namespace CommonLib
{
	namespace Diagnostics
	{
		class MessageWriter
		{
		public:
			static MessageWriter& Instance();

		private:
			MessageWriter(const MessageWriter&); 
			MessageWriter& operator=(const MessageWriter&); 
			MessageWriter(void);
			~MessageWriter(void);

			tstring messageFilePath; 

			//TODO vector로 기록할 메시지를 가지고 있다가. 소멸 될 때 한꺼번에 기록한다.
		public:
			void SetFile(const tstring& messageFilePath);
			void ToFile(const tstring& message);
			void ToFile(const tstring& filePath, const tstring& message);
			//void Flush();
		};
	}
}
