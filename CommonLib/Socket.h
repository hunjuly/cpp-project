#pragma once

namespace CommonLib
{
	namespace Network
	{
		namespace Sync
		{
			class Socket
			{
			public:
				Socket(SOCKET socket);
				virtual ~Socket(void);

				SOCKET Get();

				bool IsConnected();
				void Close();
				operator SOCKET();
			protected:
				SOCKET socket;

			private:
				Socket(const Socket&);
			};
		}
	}
}
