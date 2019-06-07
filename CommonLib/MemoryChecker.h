#pragma once

#ifdef _MEMORY_CHECK

class MemoryChecker
{    
public:    
	MemoryChecker()
	{
		//	_CRTDBG_CHECK_ALWAYS_DF //할당 할 때 마다 오류 체크. 이 옵션은 많이 느려진다.
		//	_CRTDBG_DELAY_FREE_MEM_DF //free를 딜레이 시킨다. free된 메모리에 쓰는 오류 점검
		_CrtSetDbgFlag(
			_CRTDBG_LEAK_CHECK_DF //종료 시 메모리 누수 탐지
			|_CRTDBG_ALLOC_MEM_DF
			);
	}
	~MemoryChecker()
	{
		//메모리 손상 탐지
		_ASSERTE(_CrtCheckMemory());
	}
} _MemoryChecker;
#endif
