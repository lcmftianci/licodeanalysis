#pragma once
#include "JsonObjectBase.h"
#include "WebSocketMessageParser.h"

namespace vosvideo
{
	namespace data
	{
		class ReceivedData : public JsonObjectBase
		{
		public:
			ReceivedData();
			virtual ~ReceivedData();
			virtual void Init(std::shared_ptr<WebSocketMessageParser> parser);

			virtual web::json::value ToJsonValue() const override;
			// Takes payload only, not whole message
			virtual std::wstring GetPayload();
			// Takes whole message, for serialization and retransmit 
			virtual std::wstring ToString() const;

			virtual std::wstring GetFromPeer();
			virtual std::wstring GetToPeer();

		protected:
			std::shared_ptr<WebSocketMessageParser> _parser;
		};
	}
}

