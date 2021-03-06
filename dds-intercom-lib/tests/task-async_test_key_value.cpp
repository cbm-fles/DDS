// DDS
#include "Logger.h"
#include "dds_env_prop.h"
#include "dds_intercom.h"
// STD
#include <condition_variable>
#include <exception>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
namespace bpo = boost::program_options;
using namespace MiscCommon;

const size_t g_maxValue = 1000;
const size_t g_maxWaitTime = 100; // milliseconds

int main(int argc, char* argv[])
{
    try
    {
        string sKey;
        size_t nInstances(0);
        size_t nMaxValue(g_maxValue);
        size_t nMaxWaitTime(g_maxWaitTime);
        size_t sleepTime(0);

        // Generic options
        bpo::options_description options("task-async_test_key_value options");
        options.add_options()("help,h", "Produce help message");
        options.add_options()("key", bpo::value<string>(&sKey)->default_value("property1"), "key to update");
        options.add_options()(
            "instances,i", bpo::value<size_t>(&nInstances)->default_value(0), "A number of instances");
        options.add_options()(
            "max-value", bpo::value<size_t>(&nMaxValue)->default_value(g_maxValue), "A max value of the property");
        options.add_options()("max-wait-time",
                              bpo::value<size_t>(&nMaxWaitTime)->default_value(g_maxWaitTime),
                              "A max wait time (in milliseconds), which an instannces should wait before exit");
        options.add_options()(
            "sleep-time", bpo::value<size_t>(&sleepTime)->default_value(0), "sleep time after task finishes its work.");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        if (vm.count("help") || vm.empty())
        {
            cout << options;
            return false;
        }

        // Named mutex
        const string taskID = env_prop<task_name>();
        if (taskID.empty())
            throw runtime_error("USER TASK: Can't initialize semaphore because DDS_TASK_ID variable is not set");

        // Test dds env_prop
        size_t nCollectionIdx = env_prop<collection_index>();
        cout << "TaskID = " << env_prop<task_name>() << "\nTask Index = " << env_prop<task_index>()
             << "\nCollection Index = " << nCollectionIdx << "\n";

        // TODO: document the test workflow
        // The test workflow
        // #1.

        CKeyValue keyValue;
        mutex keyMutex;
        condition_variable keyCondition;

        bool bGoodToGo = false;

        // container
        typedef set<string /*prop values*/> val_t;
        typedef map<string /*propname*/, val_t> container_t;
        container_t valContainer;

        // Subscribe on key update events
        keyValue.subscribe(
            [&keyCondition, &keyMutex, &valContainer, &bGoodToGo, &nMaxValue](const string& _key, const string _value) {
                LOG(debug) << "USER TASK received key update notification";
                {
                    unique_lock<mutex> lk(keyMutex);
                    // Add new value

                    val_t* values = &valContainer[_key];
                    values->insert(_value);
                    // check wheather all values are already there
                    for (const auto& prop : valContainer)
                    {
                        // LOG(info) << "prop.second.size()=" << prop.second.size() <<
                        bGoodToGo = (prop.second.size() == nMaxValue);
                        if (!bGoodToGo)
                            break;
                    }
                }
                keyCondition.notify_all();
            });

        const std::chrono::seconds waitingTime(10);
        this_thread::sleep_for(waitingTime);

        for (size_t i = 0; i < nMaxValue; ++i)
        {
            LOG(debug) << "USER TASK is going to set new value " << i;
            const string sCurValue = to_string(i);
            const int retVal = keyValue.putValue(sKey, sCurValue);
            LOG(debug) << "USER TASK put value return code: " << retVal;
        }

        while (true)
        {
            unique_lock<mutex> lk(keyMutex);
            if (bGoodToGo)
            {
                LOG(log_stdout) << "Task succesffuylly done";
                if (sleepTime > 0)
                {
                    LOG(log_stdout) << "Task is waiting for " << sleepTime << " sec before exit";
                    sleep(sleepTime);
                }
                return 0;
            }
            // wait for a key update event
            auto now = chrono::system_clock::now();
            int isTimeout = keyCondition.wait_until(lk, now + chrono::milliseconds(nMaxWaitTime)) == cv_status::timeout;
            if (bGoodToGo)
            {
                LOG(log_stdout) << "Task succesffuylly done";
                if (sleepTime > 0)
                {
                    LOG(log_stdout) << "Task is waiting for " << sleepTime << " sec before exit";
                    sleep(sleepTime);
                }
                return 0;
            }
            if (isTimeout)
            {
                LOG(log_stderr) << "USER TASK - timeout is reached, but not all values have been received.";

                return 1;
            }
        }
    }
    catch (const exception& _e)
    {
        LOG(log_stderr) << "USER TASK Error: " << _e.what() << endl;
        return 1;
    }
    return 1;
}
