#include "EntitySystem.hpp"
#include "Component.hpp"
#include "Template.hpp"
#include <stdexcept>
#include <algorithm>

using namespace Kunlaboro;

struct RequestSort
{
    inline bool operator()(const ComponentRegistered& a, const ComponentRegistered& b) const
    {
        return a.priority < b.priority;
    }
};

struct RequestFind
{
    const ComponentRegistered* reg;
    const ComponentRequested* req;

    RequestFind(const ComponentRegistered& a): reg(&a), req(NULL) {}
    RequestFind(const ComponentRequested& a): reg(NULL), req(&a) {}

    inline bool operator()(const ComponentRegistered& b) const
    {
        return reg->component == b.component && reg->priority == b.priority && reg->required == b.required;
    }

    inline bool operator()(const ComponentRequested& b) const
    {
        return req->name == b.name && req->reason == b.reason;
    }
};

template<typename T, typename Y>
inline void insertedPush(std::deque<T>& deque, const T& value, const Y& comp)
{
    deque.insert(std::lower_bound(deque.begin(), deque.end(), value, comp), value);
}

EntitySystem::FrozenData::RequestLock::RequestLock(const RequestLock& b)
{
    locked = b.locked;
    mutex = b.mutex;
    repriorities = b.repriorities;
    localRequests = b.localRequests;
    localRequestRemoves = b.localRequestRemoves;
    globalRequests = b.globalRequests;
    globalRequestRemoves = b.globalRequestRemoves;
}
EntitySystem::FrozenData::RequestLock& EntitySystem::FrozenData::RequestLock::operator=(const EntitySystem::FrozenData::RequestLock& b)
{
    if (this == &b)
        return *this;

    locked = b.locked;
    mutex = b.mutex;
    repriorities = b.repriorities;
    localRequests = b.localRequests;
    localRequestRemoves = b.localRequestRemoves;
    globalRequests = b.globalRequests;
    globalRequestRemoves = b.globalRequestRemoves;
    
    return *this;
}

EntitySystem::EntitySystem(bool thread) :
    mComponentCounter(1), mRequestCounter(1), mEntityCounter(1), mThreaded(thread), mFrozen(0), mEntityC(0), mComponentC(0)
{
    mFrozenData.needsProcessing = false;

#ifndef Kunlaboro_BOOST
    if (mThreaded)
        throw std::runtime_error("A threaded EntitySystem requires Kunlaboro to be compiled with Boost.");
#endif
}

EntitySystem::~EntitySystem()
{
    for (auto it = mEntities.begin(); it != mEntities.end(); ++it)
    {
        if (it->second == NULL)
            continue;

        destroyEntity(it->second->id);
    }
}

EntityId EntitySystem::createEntity()
{
    Entity* ent = new Entity();
    ent->id = mEntityCounter++;
    ent->finalised = false;

    mEntities[ent->id] = ent;
    mEntityC++;

    return mEntityCounter-1;
}

EntityId EntitySystem::createEntity(const std::string& templateName)
{
    if (mRegisteredTemplates.count(templateName) == 0)
        throw std::runtime_error("Can't instantiate a template that doesn't exist");

    EntityId ent = createEntity();
    Template* temp = mRegisteredTemplates[templateName]();
    addComponent(ent, dynamic_cast<Component*>(temp));
    finalizeEntity(ent);

    temp->instanceCreated();

    return ent;
}

void EntitySystem::destroyEntity(EntityId entity)
{
    if (entity == 0)
        throw std::runtime_error("Can't destroy a non-existant entity");

    Entity* ent = mEntities[entity];

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
        std::deque<Component*>& comps = it->second;
        while(!comps.empty())
        {
            destroyComponent(comps.back());
        }
    }

    delete ent;
    mEntities[entity] = NULL;
    mEntityC--;
}

