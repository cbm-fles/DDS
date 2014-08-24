// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "VersionCmd.h"
// DDS
#include "ProtocolCommands.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
namespace inet = MiscCommon::INet;

SVersionCmd::SVersionCmd()
    : m_version(g_protocolCommandsVersion)
{
}

void SVersionCmd::normalizeToLocal()
{
    m_version = inet::_normalizeRead16(m_version);
}

void SVersionCmd::normalizeToRemote()
{
    m_version = inet::_normalizeWrite16(m_version);
}

void SVersionCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "VersionCmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
        throw runtime_error(ss.str());
    }

    m_version = _data[0];
    m_version += (_data[1] << 8);
}

void SVersionCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    _data->push_back(m_version & 0xFF);
    _data->push_back(m_version >> 8);
}