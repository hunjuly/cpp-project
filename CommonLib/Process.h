#pragma once

namespace CommonLib
{
	namespace System
	{
		class Process
		{
		public:
			Process();
			~Process();

			void Start(const string& command);
			void Start(const wstring& command);
			void Stop();
			void Pause();
			void Resume();
			HANDLE Handle();

			//BOOL WaitComplete(DWORD waitingTime);
			int ReturnCode();
			bool IsAlive() const;

			static void KillProcess(wstring filename);

		private:
			PROCESS_INFORMATION pi;
		};
	}
}