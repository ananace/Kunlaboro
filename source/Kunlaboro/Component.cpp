#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>
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

void Component::requestMessage(const std::string& name, MessageFunction callback, bool local) const
{
    ComponentRequested req;
    req.name = name;
    req.hash = hash::hashString(name);
    req.reason = Reason_Message;

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

void Component::unrequestMessage(const std::string& name, bool local) const
{
    ComponentRequested req;
    req.name = name;
    req.hash = hash::hashString(name);
    req.reason = Reason_Message;

    ComponentRegistered reg;
    reg.component = const_cast<Component*>(this);
    reg.required = false;
    reg.priority = 0;

    if (local)
        mEntitySystem->removeLocalRequest(req, reg);
    else
        mEntitySystem->removeGlobalRequest(req, reg);
}

void Component::requestComponent(const std::string& name, MessageFunction callback, bool local) const
{
    ComponentRequested req;
    req.name = name;
    req.hash = hash::hashString(name);
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
    req.hash = hash::hashString(name);
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

void Component::sendGlobalMessage(RequestId id, const Message& m) const
{
    Message msg = m;
    mEntitySystem->sendSafeGlobalMessage(id, msg);
}

Message Component::sendGlobalQuestion(RequestId id, const Message& m) const
{
    Message msg = m;
    mEntitySystem->sendGlobalMessage(id, msg);

    return msg;
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

void Component::destroy()
{
    mDestroyed = true;
    mEntitySystem->destroyComponent(this);
}

std::string Component::toString() const
{
    std::stringstream ss;
    ss << "Component #" << getId() << " \"" << getName() << "\" owned by entity #" << getOwnerId();

    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Component& c)
{
    return os << c.toString();
}
