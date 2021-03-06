#pragma once
#include <cpprest/json.h>
#include "ConnectionProblemNotifier.h"
#include <ppltasks.h>

namespace vosvideo
{
	namespace communication
	{
		class HttpClientEngine : public ConnectionProblemNotifier
		{
		public:
			HttpClientEngine();
			virtual ~HttpClientEngine();
			virtual concurrency::task<web::json::value> Get(const std::wstring& path) = 0;
			virtual concurrency::task<web::json::value> Post(const std::wstring& path, const web::json::value& payload) = 0;
		};
	}
}
