#pragma once

#ifdef _MEMORY_CHECK

class MemoryChecker
{    
public:    
	MemoryChecker()
	{
		//	_CRTDBG_CHECK_ALWAYS_DF //�Ҵ� �� �� ���� ���� üũ. �� �ɼ��� ���� ��������.
		//	_CRTDBG_DELAY_FREE_MEM_DF //free�� ������ ��Ų��. free�� �޸𸮿� ���� ���� ����
		_CrtSetDbgFlag(
			_CRTDBG_LEAK_CHECK_DF //���� �� �޸� ���� Ž��
			|_CRTDBG_ALLOC_MEM_DF
			);
	}
	~MemoryChecker()
	{
		//�޸� �ջ� Ž��
		_ASSERTE(_CrtCheckMemory());
	}
} _MemoryChecker;
#endif
