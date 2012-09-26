#include "Component.hpp"
#include "EntitySystem.hpp"
#include <sstream>

using namespace Kunlaboro;

Component::Component(const std::string& name) : mOwner(0), mEntitySystem(0), mId(0), mName(name), mDestroyed(false)
{
}

Component::~Component()
{
}

void Component::addedToEntity()
{

}

void Component::addLocalComponent(Component* comp)
{
    mEntitySystem->addComponent(mOwner, comp);
}

void Component::requestMessage(const std::string& name, MessageFunction callback, int priority) const
{
    ComponentRequested req;
    req.name = name;
    req.reason = Reason_Message;

    ComponentRegistered reg;
    reg.callback = callback;
    reg.component = const_cast<Component*>(this);
    reg.required = false;
    reg.priority = priority;

    mEntitySystem->registerGlobalRequest(req, reg);
}

void Component::unrequestMessage(const std::string& name, MessageFunction callback, int priority) const
{
    ComponentRequested req;
    req.name = name;
    req.reason = Reason_Message;

    ComponentRegistered reg;
    reg.callback = callback;
    reg.component = const_cast<Component*>(this);
    reg.required = false;
    reg.priority = priority;

    mEntitySystem->removeGlobalRequest(req, reg);
}

void Component::requestComponent(const std::string& name, MessageFunction callback, bool local) const
{
    ComponentRequested req;
    req.name = name;
    req.reason = Reason_Component;

    ComponentRegistered reg;
    reg.callback = callback;
    reg.component = const_cast<Component*>(this);
    reg.required = false;
    reg.priority = 0;

    if (local)
        mEntitySystem->registerLocalRequest(req, reg);
    else
        mEntitySystem->registerGlobalRequest(req, reg);
}

void Component::requireComponent(const std::string& name, MessageFunction callback, bool local) const
{
    ComponentRequested req;
    req.name = name;
    req.reason = Reason_Component;

    ComponentRegistered reg;
    reg.callback = callback;
    reg.component = const_cast<Component*>(this);
    reg.required = true;
    reg.priority = 0;

    if (local)
        mEntitySystem->registerLocalRequest(req, reg);
    else
        mEntitySystem->registerGlobalRequest(req, reg);
}

void Component::changeRequestPriority(RequestId rid, int priority) const
{
    mEntitySystem->reprioritizeRequest(const_cast<Component*>(this), rid, priority);
}

RequestId Component::getMessageRequestId(const std::string& name) const
{
    return mEntitySystem->getMessageRequestId(Reason_Message, name);
}

void Component::sendMessage(RequestId id) const
{
    Message msg(Type_Message, const_cast<Component*>(this));
    mEntitySystem->sendLocalMessage(mOwner, id, msg);
}

void Component::sendMessage(RequestId id, const Payload& p) const
{
    Message msg(Type_Message, const_cast<Component*>(this), p);
    mEntitySystem->sendLocalMessage(mOwner, id, msg);
}

void Component::sendMessage(RequestId id, const Message& m) const
{
    Message msg = m;
    mEntitySystem->sendLocalMessage(mOwner, id, msg);
}

Message Component::sendQuestion(RequestId id, const Message& m) const
{
    Message msg = m;
    mEntitySystem->sendLocalMessage(mOwner, id, msg);

    return msg;
}

void Component::sendGlobalMessage(RequestId id) const
{
    Message msg(Type_Message, const_cast<Component*>(this));
    mEntitySystem->sendGlobalMessage(id, msg);
}

void Component::sendGlobalMessage(RequestId id, const Payload& p) const
{
    Message msg(Type_Message, const_cast<Component*>(this), p);
    mEntitySystem->sendGlobalMessage(id, msg);
}

void Component::sendGlobalMessage(RequestId id, const Message& m) const
{
    Message msg = m;
    mEntitySystem->sendGlobalMessage(id, msg);
}

Message Component::sendGlobalQuestion(RequestId id, const Message& m) const
{
    Message msg = m;
    mEntitySystem->sendGlobalMessage(id, msg);

    return msg;
}

void Component::sendMessageToEntity(EntityId eid, RequestId rid) const
{
    Message msg(Type_Message, const_cast<Component*>(this));
    mEntitySystem->sendLocalMessage(eid, rid, msg);
}

void Component::sendMessageToEntity(EntityId eid, RequestId rid, const Payload& p) const
{
    Message msg(Type_Message, const_cast<Component*>(this), p);
    mEntitySystem->sendLocalMessage(eid, rid, msg);
}

void Component::sendMessageToEntity(EntityId eid, RequestId rid, const Message& m) const
{
    Message msg = m;
    mEntitySystem->sendLocalMessage(eid, rid, msg);
}

Message Component::sendQuestionToEntity(EntityId eid, RequestId rid, const Message& m) const
{
    Message msg = m;
    mEntitySystem->sendLocalMessage(eid, rid, msg);

    return msg;
}

void Component::sendGlobalMessage(const std::string& id, const Payload& p) const { sendGlobalMessage(mEntitySystem->getMessageRequestId(Reason_Message, id), p); }
Message Component::sendGlobalQuestion(const std::string& id, const Payload& p) const { Message m(Type_Message, const_cast<Component*>(this), p); mEntitySystem->sendGlobalMessage(mEntitySystem->getMessageRequestId(Reason_Message, id), m); return m; }

ComponentId Component::getId() const
{
    return mId;
}

EntityId Component::getOwnerId() const
{
    return mOwner;
}

void Component::destroy()
{
    mEntitySystem->destroyComponent(this);
}

bool Component::isDestroyed() const
{
    return mDestroyed;
}

bool Component::isValid() const
{
    return mOwner != 0 && !mName.empty() && !mDestroyed;
}

const std::string& Component::getName() const
{
    return mName;
}

std::string Component::toString() const
{
    std::stringstream ss;
    ss << "Component #" << getId() << " \"" << getName() << "\" owned by entity #" << getOwnerId();

    return ss.str();
}

void Component::setDestroyed()
{
    mDestroyed = true;
}

void Component::setOwner(EntityId id)
{
    mOwner = id;
}

std::ostream& operator<<(std::ostream& os, const Component& c)
{
    return os << c.toString();
}
