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
누가 써놓은 글을 긁어온 것이다.
LocalAlloc과 GlobalAlloc은 동일하므로 LocalAlloc만을 언급하겠다.

HeapAlloc과 LocalAlloc의 차이점을 살펴보자.
첫째로 메모리 블록을 할당하는 힙 자체가 다르다는 것이다. 
LocalAlloc은 항상 디폴트 힙에서 할당한다. 
디폴트 힙은 하나의 어플리케이션이 하나씩 가지는 메인 힙이다. 
이는 프로세스가 생성되고 가상 주소 공간이 생겨날 때 같이 생성되는 힙이다. 
그 크기는 링크시 실행파일 헤더에 기록이 된다. 링크 옵션으로 그 크기를 변경할 수도 있다. 
기본 값은 1MB이다. 반면에 HeapAlloc은 함수의 첫 번째 인자에서 보듯이 임의의 힙에서 할당한다. 
HeapCreate로 만든 힙일 수도 있고 GetProcessHeap으로 얻는 디폴트 힙일 수도 있다.

둘째로 리턴값을 살펴보면 HeapAlloc은 포인터를 리턴하지만 LocalAlloc은 핸들을 리턴한다. 
따라서 LocalAlloc으로 할당받은 메모리 블록을 실제로 사용할 때는 LocalLock 함수로 포인터를 얻어와야 한다. 
LocalAlloc의 사용이 좀 더 불편하지만 핸들을 사용하므로써 동기화와 같은 기능을 제공한다. 
그리고 포인터를 직접 사용하지 않고 핸들을 사용하는 또 다른 (결과적으로) 이유는 다음과 같다.
LocalAlloc의 첫번째 인자는 할당받을 메모리 블록의 특성을 지정하는데, GMEM_MOVEABLE 이라는 특성을 지정할 수 있다. 
이는 힙 메모리 단편화 제거를 위해 또는 힙 공간이 부족할 때 힙 관리자가 메모리 블록을 이동할 수 있도록 하게 하는데 
이렇게 되면 포인터로 리턴했을 경우 이동된 후 잘못된 참조를 하게 되므로 핸들로서 관리되는 것이다. 
실제로 GMEM_MOVEABLE 이 아닌 GMEM_FIXED로 지정하게 되면 리턴값은 그 자체가 포인터이다.

※ 참고로 C run-time library인 malloc은 위의 API 함수들과 같은 기능을 하지만 
디폴트 힙이 아닌 자신만의 힙을 사용하고 자신만의 방법으로 힙을 관리한다.
*/