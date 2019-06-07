#pragma once
#include "RequestHandler.h"

namespace CommonLib
{
	namespace Io
	{
		namespace Iocp
		{
			class IRequestable : public OVERLAPPED
			{
			public:
				virtual void RequestProcessed(int receivedBytes, LPOVERLAPPED pOverlapped)=0;

				HANDLE Handle() const
				{
					return handle;
				}

				SOCKET Socket() const
				{
					return (SOCKET)handle;
				}

			protected:
				IRequestable(RequestHandler& requester):
					requester(requester)
					,handle(INVALID_HANDLE_VALUE)
				{
				}

				IRequestable(HANDLE handle, RequestHandler& requester):
					requester(requester)
					,handle(INVALID_HANDLE_VALUE)
				{
					Initialize(handle);
				}

				void ResetHandle()
				{
					this->handle=INVALID_HANDLE_VALUE;
				}

				void Initialize(SOCKET handle)
				{
					Initialize((HANDLE)handle);
				}

				void Initialize(HANDLE handle)
				{
					_ASSERT(this->handle==INVALID_HANDLE_VALUE);

					this->handle=handle;

					requester.RegisterHandle(handle, this);
				}

				virtual ~IRequestable(){}

				RequestHandler& GetRequestHandler() const
				{
					return requester;
				}

				bool IsAvailable() const
				{
					return this->handle!=INVALID_HANDLE_VALUE;
				}

			protected:
				RequestHandler& requester;
				HANDLE handle;
			};
		}
	}
}
