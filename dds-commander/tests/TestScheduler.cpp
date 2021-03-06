// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>
// DDS
#include "AgentChannel.h"
#include "HostInfoCmd.h"
#include "SSHScheduler.h"
#include "Topology.h"
// MiscCommon
#include "TimeMeasure.h"
// BOOST
#include "boost/asio.hpp"
// STD
#include <sstream>

using namespace std;
using namespace dds;
using namespace dds::commander_cmd;
using namespace dds::topology_api;
using namespace dds::user_defaults_api;
using namespace dds::protocol_api;
using namespace MiscCommon;

BOOST_AUTO_TEST_SUITE(test_dds_scheduler_performance)

BOOST_AUTO_TEST_CASE(test_dds_scheduler_performance_1)
{
    Logger::instance().init(); // Initialize log
    CUserDefaults::instance(); // Initialize user defaults

    boost::asio::io_service io_service;

    CTopology topology;
    topology.init("topology_scheduler_test_1.xml");

    CAgentChannel::connectionPtrVector_t agents;
    size_t nofTasks = 3;
    size_t nofAgentsPerTask = 1000;

    agents.reserve(nofTasks * nofAgentsPerTask);

    for (size_t i = 0; i < nofTasks; ++i)
        for (size_t j = 0; j < nofAgentsPerTask; ++j)
        {
            CAgentChannel::connectionPtr_t agent = CAgentChannel::makeNew(io_service);

            stringstream ss;
            ss << "host" << (i + 1) << "_" << j % 100;

            SHostInfoCmd hostInfo;
            hostInfo.m_host = ss.str();

            agent->setRemoteHostInfo(hostInfo);

            agents.push_back(agent);
        }

    CAgentChannel::weakConnectionPtrVector_t weakAgents;
    weakAgents.reserve(agents.size());
    for (auto& v : agents)
    {
        weakAgents.push_back(v);
    }

    CSSHScheduler scheduler;

    auto execTime = STimeMeasure<std::chrono::microseconds>::execution(
        [&scheduler, &topology, &weakAgents]() { scheduler.makeSchedule(topology, weakAgents); });
    double execTimeSeconds = execTime * 1e-6;

    BOOST_CHECK(execTimeSeconds < 3.0);

    std::cout << "SSH scheduler execution time: " << execTimeSeconds << " s\n";

    // Test failure
    // Change host info to non existing pattern
    for (auto agent : agents)
    {
        SHostInfoCmd hostInfo;
        hostInfo.m_host = "nohost";
        agent->setRemoteHostInfo(hostInfo);
    }
    auto execFailTime = STimeMeasure<std::chrono::microseconds>::execution([&scheduler, &topology, &weakAgents]() {
        BOOST_CHECK_THROW(scheduler.makeSchedule(topology, weakAgents), runtime_error);
    });
    double execFailTimeSeconds = execFailTime * 1e-6;

    BOOST_CHECK(execFailTimeSeconds < 0.1);
    std::cout << "SSH scheduler fail execution time: " << execFailTimeSeconds << " s\n";
}

void make_agent(boost::asio::io_service& _io_service,
                CAgentChannel::connectionPtrVector_t& _agents,
                const string& _hostName,
                const string& _workerId)
{
    CAgentChannel::connectionPtr_t agent = CAgentChannel::makeNew(_io_service);
    SHostInfoCmd hostInfo;
    hostInfo.m_host = _hostName;
    hostInfo.m_workerId = _workerId;
    agent->setRemoteHostInfo(hostInfo);
    _agents.push_back(agent);
}

BOOST_AUTO_TEST_CASE(test_dds_scheduler_1)
{
    Logger::instance().init(); // Initialize log
    CUserDefaults::instance(); // Initialize user defaults

    boost::asio::io_service io_service;

    CTopology topology;
    topology.init("topology_scheduler_test_2.xml");

    CAgentChannel::connectionPtrVector_t agents;

    size_t n = 3;
    agents.reserve(n * 9);
    for (size_t i = 0; i < n; ++i)
    {

        string indexStr = to_string(i);
        // Requirement type "hostname"
        make_agent(io_service, agents, "host1_" + indexStr, "wn1");
        make_agent(io_service, agents, "host2_" + indexStr, "wn2");
        make_agent(io_service, agents, "host3_" + indexStr, "wn3");
        make_agent(io_service, agents, "host4_" + indexStr, "wn4");
        make_agent(io_service, agents, "host4_" + indexStr, "wn4");
        make_agent(io_service, agents, "host4_" + indexStr, "wn4");

        // Requirement type "wnname"
        make_agent(io_service, agents, "host5_0", "wn5");
        make_agent(io_service, agents, "host6_0", "wn6");

        // No Requirement
        make_agent(io_service, agents, "noname_host", "noname_wn");
    }

    CAgentChannel::weakConnectionPtrVector_t weakAgents;
    weakAgents.reserve(agents.size());
    for (auto& v : agents)
    {
        weakAgents.push_back(v);
    }

    CSSHScheduler scheduler;
    scheduler.makeSchedule(topology, weakAgents);
    cout << scheduler.toString();
}

BOOST_AUTO_TEST_SUITE_END()
