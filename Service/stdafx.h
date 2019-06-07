// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <stdio.h>

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
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

//여기부터 이 프로젝트 종속
#include <queue>

#include "Config.h"

#pragma comment(lib, "mpr.lib")
#include <Winnetwk.h>
