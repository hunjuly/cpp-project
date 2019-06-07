#pragma once

//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>    

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

//Platform SDK header files
#include <windows.h>

#include <Winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <mswsock.h>
#pragma comment(lib, "Mswsock.lib")

#include <ws2tcpip.h>

#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

#include <Wininet.h>
#pragma comment(lib, "Wininet.lib")

#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#pragma comment(lib, "Advapi32.lib")

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

#pragma comment(lib, "user32.lib")

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include <tchar.h>

//STL header files
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <map>
#include <set>
#include <stack>
#include <memory>
#include <queue>
#include <functional>
using namespace std;

typedef basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > tstring;
typedef basic_stringstream<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > tstringstream;

#include <assert.h>
#include <ctype.h>

#ifndef DEBUG_NEW
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)    
#endif

#define MINIUPNP_STATICLIB
#include "../MiniUPnP/miniwget.h"
#include "../MiniUPnP/miniupnpc.h"
#include "../MiniUPnP/upnpcommands.h"
#include "../MiniUPnP/upnperrors.h"
#include "../MiniUPnP/miniupnpcstrings.h"

#ifdef _DEBUG
#ifdef X64
#pragma comment(lib, "../_Output/Debug/x64/miniupnpc.lib")
#else
#pragma comment(lib, "../_Output/Debug/Win32/miniupnpc.lib")
#endif
#endif //_DEBUG

#ifdef NDEBUG
#ifdef X64
#pragma comment(lib, "../_Output/Release/x64/miniupnpc.lib")
#else
#pragma comment(lib, "../_Output/Release/Win32/miniupnpc.lib")
#endif
#endif

#include "String.h"
#include "Exception.h"
#include "Diagnostics.h"
#include "MemoryChecker.h"
#include "MiniDump.h"
#include "ErrorMessage.h"
#include "MessageWriter.h"
#include "DebugInfo.h"

#include "FileReader.h"
#include "FileWriter.h"
#include "IRequestable.h"
#include "RequestHandler.h"

#include "Thread.h"
#include "CriticalSection.h"
#include "Mutex.h"
#include "SRWLock.h"
#include "MovableHeap.h"
#include "FixedHeap.h"
#include "Network.h"
#include "ConnectionListener.h"
#include "Transfer.h"
#include "TransferPool.h"
#include "Connection.h"
#include "Socket.h"
#include "SyncTransfer.h"
#include "uPnP.h"

#include "Service.h"
#include "Firewall.h"
#include "System.h"
#include "Process.h"

#include "Utility.h"
#include "Path.h"
#include "Base64.h"
#include "JsonArray.h"
#include "JsonMutableArray.h"
#include "JsonDictionary.h"
#include "JsonMutableDictionary.h"
