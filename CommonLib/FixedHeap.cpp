#include "StdAfx.h"
#include "FixedHeap.h"

namespace CommonLib
{
	namespace Memory
	{
		FixedHeap::FixedHeap(int size)
		{
			addr=HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
			
			CHECK_NULL_THROW_LAST_ERROR(addr);
		}

		FixedHeap::~FixedHeap(void)
		{
			BOOL result=HeapFree(GetProcessHeap(), 0, addr);

			CHECK_NULL_THROW_LAST_ERROR(result);
		}

		LPVOID FixedHeap::GetPointer()
		{
			return addr;
		}
	}
}

/*
���� ����� ���� �ܾ�� ���̴�.
LocalAlloc�� GlobalAlloc�� �����ϹǷ� LocalAlloc���� ����ϰڴ�.

HeapAlloc�� LocalAlloc�� �������� ���캸��.
ù°�� �޸� ����� �Ҵ��ϴ� �� ��ü�� �ٸ��ٴ� ���̴�. 
LocalAlloc�� �׻� ����Ʈ ������ �Ҵ��Ѵ�. 
����Ʈ ���� �ϳ��� ���ø����̼��� �ϳ��� ������ ���� ���̴�. 
�̴� ���μ����� �����ǰ� ���� �ּ� ������ ���ܳ� �� ���� �����Ǵ� ���̴�. 
�� ũ��� ��ũ�� �������� ����� ����� �ȴ�. ��ũ �ɼ����� �� ũ�⸦ ������ ���� �ִ�. 
�⺻ ���� 1MB�̴�. �ݸ鿡 HeapAlloc�� �Լ��� ù ��° ���ڿ��� ������ ������ ������ �Ҵ��Ѵ�. 
HeapCreate�� ���� ���� ���� �ְ� GetProcessHeap���� ��� ����Ʈ ���� ���� �ִ�.

��°�� ���ϰ��� ���캸�� HeapAlloc�� �����͸� ���������� LocalAlloc�� �ڵ��� �����Ѵ�. 
���� LocalAlloc���� �Ҵ���� �޸� ����� ������ ����� ���� LocalLock �Լ��� �����͸� ���;� �Ѵ�. 
LocalAlloc�� ����� �� �� ���������� �ڵ��� ����ϹǷν� ����ȭ�� ���� ����� �����Ѵ�. 
�׸��� �����͸� ���� ������� �ʰ� �ڵ��� ����ϴ� �� �ٸ� (���������) ������ ������ ����.
LocalAlloc�� ù��° ���ڴ� �Ҵ���� �޸� ����� Ư���� �����ϴµ�, GMEM_MOVEABLE �̶�� Ư���� ������ �� �ִ�. 
�̴� �� �޸� ����ȭ ���Ÿ� ���� �Ǵ� �� ������ ������ �� �� �����ڰ� �޸� ����� �̵��� �� �ֵ��� �ϰ� �ϴµ� 
�̷��� �Ǹ� �����ͷ� �������� ��� �̵��� �� �߸��� ������ �ϰ� �ǹǷ� �ڵ�μ� �����Ǵ� ���̴�. 
������ GMEM_MOVEABLE �� �ƴ� GMEM_FIXED�� �����ϰ� �Ǹ� ���ϰ��� �� ��ü�� �������̴�.

�� ����� C run-time library�� malloc�� ���� API �Լ���� ���� ����� ������ 
����Ʈ ���� �ƴ� �ڽŸ��� ���� ����ϰ� �ڽŸ��� ������� ���� �����Ѵ�.
*/