#ifndef _KUNLABORO_DEFINES_HPP
#define _KUNLABORO_DEFINES_HPP

#include <boost/any.hpp>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace Kunlaboro
{
    class Component;
    struct Message;

    typedef unsigned int GUID;

    typedef GUID ComponentId;
    typedef GUID EntityId;
    typedef GUID RequestId;

    typedef boost::any Payload;

    typedef std::function<void(const Message&)> MessageFunction;
    typedef std::function<Component*()> 	     FactoryFunction;

    typedef std::unordered_map<std::string, std::vector<Component*> > ComponentMap;
    
    typedef std::unordered_map<std::string, GUID> NameToIdMap;
    typedef std::unordered_map<GUID, std::string> IdToNameMap;

    enum MessageType
    {
        Type_Create,
        Type_Destroy,
        Type_Message
    };

    enum MessageReason
    {
        Reason_Message,
        Reason_Component,
        Reason_AllComponents
    };

    struct ComponentRequested
    {
        MessageReason reason;
        std::string name;
    };

    struct ComponentRegistered
    {
        Component* component;
        MessageFunction callback;
        bool required;
    };

    struct Message {
        MessageType type;
        Component* sender;
        Payload payload;
        Message(MessageType t) : type(t) {};
        Message(MessageType t, Component *c) : type(t), sender(c) {};
        Message(MessageType t, Component *c, Payload p) : type(t), sender(c), payload(p) {};
    };
}

#endif