#include "StdAfx.h"
#include "ServiceHandler.h"
#include "HttpResponse.h"
#include "Config.h"
#include "ContentsInfoService.h"
#include "VideoStreamingService.h"
#include "MovieEncodingService.h"
#include "SystemService.h"

namespace Service
{
	ServiceHandler& ServiceHandler::Instance()
	{
		static ServiceHandler instance;

		return instance;
	}

	ServiceHandler::ServiceHandler(void)
	{
		servicePool.push_back(unique_ptr<IService>(new ContentsInfoService()));
		servicePool.push_back(unique_ptr<IService>(new VideoStreamingService()));

		auto movieEncodingService = new MovieEncodingService();
		servicePool.push_back(unique_ptr<IService>(movieEncodingService));
		servicePool.push_back(unique_ptr<IService>(new SystemService(movieEncodingService)));
	}

	void ServiceHandler::Initialize()
	{
		for (const auto& service : servicePool)
		{
			service->Initialize();
		}

		isServiceLoad = true;
	}

	void ServiceHandler::Uninitialize()
	{
		isServiceLoad = false;

		for (const auto& service : servicePool)
		{
			service->Uninitialize();
		}
	}

	bool ServiceHandler::HasPassword(const Http::Receive& receive)  const
	{
		return String::IsEqual(receive.serviceType, L".htm", true);
	}

	void ServiceHandler::ProcessRequest(const Http::Receive& receive, Http::Response* response)
	{
		try
		{
			using namespace CommonLib::Utility;

			wstring product = L"";

			if (HasPassword(receive))
			{
				if (receive.JsonData().IsExistKey(L"Product"))
					product = receive.JsonData().String(L"Product");

				if (false == IsCorrectPassword(receive.JsonData().String(L"Password")))
					return response->IncorrectPassword();
				else if (false == IsCorrectVersion(product, receive.JsonData().Integer(L"Version")))
					return response->IncorrectVersion();
			}

			if (isServiceLoad)
			{
				for (const auto& service : servicePool)
				{
					if (service->IsCanProcess(receive.urlPath[0]))
					{
						service->ProcessRequest(receive, response);
					}
				}
			}
			else
			{
				response->RequestedJobFailed();
			}
		}
		catch (const Exceptions::Exception& ex)
		{
			response->RequestedJobFailed();
		}
		catch (const exception& ex)
		{
			response->RequestedJobFailed();
		}
		catch (...)
		{
			response->RequestedJobFailed();
		}
	}

	bool ServiceHandler::IsCorrectPassword(const tstring& password) const
	{
		return (String::IsEqual(Config::Instance().Password, password));
	}

	bool ServiceHandler::IsCorrectVersion(const tstring& product, const int version) const
	{
		if (String::IsEqual(L"nGinVideo", product))
		{
			return version == 200;
		}
		else if (String::IsEqual(L"nGinMediaConsole", product))
		{
			return version == 300;
		}

		return false;
	}
}
