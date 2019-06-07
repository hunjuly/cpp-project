#include "stdafx.h"
#include "FFMpeg.h"

using namespace CommonLib::Utility;

void ReadMetaData(AVDictionary *m, std::wstring* target)
{
	if (!m)
		return;

	for (int i = 0; i < m->count; ++i)
	{
		AVDictionaryEntry *tag = m->elems + i;

		//if(strcmp(tag->key, "title") == 0)
		{
			target->append(String::MultiByteToWideChar(tag->value));
			target->append(L", ");
			//break;
		}
	}
}

HANDLE CreatePipe(const tstring& pipeName, SECURITY_ATTRIBUTES* sa);
HANDLE OpenStream(const tstring& pipeName, SECURITY_ATTRIBUTES* sa);
DWORD Read(BYTE* buffer, int bufferSize, DWORD waitingTime, HANDLE readPipe, SECURITY_ATTRIBUTES& sa);
void Close(HANDLE& readStream, HANDLE& readPipe, PROCESS_INFORMATION& pi);

vector<BYTE> CommandResult(tstring command)
{
	vector<BYTE> buffer(1024 * 1024);

	int writtenSize = 0;
	ExecuteFFMpeg(command, &buffer[0], buffer.size(), &writtenSize);
	buffer.resize(writtenSize);
	return buffer;
}

void ExecuteFFMpeg(const tstring& command, BYTE* buffer, int bufferSize, int* writtenSize)
{
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
	{
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;
	}

	tstring pipeName = String::Format(L"\\\\.\\pipe\\%s", CommonLib::Utility::CreateUuid().c_str());

	HANDLE readPipe = CreatePipe(pipeName, &sa);
	HANDLE readStream = INVALID_HANDLE_VALUE;

	if (readPipe == INVALID_HANDLE_VALUE)
	{
		Close(readStream, readPipe, pi);
		return;
	}

	readStream = OpenStream(pipeName, &sa);

	if (readStream == INVALID_HANDLE_VALUE)
	{
		Close(readStream, readPipe, pi);
		return;
	}

	STARTUPINFOA si;
	ZeroMemory(&si, sizeof(STARTUPINFOA));
	{
		si.cb = sizeof(STARTUPINFOA);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdOutput = readStream; //서버가 읽고 FFMpeg이 쓰는 것이다. 헷갈리지 말자
		si.hStdInput = NULL;
		si.hStdError = NULL;
	}

	string ffmpegCommand = String::WideCharToMultiByte(
		String::Format(L"%s\\FFMpegnGinSoft.exe %s", Path::ModuleFilePath().c_str(), command.c_str()));

	if (CreateProcessA(NULL, (LPSTR)ffmpegCommand.c_str(), &sa, &sa, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		if (WAIT_OBJECT_0 == ::WaitForSingleObject(pi.hProcess, 10 * 1000))
		{
			DWORD code;
			GetExitCodeProcess(pi.hProcess, &code);

			if (code == 0)
			{
				DWORD readSize = Read(&buffer[0], bufferSize, 10000, readPipe, sa);

				*writtenSize = readSize;
			}
		}
	}
	else //실패 이유는 권한 상승(UAC) 혹은 ffmpeg가 경로에 없어서, 아니면 ffmpeg.exe가 필요한 dll이 없어서
	{
		Close(readStream, readPipe, pi);

		throw Exceptions::Exception(L"FFMpeg Fail");
	}

	Close(readStream, readPipe, pi);
}

bool ExecuteFFMpeg(const tstring& command)
{
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
	{
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;
	}

	STARTUPINFOA si;
	ZeroMemory(&si, sizeof(STARTUPINFOA));
	{
		si.cb = sizeof(STARTUPINFOA);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdOutput = NULL;
		si.hStdInput = NULL;
		si.hStdError = NULL;
	}

	string ffmpegCommand = String::WideCharToMultiByte(String::Format(L"%s\\FFMpegnGinSoft.exe %s", Path::ModuleFilePath().c_str(), command.c_str()));

	if (CreateProcessA(NULL, (LPSTR)ffmpegCommand.c_str(), &sa, &sa, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		if (WAIT_OBJECT_0 == ::WaitForSingleObject(pi.hProcess, 10 * 60 * 1000))
		{
			DWORD code;
			GetExitCodeProcess(pi.hProcess, &code);

			return code == 0;
		}
	}

	return false;
}

HANDLE CreatePipe(const tstring& pipeName, SECURITY_ATTRIBUTES* sa)
{
	return CreateNamedPipe(
		pipeName.c_str(),                 // pipe name 
		FILE_FLAG_OVERLAPPED | PIPE_ACCESS_INBOUND,       // read/write access 
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		0,            // output buffer size 
		1024 * 1024,             // input buffer size 
		NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
		sa);                    // default security attribute 
}

HANDLE OpenStream(const tstring& pipeName, SECURITY_ATTRIBUTES* sa)
{
	return CreateFile(
		pipeName.c_str()
		, FILE_WRITE_DATA | SYNCHRONIZE
		, 0
		, sa
		, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL
		, 0
		);
}

void Close(HANDLE& readStream, HANDLE& readPipe, PROCESS_INFORMATION& pi)
{
	if (readStream != INVALID_HANDLE_VALUE)
	{
		CancelIo(readStream);
		CloseHandle(readStream);
		readStream = INVALID_HANDLE_VALUE;
	}

	if (readPipe != INVALID_HANDLE_VALUE)
	{
		CancelIo(readPipe);
		CloseHandle(readPipe);
		readPipe = INVALID_HANDLE_VALUE;
	}

	if (pi.hProcess) //INVALID_HANDLE_VALUE로 비교하지 않은 것이 맞다. ZeroMemory로 초기화 했다.
	{
		if (WAIT_TIMEOUT == ::WaitForSingleObject(pi.hProcess, 0))
			::TerminateProcess(pi.hProcess, 0);

		CloseHandle(pi.hProcess);
		pi.hProcess = 0;
	}

	if (pi.hThread)
	{
		CloseHandle(pi.hThread);
		pi.hThread = 0;
	}
}

DWORD Read(BYTE* buffer, int bufferSize, DWORD waitingTime, HANDLE readPipe, SECURITY_ATTRIBUTES& sa)
{
	DWORD readSize = 0;

	OVERLAPPED over = { 0 };
	over.hEvent = CreateEvent(&sa, TRUE, TRUE, 0);

	if (::ReadFile(readPipe, buffer, bufferSize, 0, &over))
		readSize = over.InternalHigh;

	CloseHandle(over.hEvent);

	return readSize;
}
