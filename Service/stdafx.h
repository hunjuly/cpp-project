// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <stdio.h>

// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
#define _MEMORY_CHECK
#define _CRTDBG_MAP_ALLOC
#include "../Libraries/CommonLib/CommonLib.h"
using namespace CommonLib;


#ifdef _DEBUG
#ifdef X64
#pragma comment(lib, "../_Output/Debug/x64/CommonLib.lib")
#else
#pragma comment(lib, "../_Output/Debug/Win32/CommonLib.lib")
#endif
#endif //_DEBUG

#ifdef NDEBUG
#ifdef X64
#pragma comment(lib, "../_Output/Release/x64/CommonLib.lib")
#else
#pragma comment(lib, "../_Output/Release/Win32/CommonLib.lib")
#endif
#endif

#ifdef X64
#pragma comment(lib, "../Libraries/ffmpeg/x64/lib/avcodec.lib")
#pragma comment(lib, "../Libraries/ffmpeg/x64/lib/avformat.lib")
#pragma comment(lib, "../Libraries/ffmpeg/x64/lib/avutil.lib")
#else
#pragma comment(lib, "../Libraries/ffmpeg/x86/lib/avcodec.lib")
#pragma comment(lib, "../Libraries/ffmpeg/x86/lib/avformat.lib")
#pragma comment(lib, "../Libraries/ffmpeg/x86/lib/avutil.lib")
#endif

//������� �� ������Ʈ ����
#include <queue>

#include "Config.h"

#pragma comment(lib, "mpr.lib")
#include <Winnetwk.h>