bool EntitySystem::finalizeEntity(EntityId entity)
{
    if (entity == 0)
        throw std::runtime_error("Can't finalize a non-existant entity");

    Entity* ent = mEntities[entity];

    if (ent == NULL)
        throw std::runtime_error("Can't finalize a non-existant entity");

    ent->finalised = true;

    if (mRequiredComponents.count(entity) == 0)
        return true;

    std::vector<std::string>& reqs = mRequiredComponents[entity];
    bool destroy = false;

    for (std::vector<std::string>::iterator it = reqs.begin(); it != reqs.end(); ++it)
    {
        if (ent->components.count(*it) == 0)
            destroy = true;
    }

    if (destroy)
        destroyEntity(entity);

    return !destroy;
}

void EntitySystem::registerComponent(const std::string& name, ComponentFactory func)
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

    mComponentC++;
    comp->mEntitySystem = this;
    comp->mId = mComponentCounter++;
    return comp;
}

void EntitySystem::destroyComponent(Component* component)
{
    if (!component->isValid())
        throw std::runtime_error("Can't destroy an invalid component");

    component->setDestroyed();

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
            continue;

        for(auto reg = mGlobalRequests[reqid].begin(); reg != mGlobalRequests[reqid].end();)
        {
            if (reg->component->getId() == component->getId())
                reg = mGlobalRequests[reqid].erase(reg);
            else
                ++reg;
        }
    }

    Entity* ent = mEntities[component->getOwnerId()];

    std::deque<Component*>& comps = ent->components[component->getName()];
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
        freeze(reqid);

        for (auto it = mGlobalRequests[reqid].begin(); it != mGlobalRequests[reqid].end(); ++it)
            it->callback(msg);

        std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
        for (unsigned int i = 0; i < regs.size(); i++)
        {
            regs[i].callback(msg);
        }

        unfreeze(reqid);
    }

    mComponentC--;

    delete component;
}

void EntitySystem::addComponent(EntityId entity, Component* component)
{
    if (component->getOwnerId() != 0)
        throw std::runtime_error("Can't add a component to several entities");

    if (entity == 0)
        throw std::runtime_error("Can't add a component to a non-existant entity");

    Entity* ent = mEntities[entity];

    if (ent == NULL)
        throw std::runtime_error("Can't add a component to a non-existant entity");

    component->setOwner(entity);
    ent->components[component->getName()].push_back(component);

    component->addedToEntity();

    RequestId reqid = getExistingRequestId(Reason_Component, component->getName());

    if (reqid == 0)
        return;

    Message msg(Type_Create, component);

    freeze(reqid);

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

    unfreeze(reqid);
}

void EntitySystem::removeComponent(EntityId entity, Component* component)
{
    if (entity == 0)
        throw std::runtime_error("Can't remove a component from a non-existant entity");

    Entity* ent = mEntities[entity];

    if (ent == NULL)
        throw std::runtime_error("Can't remove a component from a non-existant entity");

    std::deque<Component*>::iterator found;
    std::deque<Component*>& comps = ent->components[component->getName()];
    found = std::find(comps.begin(), comps.end(), component);

    if (found == comps.end())
        throw std::runtime_error("Can't remove a component from an entity that doesn't contain it");

    std::vector<ComponentRequested>& reqs = mRequestsByComponent[component->getId()];
    for (auto it = reqs.begin(); it != reqs.end(); ++it)
    {
        RequestId reqid = getExistingRequestId(it->reason, it->name);

        if (reqid == 0)
            continue;

        for(auto reg = mGlobalRequests[reqid].begin(); reg != mGlobalRequests[reqid].end();)
        {
            if (reg->component->getId() == component->getId())
                reg = mGlobalRequests[reqid].erase(reg);
            else
                ++reg;
        }
    }
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

    component->setOwner(0);
    while ((found = std::find(comps.begin(), comps.end(), component)) != comps.end())
        ent->components[component->getName()].erase(found);

    Message msg(Type_Destroy, component);

    RequestId reqid = getExistingRequestId(Reason_Component, component->getName());

    if (reqid != 0)
    {
        freeze(reqid);

        for (auto it = mGlobalRequests[reqid].begin(); it != mGlobalRequests[reqid].end(); ++it)
            it->callback(msg);

        std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
        for (unsigned int i = 0; i < regs.size(); i++)
        {
            regs[i].callback(msg);
        }

        unfreeze(reqid);
    }
}

