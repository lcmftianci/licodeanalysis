#pragma once
#include "ReceivedData.h"

namespace vosvideo
{
	namespace data
	{
		class DeviceConfigurationMsg final : public ReceivedData
		{
		public:
			DeviceConfigurationMsg();
			virtual ~DeviceConfigurationMsg();

			virtual web::json::value ToJsonValue() const override;
			virtual void FromJsonValue(const web::json::value& obj) override;
			virtual std::wstring ToString() const override;
		};
	}
}