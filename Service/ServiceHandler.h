#pragma once
#include "IService.h"
#include "HttpResponse.h"
#include "HttpReceive.h"

namespace Service
{
	class ServiceHandler
	{
	public:
		static ServiceHandler& Instance();
		void ProcessRequest(const Http::Receive& receive, Http::Response* response);
		void Initialize();
		void Uninitialize();
		
	private:
		ServiceHandler(void);

		vector<unique_ptr<IService>> servicePool;

		bool IsCorrectPassword(const tstring& password) const;
		bool HasPassword(const Http::Receive& receive)  const;
		bool IsCorrectVersion(const tstring& product, const int version) const;


		bool isServiceLoad = false;
	};
}