// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TalkToAgent.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;

int CTalkToAgent::on_cmdHANDSHAKE(const CProtocolMessage& _msg)
{
    SVersionCmd ver;
    ver.convertFromData(_msg.bodyToContainer());
    // send shutdown if versions are incompatible
    if (ver != SVersionCmd())
    {
        m_isHandShakeOK = false;
        // Send reply that the version of the protocol is incompatible
        LOG(warning) << "Client's protocol version is incompatable. Client: "
                     << socket().remote_endpoint().address().to_string();

        pushMsg<cmdREPLY_ERR_BAD_PROTOCOL_VERSION>();
    }
    else
    {
        m_isHandShakeOK = true;
        // everything is OK, we can work with this agent
        LOG(info) << "The Agent [" << socket().remote_endpoint().address().to_string()
                  << "] has succesfully connected.";

        pushMsg<cmdREPLY_HANDSHAKE_OK>();
    }
    return 0;
}

int CTalkToAgent::on_cmdHANDSHAKE_AGENT(const CProtocolMessage& _msg)
{
    SVersionCmd ver;
    ver.convertFromData(_msg.bodyToContainer());
    // send shutdown if versions are incompatible
    if (ver != SVersionCmd())
    {
        m_isHandShakeOK = false;
        // Send reply that the version of the protocol is incompatible
        LOG(warning) << "Client's protocol version is incompatable. Client: "
                     << socket().remote_endpoint().address().to_string();
        CProtocolMessage msg;
        msg.encode<cmdREPLY_ERR_BAD_PROTOCOL_VERSION>();
        pushMsg(msg);
    }
    else
    {
        m_isHandShakeOK = true;
        // everything is OK, we can work with this agent
        LOG(info) << "The Agent [" << socket().remote_endpoint().address().to_string()
                  << "] has succesfully connected.";

        pushMsg<cmdREPLY_HANDSHAKE_OK>();
        pushMsg<cmdGET_HOST_INFO>();
    }
    return 0;
}

int CTalkToAgent::on_cmdSUBMIT(const CProtocolMessage& _msg)
{
    SSubmitCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());
    LOG(info) << "Recieved a Submit of the topo [" << cmd.m_sTopoFile
              << "] command from: " << socket().remote_endpoint().address().to_string();

    // TODO: Implement me. So far we always send OK
    SSimpleMsgCmd msg_cmd;
    msg_cmd.m_sMsg = "Dummy job info, JOBIds";
    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdREPLY_SUBMIT_OK>(msg_cmd);
    pushMsg(msg);

    return 0;
}

int CTalkToAgent::on_cmdREPLY_HOST_INFO(const CProtocolMessage& _msg)
{
    SHostInfoCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

    LOG(info) << "Recieved a HostInfo [" << cmd
              << "] command from: " << socket().remote_endpoint().address().to_string();

    pushMsg<cmdDISCONNECT>();

    return 0;
}

int CTalkToAgent::on_cmdGED_PID(const CProtocolMessage& _msg)
{
    pid_t pid = getpid();
    SSimpleMsgCmd cmd_attachment;
    stringstream ss;
    ss << pid;
    cmd_attachment.m_sMsg = ss.str();
    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdREPLY_PID>(cmd_attachment);
    pushMsg(msg);

    return 0;
}
