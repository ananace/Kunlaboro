#include "EntitySystem.hpp"
#include "Component.hpp"
#include <stdexcept>
#include <algorithm>

using namespace Kunlaboro;

struct RequestSort
{
    bool operator()(ComponentRegistered a, ComponentRegistered b)
    {
        return a.priority < b.priority;
    }
};

EntitySystem::EntitySystem() :
    mComponentCounter(1), mRequestCounter(1), mEntityCounter(1), mFrozen(0)
{
}

EntitySystem::~EntitySystem()
{
    while(!mEntities.empty())
    {
        destroyEntity(mEntities.back()->id);
    }
}

EntityId EntitySystem::createEntity()
{
    Entity* ent = new Entity();
    ent->id = mEntityCounter++;
    ent->finalised = false;

    mEntities.push_back(ent);

    return mEntityCounter-1;
}

void EntitySystem::destroyEntity(EntityId entity)
{
    if (entity == 0 || entity > mEntities.size())
        throw std::runtime_error("Can't destroy a non-existant entity");

    Entity* ent = mEntities[entity-1];

    if (ent == NULL)
        throw std::runtime_error("Can't destroy a non-existant entity");

    if (isFrozen())
    {
        mFrozenData.frozenEntityDestructions.push_back(entity);
        mFrozenData.needsProcessing = true;
        return;
    }

    for (ComponentMap::iterator it = ent->components.begin(); it != ent->components.end(); ++it)
    {
        std::vector<Component*>& comps = it->second;
        for (auto cit = comps.begin(); cit != comps.end(); ++cit)
        {
            destroyComponent(*cit);

            if (comps.empty())
                break;
        }
    }

    delete ent;
    Entity** entP = &mEntities.front();

    for (unsigned k = entity; k < mEntities.size(); ++k) {
        entP[k-1] = entP[k];
    }

    mEntities.pop_back();
}

void EntitySystem::finalizeEntity(EntityId entity)
{
    if (entity == 0 || entity > mEntities.size())
        throw std::runtime_error("Can't finalize a non-existant entity");

    Entity* ent = mEntities[entity-1];

    if (ent == NULL)
        throw std::runtime_error("Can't finalize a non-existant entity");

    ent->finalised = true;

    if (mRequiredComponents.count(entity) == 0)
        return;

    std::vector<std::string>& reqs = mRequiredComponents[entity];
    bool destroy = false;

    for (std::vector<std::string>::iterator it = reqs.begin(); it != reqs.end(); ++it)
    {
        if (ent->components.count(*it) == 0)
            destroy = true;
    }

    if (destroy)
        destroyEntity(entity);
}

void EntitySystem::registerComponent(const std::string& name, FactoryFunction func)
{
    if (mRegisteredComponents.count(name) > 0)
        throw std::runtime_error("Registered components must have unique names");

    mRegisteredComponents[name] = func;
}

Component* EntitySystem::createComponent(const std::string& name)
{
    if (mRegisteredComponents.count(name) == 0)
        throw std::runtime_error("Can't create a non-existant component");

    Component* comp = mRegisteredComponents[name]();

    if (comp->getName() != name)
        throw std::runtime_error("The factory function for " + name + " creates components of type " + comp->getName());

    comp->mEntitySystem = this;
    comp->mId = mComponentCounter++;
    return comp;
}

void EntitySystem::destroyComponent(Component* component)
{
    if (!component->isValid())
        throw std::runtime_error("Can't destroy an invalid component");

    if (isFrozen())
    {
        mFrozenData.frozenComponentDestructions.push_back(component);
        mFrozenData.needsProcessing = true;
        return;
    }

    std::vector<ComponentRequested>& reqs = mRequestsByComponent[component->getId()];
    for (auto it = reqs.begin(); it != reqs.end(); ++it)
    {
        RequestId reqid = getExistingRequestId(it->reason, it->name);

        if (reqid == 0)
            return;

        for(auto reg = mGlobalRequests[reqid].begin(); reg != mGlobalRequests[reqid].end();)
        {
            if (reg->component->getId() == component->getId())
                reg = mGlobalRequests[reqid].erase(reg);
            else
                ++reg;
        }
    }

    Entity* ent = mEntities[component->getOwnerId()-1];

    std::vector<Component*>& comps = ent->components[component->getName()];
    for (auto it = comps.begin(); it != comps.end(); ++it)
        if ((*it)->getId() == component->getId())
        {
            comps.erase(it);
            break;
        }

    for (unsigned int i = 0; i < ent->localRequests.size(); i++)
    {
        std::deque<ComponentRegistered>& regs = ent->localRequests[i];
        if (regs.empty())
            continue;

        for (auto it = regs.begin(); it != regs.end(); ++it)
        {
            if (it->component->getId() == component->getId())
            {
                regs.erase(it);
                break;
            }
        }
    }

    Message msg(Type_Destroy, component);

    RequestId reqid = getExistingRequestId(Reason_Component, component->getName());

    if (reqid != 0)
    {
        freeze();

        for (auto it = mGlobalRequests[reqid].begin(); it != mGlobalRequests[reqid].end(); ++it)
            it->callback(msg);

        std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
        for (unsigned int i = 0; i < regs.size(); i++)
        {
            regs[i].callback(msg);
        }

        unfreeze();
    }

    delete component;
}