std::vector<Component*> EntitySystem::getAllComponentsOnEntity(EntityId entity, const std::string& name)
{
    if (entity == 0)
        throw std::runtime_error("Can't check for components on a non-existant entity");

    Entity* ent = mEntities[entity];

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
        {
            std::vector<Component*> components;
            std::deque<Component*>& comp = ent->components[name];
            components.reserve(comp.size());

            for (auto cit = comp.begin(); cit != comp.end(); ++cit)
                components.push_back(*cit);

            return components;
        }
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

    RequestId reqid = getMessageRequestId(req.reason, req.name);

    if (req.reason != Reason_AllComponents)
    {
        if (isFrozen(reqid))
        {
            mFrozenData.frozenRequests[reqid].globalRequests.push_back(std::pair<ComponentRequested, ComponentRegistered>(req, reg));
            mFrozenData.needsProcessing = true;
            return;
        }

        insertedPush(mGlobalRequests[reqid], reg, RequestSort());

        if (req.reason == Reason_Message)
        {
           insertedPush(mEntities[reg.component->getOwnerId()]->localRequests[reqid], reg, RequestSort());
        }

        if (reg.required && !mEntities[reg.component->getOwnerId()]->finalised)
            mRequiredComponents[reg.component->getOwnerId()].push_back(req.name);

        mRequestsByComponent[reg.component->getId()].push_back(req);
    }

    if (req.reason == Reason_Message)
        return;

    freeze(reqid);

    Message msg(Type_Create);
    for (unsigned int i = 0; i < mEntities.size(); i++)
    {
        if (mEntities[i] == 0)
            continue;

        if (mEntities[i]->components.count(req.name) == 0)
            continue;

        std::deque<Component*>& comps = mEntities[i]->components[req.name];
        for (auto it = comps.begin(); it != comps.end(); ++it)
        {
            if ((*it)->isValid() && reg.component->getId() != (*it)->getId())
            {
                msg.sender = (*it);

                reg.callback(msg);
            }
        }
    }

    unfreeze(reqid);
}

void EntitySystem::registerLocalRequest(const ComponentRequested& req, const ComponentRegistered& reg)
{
    RequestId reqid = getMessageRequestId(req.reason, req.name);

    if (isFrozen(reqid))
    {
        mFrozenData.frozenRequests[reqid].localRequests.push_back(std::pair<ComponentRequested, ComponentRegistered>(req, reg));
        mFrozenData.needsProcessing = true;
        return;
    }

    Entity* ent = mEntities[reg.component->getOwnerId()];

    std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
    insertedPush(regs, reg, RequestSort());

    if (reg.required && !ent->finalised)
        mRequiredComponents[ent->id].push_back(req.name);

    if (req.reason != Reason_Component)
        return;

    Message msg(Type_Create);

    if (ent->components.count(req.name) == 0)
        return;

    freeze(reqid);

    std::deque<Component*>& comps = ent->components[req.name];
    for (auto it = comps.begin(); it != comps.end(); ++it)
    {
        if ((*it)->isValid() && reg.component->getId() != (*it)->getId())
        {
            msg.sender = *it;

            reg.callback(msg);
        }
    }

    unfreeze(reqid);
}

