#pragma once

namespace Http
{
	class Response;
	class Receive;
}

namespace Service
{
	class IService
	{
	public:
		virtual bool IsCanProcess(const tstring& serviceName) = 0;
		virtual void ProcessRequest(const Http::Receive& receive, Http::Response* response) = 0;

		virtual void Initialize();
		virtual void Uninitialize();
	};
}