void EntitySystem::addComponent(EntityId entity, Component* component)
{
    if (component->getOwnerId() != 0)
        throw std::runtime_error("Can't add a component to several entities");

    if (entity == 0 || entity > mEntities.size())
        throw std::runtime_error("Can't add a component to a non-existant entity");

    Entity* ent = mEntities[entity-1];
    
    if (ent == NULL)
        throw std::runtime_error("Can't add a component to a non-existant entity");

    component->setOwner(entity);
    ent->components[component->getName()].push_back(component);

    component->addedToEntity();

    RequestId reqid = getExistingRequestId(Reason_Component, component->getName());

    if (reqid == 0)
        return;

    Message msg(Type_Create, component);

    freeze();

    if (mGlobalRequests.count(reqid) > 0)
        for (std::deque<ComponentRegistered>::iterator it = mGlobalRequests[reqid].begin(); it != mGlobalRequests[reqid].end(); ++it)
        {
            if (it->component->getId() != component->getId())
                it->callback(msg);
        }

    if (ent->localRequests.count(reqid) > 0)
    {
        std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
        for (unsigned int i = 0; i < regs.size(); i++)
        {
            regs[i].callback(msg);
        }
    }

    unfreeze();
}

void EntitySystem::removeComponent(EntityId entity, Component* component)
{
    if (entity == 0 || entity > mEntities.size())
        throw std::runtime_error("Can't remove a component from a non-existant entity");

    Entity* ent = mEntities[entity-1];
    
    if (ent == NULL)
        throw std::runtime_error("Can't remove a component from a non-existant entity");

    std::vector<Component*>::iterator found;
    std::vector<Component*>& comps = ent->components[component->getName()];
    found = std::find(comps.begin(), comps.end(), component);

    if (found == comps.end())
        throw std::runtime_error("Can't remove a component from an entity that doesn't contain it");

    component->setOwner(0);
    ent->components[component->getName()].erase(found);

    RequestId reqid = getExistingRequestId(Reason_Component, component->getName());

    if (reqid == 0)
        return;

    Message msg(Type_Destroy, component);

    freeze();

    for (std::deque<ComponentRegistered>::iterator it = mGlobalRequests[entity].begin(); it != mGlobalRequests[entity].end(); ++it)
    {
        if (it->component->getId() != component->getId())
            it->callback(msg);
    }

    std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
    for (unsigned int i = 0; i < regs.size(); i++)
    {
        regs[i].callback(msg);
    }

    unfreeze();
}

std::vector<Component*> EntitySystem::getAllComponentsOnEntity(EntityId entity, const std::string& name)
{
    if (entity == 0 || entity > mEntities.size())
        throw std::runtime_error("Can't check for components on a non-existant entity");

    Entity* ent = mEntities[entity-1];
    
    if (ent == NULL)
        throw std::runtime_error("Can't check for components on a non-existant entity");

    if (name == "")
    {
        std::vector<Component*> components;

        for (auto it = ent->components.begin(); it != ent->components.end(); ++it)
        {
            for (auto cit = it->second.begin(); cit != it->second.end(); ++cit)
                components.push_back(*cit);
        }

        return components;
    }
    else
    {
        if (ent->components.count(name) > 0)
            return ent->components[name];
        else
            return std::vector<Component*>();
    }
}

RequestId EntitySystem::getMessageRequestId(MessageReason reason, const std::string& name)
{
    if (reason == Reason_AllComponents)
        reason = Reason_Component;

    if (mNameMap[reason].count(name) == 0)
    {
        mNameMap[reason][name] = mRequestCounter;
        mIdMap[reason][mRequestCounter] = name;

        return mRequestCounter++;
    }

    return mNameMap[reason][name];
}

void EntitySystem::registerGlobalRequest(const ComponentRequested& req, const ComponentRegistered& reg)
{
    if (!reg.component->isValid())
        throw std::runtime_error("Can't register a request from an invalid component");

    if (isFrozen())
    {
        mFrozenData.frozenGlobalRequests.push_back(std::pair<ComponentRequested, ComponentRegistered>(req, reg));
        mFrozenData.needsProcessing = true;
        return;
    }

    RequestId reqid = getMessageRequestId(req.reason, req.name);

    if (req.reason != Reason_AllComponents)
    {
        mGlobalRequests[reqid].push_back(reg);
        std::sort(mGlobalRequests[reqid].begin(), mGlobalRequests[reqid].end(), RequestSort());

        if (req.reason == Reason_Message)
        {
            mEntities[reg.component->getOwnerId()-1]->localRequests[reqid].push_back(reg);
        }

        if (reg.required && !mEntities[reg.component->getOwnerId()-1]->finalised)
            mRequiredComponents[reg.component->getOwnerId()].push_back(req.name);

        mRequestsByComponent[reg.component->getId()].push_back(req);
    }

    if (req.reason == Reason_Message)
        return;

    Message msg(Type_Create);
    for (unsigned int i = 0; i < mEntities.size(); i++)
    {
        if (mEntities[i] == 0)
            continue;

        if (mEntities[i]->components.count(req.name) == 0)
            continue;

        std::vector<Component*>& comps = mEntities[i]->components[req.name];
        for (auto it = comps.begin(); it != comps.end(); ++it)
        {
            if ((*it)->isValid() && reg.component->getId() != (*it)->getId())
            {
                msg.sender = (*it);
                
                reg.callback(msg);
            }
        }
    }
}