void EntitySystem::removeGlobalRequest(const ComponentRequested& req, const ComponentRegistered& reg)
{
    if (!reg.component->isValid())
        throw std::runtime_error("Can't remove a request from an invalid component");

    RequestId reqid = getMessageRequestId(req.reason, req.name);

    if (isFrozen(reqid))
    {
        mFrozenData.frozenRequests[reqid].globalRequestRemoves.push_back(std::pair<ComponentRequested, ComponentRegistered>(req, reg));
        mFrozenData.needsProcessing = true;
        return;
    }

    freeze(reqid);

    if (mGlobalRequests.count(reqid) > 0)
    {
        std::deque<ComponentRegistered>& regs = mGlobalRequests[reqid];
        for (auto it = regs.begin(); it != regs.end(); ++it)
            if (it->component == reg.component)
            {
                regs.erase(it);

                if (req.reason == Reason_Message)
                {
                    Entity* ent = mEntities[reg.component->getOwnerId()];

                    if (ent->localRequests.count(reqid) > 0)
                    {
                        std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
                        for (auto it = regs.begin(); it != regs.end(); ++it)
                            if (it->component == reg.component)
                            {
                                regs.erase(it);
                                break;
                            }
                    }
                }

                std::vector<ComponentRequested>& reqs = mRequestsByComponent[reg.component->getId()];
                auto it2 = std::find_if(reqs.begin(), reqs.end(), RequestFind(req));
                if (it2 != reqs.end())
                    reqs.erase(it2);

                break;
        }
    }

    unfreeze(reqid);
}

void EntitySystem::removeLocalRequest(const ComponentRequested& req, const ComponentRegistered& reg)
{
    if (!reg.component->isValid())
        throw std::runtime_error("Can't remove a request from an invalid component");

    RequestId reqid = getMessageRequestId(req.reason, req.name);

    if (isFrozen(reqid))
    {
        mFrozenData.frozenRequests[reqid].localRequestRemoves.push_back(std::pair<ComponentRequested, ComponentRegistered>(req, reg));
        mFrozenData.needsProcessing = true;
        return;
    }

    freeze(reqid);

    Entity* ent = mEntities[reg.component->getOwnerId()];

    if (ent->localRequests.count(reqid) > 0)
    {
        std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
        for (auto it = regs.begin(); it != regs.end(); ++it)
            if (it->component == reg.component)
            {
                regs.erase(it);
                break;
            }
    }

    unfreeze(reqid);
}

void EntitySystem::reprioritizeRequest(Component* comp, RequestId reqid, int priority)
{
    if (isFrozen(reqid))
    {
        mFrozenData.frozenRequests[reqid].repriorities.push_back(std::pair<Component*, std::pair<RequestId, int> >(comp, std::pair<RequestId, int>(reqid, priority)));
        mFrozenData.needsProcessing = true;
        return;
    }

    freeze(reqid);

    Entity* ent = mEntities[comp->getOwnerId()];
    if (ent->localRequests.count(reqid) > 0)
    {
        std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
        for (auto it = regs.begin(); it != regs.end(); ++it)
            if (it->component == comp)
            {
                ComponentRegistered reg = *it;
                regs.erase(it);

                reg.priority = priority;

                insertedPush(regs, reg, RequestSort());
                break;
            }
    }

    if (mGlobalRequests.count(reqid) > 0)
    {
        std::deque<ComponentRegistered>& regs = mGlobalRequests[reqid];
        for (auto it = regs.begin(); it != regs.end(); ++it)
            if (it->component == comp)
            {
                ComponentRegistered reg = *it;
                regs.erase(it);

                reg.priority = priority;

                insertedPush(regs, reg, RequestSort());
                break;
            }
    }

    unfreeze(reqid);
}

void EntitySystem::sendGlobalMessage(RequestId reqid, Message& msg)
{
    if (msg.sender != 0 && !msg.sender->isValid())
        throw std::runtime_error("Invalid sender for global message");

    freeze(reqid);

    for (auto it = mGlobalRequests[reqid].begin(); it != mGlobalRequests[reqid].end(); ++it)
    {
        it->callback(msg);

        if (msg.handled)
        {
            msg.sender = it->component;
            break;
        }
    }

    unfreeze(reqid);
}

