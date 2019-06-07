#pragma once

namespace CommonLib
{
	namespace Memory
	{
		class MovableHeap
		{
		public:
			MovableHeap(int size);
			~MovableHeap(void);

			PVOID Lock();
			void Unlock();

		private:
			HGLOBAL hgbl;
		};
	}
}