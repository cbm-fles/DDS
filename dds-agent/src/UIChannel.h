// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_UIChannel_h
#define DDS_UIChannel_h

// DDS
#include "ServerChannelImpl.h"

namespace dds
{
    namespace agent_cmd
    {
        class CUIChannel : public protocol_api::CServerChannelImpl<CUIChannel>
        {
          private:
            CUIChannel(boost::asio::io_service& _service);
            std::string _remoteEndIDString();

          public:
            BEGIN_MSG_MAP(CUIChannel)
            MESSAGE_HANDLER(cmdUPDATE_KEY, on_cmdUPDATE_KEY)
            MESSAGE_HANDLER(cmdCUSTOM_CMD, on_cmdCUSTOM_CMD)
            END_MSG_MAP()

          private:
            // Message Handlers
            bool on_cmdUPDATE_KEY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment);
            bool on_cmdCUSTOM_CMD(protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment);
        };
    }
}

#endif
