#include "EntitySystem.hpp"
#include "Component.hpp"
#include <stdexcept>

using namespace Kunlaboro;

EntitySystem::EntitySystem() :
    mComponentCounter(1), mRequestCounter(1), mEntityCounter(1)
{
}

EntitySystem::~EntitySystem()
{
    for (unsigned int i = 0; i < mEntities.size(); i++)
    {
        if (mEntities[i] == 0)
            continue;

        Entity* ent = mEntities[i];

        for (ComponentMap::iterator it = ent->components.begin(); it != ent->components.end(); ++it)
        {
            for (std::vector<Component*>::iterator cit = it->second.begin(); cit != it->second.end(); ++cit)
            {
                destroyComponent(*cit);

                if (it->second.empty())
                    break;
            }
        }

        delete ent;
        mEntities[i] = 0;
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
        throw new std::runtime_error("Can't destroy a non-existant entity");

    Entity* ent = mEntities[entity-1];

    if (ent == NULL)
        throw new std::runtime_error("Can't destroy a non-existant entity");

    for (ComponentMap::iterator it = ent->components.begin(); it != ent->components.end(); ++it)
    {
        for (std::vector<Component*>::iterator cit = it->second.begin(); cit != it->second.end(); ++it)
            destroyComponent(*cit);
    }

    delete ent;
    mEntities[entity-1] = NULL;
}

void EntitySystem::finalizeEntity(EntityId entity)
{
    if (entity == 0 || entity > mEntities.size())
        throw new std::runtime_error("Can't finalize a non-existant entity");

    Entity* ent = mEntities[entity-1];

    if (ent == NULL)
        throw new std::runtime_error("Can't finalize a non-existant entity");

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
        throw new std::runtime_error("Registered components must have unique names");

    mRegisteredComponents[name] = func;
}

Component* EntitySystem::createComponent(const std::string& name)
{
    if (mRegisteredComponents.count(name) == 0)
        throw new std::runtime_error("Can't create a non-existant component");

    Component* comp = mRegisteredComponents[name]();
    comp->mEntitySystem = this;
    comp->mId = mComponentCounter++;
    return comp;
}

void EntitySystem::destroyComponent(Component* component)
{
    if (!component->isValid())
        throw new std::runtime_error("Can't destroy an invalid component");

    std::vector<ComponentRequested>& reqs = mRequestsByComponent[component->getId()];
    for (auto it = reqs.begin(); it != reqs.end(); ++it)
    {
        RequestId reqid = getExistingRequestId(it->reason, it->name);

        if (reqid == 0)
            return;

        std::vector<ComponentRegistered>& reqs = mGlobalRequests[reqid];
        for(auto reg = reqs.begin(); reg != reqs.end();)
        {
            if (reg->component->getId() == component->getId())
                reg = reqs.erase(reg);
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
        std::vector<ComponentRegistered>& regs = ent->localRequests[i];
        if (regs.empty())
            continue;

        ComponentRegistered* regsP = &regs.front();
        for (unsigned int j = 0; j < regs.size(); j++)
        {
            if (regsP[j].component->getId() == component->getId())
            {
                for (unsigned k = j+1; k < regs.size(); ++k) {
                    regsP[k-1] = regsP[k];
                }

                regs.pop_back();
                break;
            }
        }
    }

    Message msg(Type_Destroy, component);

    RequestId reqid = getExistingRequestId(Reason_Component, component->getName());

    if (reqid != 0)
    {
        for (auto it = mGlobalRequests[reqid].begin(); it != mGlobalRequests[reqid].end(); ++it)
            it->callback(msg);

        std::vector<ComponentRegistered>& regs = ent->localRequests[reqid];
        for (unsigned int i = 0; i < regs.size(); i++)
        {
            regs[i].callback(msg);
        }
    }

    delete component;
}

void EntitySystem::addComponent(EntityId entity, Component* component)
{
    if (component->getOwnerId() != 0)
        throw new std::runtime_error("Can't add a component to several entities");

    if (entity == 0 || entity > mEntities.size())
        throw new std::runtime_error("Can't add a component to a non-existant entity");

    Entity* ent = mEntities[entity-1];
    
    if (ent == NULL)
        throw new std::runtime_error("Can't add a component to a non-existant entity");

    component->setOwner(entity);
    ent->components[component->getName()].push_back(component);

    component->addedToEntity();

    RequestId reqid = getExistingRequestId(Reason_Component, component->getName());

    if (reqid == 0)
        return;

    Message msg(Type_Create, component);

    for (std::vector<ComponentRegistered>::iterator it = mGlobalRequests[entity].begin(); it != mGlobalRequests[entity].end(); ++it)
    {
        if (it->component->getId() != component->getId())
            it->callback(msg);
    }

    std::vector<ComponentRegistered>& regs = ent->localRequests[reqid];
    for (unsigned int i = 0; i < regs.size(); i++)
    {
        regs[i].callback(msg);
    }
}

void EntitySystem::removeComponent(EntityId entity, Component* component)
{
    if (entity == 0 || entity > mEntities.size())
        throw new std::runtime_error("Can't remove a component from a non-existant entity");

    Entity* ent = mEntities[entity-1];
    
    if (ent == NULL)
        throw new std::runtime_error("Can't remove a component from a non-existant entity");

    std::vector<Component*>::iterator found;
    std::vector<Component*>& comps = ent->components[component->getName()];
    found = std::find(comps.begin(), comps.end(), component);

    if (found == comps.end())
        throw new std::runtime_error("Can't remove a component from an entity that doesn't contain it");

    component->setOwner(0);
    ent->components[component->getName()].erase(found);

    RequestId reqid = getExistingRequestId(Reason_Component, component->getName());

    if (reqid == 0)
        return;

    Message msg(Type_Destroy, component);

    for (std::vector<ComponentRegistered>::iterator it = mGlobalRequests[entity].begin(); it != mGlobalRequests[entity].end(); ++it)
    {
        if (it->component->getId() != component->getId())
            it->callback(msg);
    }

    std::vector<ComponentRegistered>& regs = ent->localRequests[reqid];
    for (unsigned int i = 0; i < regs.size(); i++)
    {
        regs[i].callback(msg);
    }
}

RequestId EntitySystem::getMessageRequestId(MessageReason reason, const std::string& name)
{
    if (reason == Reason_AllComponents)
        reason = Reason_Component;

    if (mNameMap[reason].find(name) == mNameMap[reason].end())
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
        throw new std::runtime_error("Can't register a request from an invalid component");

    RequestId reqid = getMessageRequestId(req.reason, req.name);

    if (req.reason != Reason_AllComponents)
    {
        mGlobalRequests[reqid].push_back(reg);

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
        if (mEntities[i] = 0)
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
    RequestId reqid = getMessageRequestId(req.reason, req.name);

    Entity* ent = mEntities[reg.component->getOwnerId()-1];
    ent->localRequests[reqid].push_back(reg);

    if (reg.required && !ent->finalised)
        mRequiredComponents[ent->id].push_back(req.name);

    if (req.reason != Reason_Component)
        return;

    Message msg(Type_Create);

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

void EntitySystem::sendGlobalMessage(RequestId reqid, const Message& msg)
{
    if (msg.sender != 0 && !msg.sender->isValid())
        throw new std::runtime_error("Invalid sender for global message");

    for (auto it = mGlobalRequests[reqid].begin(); it != mGlobalRequests[reqid].end(); ++it)
    {
        it->callback(msg);
    }
}

void EntitySystem::sendLocalMessage(EntityId entity, RequestId reqid, const Message& msg)
{
    if (entity == 0 || entity > mEntities.size())
        throw new std::runtime_error("Can't send a message to a non-existant entity");

    Entity* ent = mEntities[entity-1];
    
    if (ent == NULL)
        throw new std::runtime_error("Can't send a message to a non-existant entity");

    if (ent->localRequests.count(reqid) == 0)
        return;

    std::vector<ComponentRegistered>& regs = ent->localRequests[reqid];
    for (unsigned int i = 0; i < regs.size(); i++)
    {
        regs[i].callback(msg);
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