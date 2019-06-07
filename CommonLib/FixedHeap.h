#pragma once

namespace CommonLib
{
	namespace Memory
	{
		class FixedHeap
		{
		public:
			FixedHeap(int size);
			~FixedHeap(void);

			LPVOID GetPointer();
		
		private:
			PVOID addr;
		};
	}
}