void EntitySystem::sendLocalMessage(EntityId entity, RequestId reqid, Message& msg)
{
    if (entity == 0)
        throw std::runtime_error("Can't send a message to a non-existant entity");

    Entity* ent = mEntities[entity];

    if (ent == NULL)
        throw std::runtime_error("Can't send a message to a non-existant entity");

    if (ent->localRequests.count(reqid) == 0)
        return;

    freeze(reqid);

    std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
    for (auto it = regs.begin(); it != regs.end(); ++it)
    {
        auto& reg = *it;
        reg.callback(msg);

        if (msg.handled)
        {
            msg.sender = reg.component;
            break;
        }
    }

    if (!msg.handled)
    {
        for (auto it = mGlobalRequests[reqid].begin(); it != mGlobalRequests[reqid].end(); ++it)
        {
            if (it->component->getOwnerId() != entity)
                continue;

            it->callback(msg);

            if (msg.handled)
            {
                msg.sender = it->component;
                break;
            }
        }
    }

    unfreeze(reqid);
}

void EntitySystem::freeze(RequestId rid)
{
    FrozenData::RequestLock& lock = mFrozenData.frozenRequests[rid];

#ifdef Kunlaboro_BOOST
    if (mThreaded)
    {
        boost::thread::id cur_thread = boost::this_thread::get_id();

        if (lock.locked && lock.owner == cur_thread)
            return;

        lock.mutex->lock();
        lock.owner = cur_thread;
    }
    else
#endif
    if (lock.locked)
        return;

    mFrozen++;
    lock.locked = true;
}

void EntitySystem::unfreeze(RequestId rid)
{
    FrozenData::RequestLock& lock = mFrozenData.frozenRequests[rid];

    if (!lock.locked) throw std::runtime_error("Tried to unfreeze a request that wasn't frozen!");
    lock.locked = false;

    if (--mFrozen < 0) mFrozen = 0;

    std::list<std::pair<Component*, std::pair<RequestId, int> > >& repriorities = lock.repriorities;
    std::list<std::pair<ComponentRequested, ComponentRegistered>>& localRequests = lock.localRequests;
    std::list<std::pair<ComponentRequested, ComponentRegistered>>& localRequestRemoves = lock.localRequestRemoves;
    std::list<std::pair<ComponentRequested, ComponentRegistered>>& globalRequests = lock.globalRequests;
    std::list<std::pair<ComponentRequested, ComponentRegistered>>& globalRequestRemoves = lock.globalRequestRemoves;

    for (auto it = localRequests.begin(); it != localRequests.end(); ++it)
        registerLocalRequest(it->first, it->second);

    for (auto it = globalRequests.begin(); it != globalRequests.end(); ++it)
        registerGlobalRequest(it->first, it->second);

    for (auto it = repriorities.begin(); it != repriorities.end(); ++it)
        reprioritizeRequest(it->first, it->second.first, it->second.second);

    for (auto it = localRequestRemoves.begin(); it != localRequestRemoves.end(); ++it)
        removeLocalRequest(it->first, it->second);

    for (auto it = globalRequestRemoves.begin(); it != globalRequestRemoves.end(); ++it)
        removeGlobalRequest(it->first, it->second);

    repriorities.clear();
    localRequests.clear();
    localRequestRemoves.clear();
    globalRequests.clear();
    globalRequestRemoves.clear();

    //mFrozenData.frozenRequests.erase(rid);

    if (mFrozen <= 0 && mFrozenData.needsProcessing)
    {
        mFrozenData.needsProcessing = false;
        //Process frozen queues

        std::list<Component*> frozenComponentDestructions = mFrozenData.frozenComponentDestructions;
        std::list<EntityId> frozenEntityDestructions = mFrozenData.frozenEntityDestructions;
        mFrozenData.frozenComponentDestructions.clear();
        mFrozenData.frozenEntityDestructions.clear();

        for (auto it = frozenComponentDestructions.begin(); it != frozenComponentDestructions.end(); ++it)
            destroyComponent(*it);

        for (auto it = frozenEntityDestructions.begin(); it != frozenEntityDestructions.end(); ++it)
            destroyEntity(*it);
    }

#ifdef Kunlaboro_BOOST
    if (mThreaded)
    {
        lock.owner = boost::thread::id();
        lock.mutex->unlock();
    }
#endif
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
