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

			//TODO vector�� ����� �޽����� ������ �ִٰ�. �Ҹ� �� �� �Ѳ����� ����Ѵ�.
		public:
			void SetFile(const tstring& messageFilePath);
			void ToFile(const tstring& message);
			void ToFile(const tstring& filePath, const tstring& message);
			//void Flush();
		};
	}
}
