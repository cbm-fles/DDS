// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "Task.h"
#include "TaskGroup.h"
// STD
#include <sstream>
#include <string>
#include <iostream>
#include <memory>
// BOOST
#include <boost/property_tree/ptree.hpp>

using namespace std;
using namespace boost::property_tree;
using namespace dds;

CTask::CTask()
    : CTopoElement()
    , m_exe()
    , m_env()
    , m_exeReachable(true)
    , m_envReachable(true)
    , m_properties()
{
    setType(ETopoType::TASK);
}

CTask::~CTask()
{
}

void CTask::setExe(const string& _exe)
{
    m_exe = _exe;
}

void CTask::setEnv(const string& _env)
{
    m_env = _env;
}

void CTask::setExeReachable(bool _exeReachable)
{
    m_exeReachable = _exeReachable;
}

void CTask::setEnvReachable(bool _envReachable)
{
    m_envReachable = _envReachable;
}

void CTask::setProperties(const TopoPropertyPtrVector_t& _properties)
{
    m_properties = _properties;
}

void CTask::addProperty(TopoPropertyPtr_t _property)
{
    m_properties.push_back(_property);
}

void CTask::setRestriction(RestrictionPtr_t _restriction)
{
    m_restriction = _restriction;
}

size_t CTask::getNofTasks() const
{
    return 1;
}

size_t CTask::getTotalNofTasks() const
{
    return 1;
}

const string& CTask::getExe() const
{
    return m_exe;
}

const string& CTask::getEnv() const
{
    return m_env;
}

bool CTask::isExeReachable() const
{
    return m_exeReachable;
}

bool CTask::isEnvReachable() const
{
    return m_envReachable;
}

size_t CTask::getNofProperties() const
{
    return m_properties.size();
}

size_t CTask::getTotalCounter() const
{
    return getTotalCounterDefault();
}

TopoPropertyPtr_t CTask::getProperty(size_t _i) const
{
    if (_i >= getNofProperties())
        throw out_of_range("Out of range exception");
    return m_properties[_i];
}

const TopoPropertyPtrVector_t& CTask::getProperties() const
{
    return m_properties;
}

RestrictionPtr_t CTask::getRestriction() const
{
    return m_restriction;
}

void CTask::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& taskPT = CTopoElement::findElement(ETopoType::TASK, _name, _pt.get_child("topology"));

        setId(taskPT.get<string>("<xmlattr>.id"));
        setExe(taskPT.get<string>("exe"));
        setEnv(taskPT.get<string>("env", ""));
        setExeReachable(taskPT.get<bool>("exe.<xmlattr>.reachable", true));
        setEnvReachable(taskPT.get<bool>("env.<xmlattr>.reachable", true));

        string restrictionId = taskPT.get<string>("restriction", "");
        if (!restrictionId.empty())
        {
            RestrictionPtr_t newRestriction = make_shared<CRestriction>();
            newRestriction->setParent(this);
            newRestriction->initFromPropertyTree(restrictionId, _pt);
            setRestriction(newRestriction);
        }

        boost::optional<const ptree&> propertiesPT = taskPT.get_child_optional("properties");
        if (propertiesPT)
        {
            for (const auto& property : propertiesPT.get())
            {
                TopoPropertyPtr_t newProperty = make_shared<CTopoProperty>();
                newProperty->setParent(this);
                newProperty->initFromPropertyTree(property.second.data(), _pt);
                addProperty(newProperty);
            }
        }
    }
    catch (exception& error) // ptree_error, logic_error
    {
        throw logic_error("Unable to initialize task " + _name + " error: " + error.what());
    }
}

string CTask::toString() const
{
    stringstream ss;
    ss << "Task: m_id=" << getId() << " m_exe=" << m_exe << " m_env=" << m_env << " m_properties:\n";
    for (const auto& property : m_properties)
    {
        ss << " - " << property->toString() << endl;
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTask& _task)
{
    _strm << _task.toString();
    return _strm;
}