void EntitySystem::registerLocalRequest(const ComponentRequested& req, const ComponentRegistered& reg)
{
    if (isFrozen())
    {
        mFrozenData.frozenLocalRequests.push_back(std::pair<ComponentRequested, ComponentRegistered>(req, reg));
        mFrozenData.needsProcessing = true;
        return;
    }

    RequestId reqid = getMessageRequestId(req.reason, req.name);

    Entity* ent = mEntities[reg.component->getOwnerId()-1];

    std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
    regs.push_back(reg);
    std::sort(regs.begin(), regs.end(), RequestSort());

    if (reg.required && !ent->finalised)
        mRequiredComponents[ent->id].push_back(req.name);

    if (req.reason != Reason_Component)
        return;

    Message msg(Type_Create);

    if (ent->components.count(req.name) == 0)
        return;

    std::vector<Component*>& comps = ent->components[req.name];
    for (auto it = comps.begin(); it != comps.end(); ++it)
    {
        if ((*it)->isValid() && reg.component->getId() == (*it)->getId())
        {
            msg.sender = *it;

            reg.callback(msg);
        }
    }
}

void EntitySystem::reprioritizeRequest(Component* comp, RequestId reqid, int priority)
{
    Entity* ent = mEntities[comp->getOwnerId()-1];
    if (ent->localRequests.count(reqid) > 0)
    {
        std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
        for (auto it = regs.begin(); it != regs.end(); ++it)
            if (it->component == comp)
            {
                it->priority = priority;
                break;
            }

        std::sort(regs.begin(), regs.end(), RequestSort());
    }

    if (mGlobalRequests.count(reqid) > 0)
    {
        std::deque<ComponentRegistered>& regs = mGlobalRequests[reqid];
        for (auto it = regs.begin(); it != regs.end(); ++it)
            if (it->component == comp)
            {
                it->priority = priority;
                break;
            }

        std::sort(regs.begin(), regs.end(), RequestSort());
    }
}

void EntitySystem::sendGlobalMessage(RequestId reqid, const Message& msg)
{
    if (msg.sender != 0 && !msg.sender->isValid())
        throw std::runtime_error("Invalid sender for global message");

    freeze();

    for (auto it = mGlobalRequests[reqid].begin(); it != mGlobalRequests[reqid].end(); ++it)
    {
        it->callback(msg);
    }

    unfreeze();
}

void EntitySystem::sendLocalMessage(EntityId entity, RequestId reqid, const Message& msg)
{
    if (entity == 0 || entity > mEntities.size())
        throw std::runtime_error("Can't send a message to a non-existant entity");

    Entity* ent = mEntities[entity-1];
    
    if (ent == NULL)
        throw std::runtime_error("Can't send a message to a non-existant entity");

    if (ent->localRequests.count(reqid) == 0)
        return;

    freeze();

    std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
    for (unsigned int i = 0; i < regs.size(); i++)
    {
        regs[i].callback(msg);
    }

    unfreeze();
}

void EntitySystem::freeze()
{
    mFrozen++;
}

void EntitySystem::unfreeze()
{
    if (--mFrozen == 0 && mFrozenData.needsProcessing)
    {
        //Process frozen queues

        for (auto it = mFrozenData.frozenGlobalRequests.begin(); it != mFrozenData.frozenGlobalRequests.end(); ++it)
            registerGlobalRequest(it->first, it->second);
        for (auto it = mFrozenData.frozenLocalRequests.begin(); it != mFrozenData.frozenLocalRequests.end(); ++it)
            registerLocalRequest(it->first, it->second);

        for (auto it = mFrozenData.frozenComponentDestructions.begin(); it != mFrozenData.frozenComponentDestructions.end(); ++it)
            destroyComponent(*it);

        for (auto it = mFrozenData.frozenEntityDestructions.begin(); it != mFrozenData.frozenEntityDestructions.end(); ++it)
            destroyEntity(*it);

        mFrozenData.frozenComponentDestructions.clear();
        mFrozenData.frozenEntityDestructions.clear();
        mFrozenData.frozenGlobalRequests.clear();
        mFrozenData.frozenLocalRequests.clear();

        mFrozenData.needsProcessing = false;
    }
}

RequestId EntitySystem::getExistingRequestId(MessageReason reason, const std::string& name)
{
    if (mNameMap[reason].count(name) == 0)
        return 0;

    RequestId reqid = mNameMap[reason][name];
    //if (mGlobalRequests.count(reqid) == 0)
    //    return 0;

    return reqid;
}
