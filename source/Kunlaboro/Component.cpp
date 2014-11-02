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

void Component::unrequestMessage(RequestId rid, bool local)
{
    ComponentRequested req;
    req.hash = rid;
    req.reason = Reason_Message;

    ComponentRegistered reg;
    reg.component = this;
    reg.required = false;
    reg.priority = 0;

    if (local)
        mEntitySystem->removeLocalRequest(req, reg);
    else
        mEntitySystem->removeGlobalRequest(req, reg);
}

void Component::requestComponent(const std::string& name, const ComponentCallback& func, bool local)
{
    ComponentRequested req;
    req.name = name;
    req.hash = Kunlaboro::hash::hashString(name);
    req.reason = Reason_Component;

    ComponentRegistered reg;
    reg.component = this;
    reg.functional = *reinterpret_cast<std::function<void()>*>(const_cast<ComponentCallback*>(&func));
    reg.type = &typeid(func);
    reg.required = false;
    reg.priority = 0;

    if (local)
        mEntitySystem->registerLocalRequest(req, reg);
    else
        mEntitySystem->registerGlobalRequest(req, reg);
}
void Component::requireComponent(const std::string& name, const ComponentCallback& func, bool local)
{
    ComponentRequested req;
    req.name = name;
    req.hash = Kunlaboro::hash::hashString(name);
    req.reason = Reason_Component;

    ComponentRegistered reg;
    reg.component = this;
    reg.functional = *reinterpret_cast<std::function<void()>*>(const_cast<ComponentCallback*>(&func));
    reg.type = &typeid(func);
    reg.required = false;
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
