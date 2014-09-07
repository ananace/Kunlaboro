#include <Kunlaboro/EntitySystem.hpp>
#include <Kunlaboro/Component.hpp>
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
    
    auto temp = mRegisteredTemplates[templateName];
	std::for_each(temp.begin(), temp.end(), [this, ent](const std::string& comp) { addComponent(ent, comp); });

    if (finalizeEntity(ent))
        return ent;
    else
        return 0;
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

	for (auto& it : ent->components)
	{
		auto copy = it.second;
		for (auto& comp : copy)
			destroyComponent(comp);
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

	auto it = std::find_if(reqs.begin(), reqs.end(), [ent](const std::string& str) { return (ent->components.count(str) == 0); });

	if (it != reqs.end())
        destroyEntity(entity);

	return it == reqs.end();
}

void EntitySystem::registerTemplate(const std::string& name, const std::vector<std::string>& components)
{
    // TODO: Decide if this should be a one-time thing, or allow changing templates.
    mRegisteredTemplates[name] = components;
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
    {
        delete component;
        return;
    }
        //throw std::runtime_error("Can't destroy an invalid component");

    component->setDestroyed();

    if (isFrozen())
    {
        mFrozenData.frozenComponentDestructions.push_back(component);
        mFrozenData.needsProcessing = true;
        return;
    }

    std::vector<ComponentRequested>& reqs = mRequestsByComponent[component->getId()];
    for (auto& it : reqs)
    {
        RequestId reqid = getExistingRequestId(it.reason, it.name);

        if (reqid == 0)
            continue;

		auto& reqList = mGlobalRequests[reqid];
		auto toRemove = std::remove_if(reqList.begin(), reqList.end(), [component](ComponentRegistered& req) { return req.component == component; });
		reqList.erase(toRemove, reqList.end());
    }

	Entity* ent = mEntities[component->getOwnerId()];

	{
		std::deque<Component*>& comps = ent->components[component->getName()];
		auto toRemove = std::remove_if(comps.begin(), comps.end(), [component](Component* req) { return req == component; });
		comps.erase(toRemove, comps.end());
	}

	for (auto& regs : ent->localRequests)
	{
		auto toRemove = std::remove_if(regs.second.begin(), regs.second.end(), [component](ComponentRegistered& req) { return req.component == component; });
		regs.second.erase(toRemove, regs.second.end());
	}

    Message msg(Type_Destroy, component);

    RequestId reqid = getExistingRequestId(Reason_Component, component->getName());

    if (reqid != 0)
    {
        freeze(reqid);

        for (auto& it : mGlobalRequests[reqid])
            it.callback(msg);

        for (auto& it : ent->localRequests[reqid])
            it.callback(msg);

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
        for (auto& it : mGlobalRequests[reqid])
        {
            if (it.component != component)
                it.callback(msg);
        }

    if (ent->localRequests.count(reqid) > 0)
        for (auto& it : ent->localRequests[reqid])
        {
            it.callback(msg);
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

    std::deque<Component*>& comps = ent->components[component->getName()];
    auto found = std::find(comps.begin(), comps.end(), component);

    if (found == comps.end())
        throw std::runtime_error("Can't remove a component from an entity that doesn't contain it");

    std::vector<ComponentRequested>& reqs = mRequestsByComponent[component->getId()];
    for (auto& it : reqs)
    {
        RequestId reqid = getExistingRequestId(it.reason, it.name);

        if (reqid == 0)
            continue;

		auto& reqList = mGlobalRequests[reqid];
		auto toRemove = std::remove_if(reqList.begin(), reqList.end(), [component](ComponentRegistered& req) { return req.component == component; });
		reqList.erase(toRemove, reqList.end());
    }

	auto toRemove = std::remove_if(comps.begin(), comps.end(), [component](Component* req) { return req == component; });
	comps.erase(toRemove, comps.end());

    for (auto& regs : ent->localRequests)
	{
		auto toRemove = std::remove_if(regs.second.begin(), regs.second.end(), [component](ComponentRegistered& req) { return req.component == component; });
		regs.second.erase(toRemove, regs.second.end());
	}

    component->setOwner(0);
	comps.erase(std::remove(comps.begin(), comps.end(), component), comps.end());

    Message msg(Type_Destroy, component);

    RequestId reqid = getExistingRequestId(Reason_Component, component->getName());

    if (reqid != 0)
    {
        freeze(reqid);

		if (mGlobalRequests.count(reqid) > 0)
		for (auto& it : mGlobalRequests[reqid])
		{
			if (it.component != component)
				it.callback(msg);
		}

		if (ent->localRequests.count(reqid) > 0)
		for (auto& it : ent->localRequests[reqid])
		{
			it.callback(msg);
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

	std::vector<Component*> components;
	auto insert = std::back_inserter(components);

    if (name == "")
    {
		components.reserve(ent->components.size());

        for (auto& it : ent->components)
			std::copy(it.second.begin(), it.second.end(), insert);
    }
    else
    {
        if (ent->components.count(name) > 0)
        {
            std::deque<Component*>& comp = ent->components[name];
            components.reserve(comp.size());

			std::copy(comp.begin(), comp.end(), insert);
        }
    }

	return components;
}

RequestId EntitySystem::getMessageRequestId(MessageReason reason, const std::string& name)
{
    if (reason == Reason_AllComponents)
        reason = Reason_Component;

	auto& nameMap = mNameMap[reason];
	if (nameMap.count(name) == 0)
    {
		nameMap[name] = mRequestCounter;
        mIdMap[reason][mRequestCounter] = name;

        return mRequestCounter++;
    }

	return nameMap[name];
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

        if (reg.required && !mEntities[reg.component->getOwnerId()]->finalised)
            mRequiredComponents[reg.component->getOwnerId()].push_back(req.name);

        mRequestsByComponent[reg.component->getId()].push_back(req);
    }

    if (req.reason == Reason_Message)
        return;

    freeze(reqid);

    Message msg(Type_Create);
    for (auto& ent : mEntities)
    {
		if (ent.second == nullptr || ent.second->components.count(req.name) == 0)
			continue;

        std::deque<Component*>& comps = ent.second->components[req.name];
        for (auto& it : comps)
        {
            if (it->isValid() && reg.component->getId() != it->getId())
            {
                msg.sender = it;
                reg.callback(msg);

				msg.handled = false;
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
	for (auto& it : comps)
	{
		if (it->isValid() && reg.component->getId() != it->getId())
		{
			msg.sender = it;
			reg.callback(msg);

			msg.handled = false;
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
		auto toRemove = std::remove_if(regs.begin(), regs.end(), [this, reg, req, reqid](ComponentRegistered& it) {
			if (it.component != reg.component)
				return false;

			if (req.reason == Reason_Message)
			{
				Entity* ent = mEntities[reg.component->getOwnerId()];

				if (ent->localRequests.count(reqid) > 0)
				{
					std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
					regs.erase(std::remove_if(regs.begin(), regs.end(), [reg](ComponentRegistered& it) {return it.component == reg.component; }), regs.end());
				}
			}

			std::vector<ComponentRequested>& reqs = mRequestsByComponent[reg.component->getId()];
			auto it2 = std::find_if(reqs.begin(), reqs.end(), RequestFind(req));
			if (it2 != reqs.end())
				reqs.erase(it2);

			return true;
		});
        
		regs.erase(toRemove, regs.end());
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
		regs.erase(std::remove_if(regs.begin(), regs.end(), [reg](ComponentRegistered& it) {return it.component == reg.component; }), regs.end());
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
	RequestSort sort;

	if (ent->localRequests.count(reqid) > 0)
    {
        std::deque<ComponentRegistered>& regs = ent->localRequests[reqid];
        for (auto it = regs.begin(); it != regs.end(); ++it)
            if (it->component == comp)
            {
                ComponentRegistered reg = *it;
                regs.erase(it);

                reg.priority = priority;

                insertedPush(regs, reg, sort);
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

                insertedPush(regs, reg, sort);
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

    for (auto& it : mGlobalRequests[reqid])
    {
        it.callback(msg);

        if (msg.handled)
        {
            msg.sender = it.component;
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

	for (auto& it : ent->localRequests[reqid])
    {
        it.callback(msg);

        if (msg.handled)
        {
            msg.sender = it.component;
            break;
        }
    }

    unfreeze(reqid);
}

void EntitySystem::freeze(RequestId rid)
{
    FrozenData::RequestLock& lock = mFrozenData.frozenRequests[rid];

    if (mThreaded)
        lock.mutex.lock();
    else if (lock.locked)
        return;

    mFrozen++;
    lock.locked = true;
}

void EntitySystem::unfreeze(RequestId rid)
{
    FrozenData::RequestLock& lock = mFrozenData.frozenRequests[rid];

    if (!lock.locked) throw std::runtime_error("Tried to unfreeze a request that wasn't frozen!");

    if (--mFrozen < 0) mFrozen = 0;

    auto& repriorities = lock.repriorities;
	auto& localRequests = lock.localRequests;
	auto& localRequestRemoves = lock.localRequestRemoves;
	auto& globalRequests = lock.globalRequests;
	auto& globalRequestRemoves = lock.globalRequestRemoves;

    for (auto& it : localRequests)
        registerLocalRequest(it.first, it.second);

    for (auto& it : globalRequests)
        registerGlobalRequest(it.first, it.second);

    for (auto& it : repriorities)
        reprioritizeRequest(it.first, it.second.first, it.second.second);

    for (auto& it : localRequestRemoves)
        removeLocalRequest(it.first, it.second);

    for (auto& it : globalRequestRemoves)
        removeGlobalRequest(it.first, it.second);

    //mFrozenData.frozenRequests.erase(rid);

    if (mFrozen <= 0 && mFrozenData.needsProcessing)
    {
        mFrozenData.needsProcessing = false;
        //Process frozen queues

        auto frozenComponentDestructions = mFrozenData.frozenComponentDestructions;
        auto frozenEntityDestructions = mFrozenData.frozenEntityDestructions;
        mFrozenData.frozenComponentDestructions.clear();
        mFrozenData.frozenEntityDestructions.clear();

        for (auto& it : frozenComponentDestructions)
            destroyComponent(it);

        for (auto& it : frozenEntityDestructions)
            destroyEntity(it);
    }

    lock.locked = false;

    if (mThreaded)
        lock.mutex.unlock();
}

RequestId EntitySystem::getExistingRequestId(MessageReason reason, const std::string& name)
{
	auto& nameMap = mNameMap[reason];
    if (nameMap.count(name) == 0)
        return 0;

	RequestId reqid = nameMap[name];
    //if (mGlobalRequests.count(reqid) == 0)
    //    return 0;

    return reqid;
}
