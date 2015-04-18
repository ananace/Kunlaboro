#include <Kunlaboro/EntitySystem.hpp>
#include <Kunlaboro/Component.hpp>
#include <stdexcept>
#include <algorithm>
#include <iterator>

using namespace Kunlaboro;

inline bool RequestSort(const ComponentRegistered& a, const ComponentRegistered& b)
{
    return a.priority < b.priority;
}

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
        return req->hash == b.hash && req->reason == b.reason;
    }
};

template<typename T, typename Y>
inline void insertedPush(std::deque<T>& deque, const T& value, Y comp)
{
    deque.insert(std::lower_bound(deque.begin(), deque.end(), value, comp), value);
}

EntitySystem::EntitySystem(bool thread) :
    mComponentCounter(1), mEntityCounter(1), mThreaded(thread), mEntityC(0), mComponentC(0)
{
    
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
    
    auto& temp = mRegisteredTemplates[templateName];
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

    component->setDestroyed();

	auto reqcopy = mRequestsByComponent[component->getId()];
	for (auto& req : reqcopy)
	{
		std::deque<ComponentRegistered> copy;
		if (req.reason == Reason_Message)
			copy = mGlobalMessageRequests[req.hash];
		else
			copy = mGlobalComponentRequests[req.hash];

		for (auto& reg : copy)
			if (reg.component == component)
				removeGlobalRequest(req, reg);
	}

    mRequestsByComponent.erase(component->getId());

	Entity* ent = mEntities[component->getOwnerId()];

	{
		std::deque<Component*>& comps = ent->components[component->getName()];
		auto toRemove = std::remove_if(comps.begin(), comps.end(), [component](Component* req) { return req == component; });
		comps.erase(toRemove, comps.end());
	}

    for (auto& regs : ent->localComponentRequests)
    {
        auto toRemove = std::remove_if(regs.second.begin(), regs.second.end(), [component](ComponentRegistered& req) { return req.component == component; });
        regs.second.erase(toRemove, regs.second.end());
    }
	for (auto& regs : ent->localMessageRequests)
	{
		auto toRemove = std::remove_if(regs.second.begin(), regs.second.end(), [component](ComponentRegistered& req) { return req.component == component; });
		regs.second.erase(toRemove, regs.second.end());
	}

    RequestId reqid = hash::hashString(component->getName());

    if (reqid != 0)
    {
        auto reqs = mGlobalComponentRequests[reqid];
        for (auto& it : reqs)
        {
            if (it.component != component)
                (*reinterpret_cast<ComponentCallback*>(&it.functional))(component, Type_Destroy);
                
        }

        reqs = ent->localComponentRequests[reqid];
        for (auto& it : reqs)
        {
            (*reinterpret_cast<ComponentCallback*>(&it.functional))(component, Type_Destroy);
        }
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

    RequestId reqid = hash::hashString(component->getName());

    if (reqid == 0)
        return;

    if (mGlobalComponentRequests.count(reqid) > 0)
    {
        auto reqs = mGlobalComponentRequests.at(reqid);
        for (auto& it : reqs)
        {
            if (it.component != component)
                (*reinterpret_cast<ComponentCallback*>(&it.functional))(component, Type_Create);
        }
    }

    if (ent->localComponentRequests.count(reqid) > 0)
    {
        auto reqs = ent->localComponentRequests.at(reqid);
        for (auto& it : reqs)
        {
            (*reinterpret_cast<ComponentCallback*>(&it.functional))(component, Type_Create);
        }
    }
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

	for (auto& req : mRequestsByComponent[component->getId()])
	{
		std::deque<ComponentRegistered> copy;
		if (req.reason == Reason_Message)
			copy = mGlobalMessageRequests[req.hash];
		else
			copy = mGlobalComponentRequests[req.hash];

		for (auto& reg : copy)
			removeGlobalRequest(req, reg);
	}
    mRequestsByComponent.erase(component->getId());

	auto toRemove = std::remove_if(comps.begin(), comps.end(), [component](Component* req) { return req == component; });
	comps.erase(toRemove, comps.end());

    for (auto& regs : ent->localComponentRequests)
	{
		auto toRemove = std::remove_if(regs.second.begin(), regs.second.end(), [component](ComponentRegistered& req) { return req.component == component; });
		regs.second.erase(toRemove, regs.second.end());
	}
    for (auto& regs : ent->localMessageRequests)
    {
        auto toRemove = std::remove_if(regs.second.begin(), regs.second.end(), [component](ComponentRegistered& req) { return req.component == component; });
        regs.second.erase(toRemove, regs.second.end());
    }

    component->setOwner(0);
	comps.erase(std::remove(comps.begin(), comps.end(), component), comps.end());

    RequestId reqid = hash::hashString(component->getName());

    if (reqid == 0)
        return;

    if (mGlobalComponentRequests.count(reqid) > 0)
    {
        auto requests = mGlobalComponentRequests.at(reqid);
        for (auto& it : requests)
        {
            if (it.component != component)
                (*reinterpret_cast<ComponentCallback*>(&it.functional))(component, Type_Destroy);
        }
    }

    if (ent->localComponentRequests.count(reqid) > 0)
    {
        auto requests = ent->localComponentRequests.at(reqid);
        for (auto& it : requests)
        {
            (*reinterpret_cast<ComponentCallback*>(&it.functional))(component, Type_Destroy);
        }
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

void EntitySystem::registerGlobalRequest(const ComponentRequested& req, const ComponentRegistered& reg)
{
    if (!reg.component->isValid())
        throw std::runtime_error("Can't register a request from an invalid component");

    RequestId reqid = req.hash;

    if (reqid == 0)
        throw std::runtime_error("Invalid request hash");

    if (req.reason == Reason_Component)
        insertedPush(mGlobalComponentRequests[reqid], reg, RequestSort);
    else
        insertedPush(mGlobalMessageRequests[reqid], reg, RequestSort);

    if (reg.required && !mEntities[reg.component->getOwnerId()]->finalised)
        mRequiredComponents[reg.component->getOwnerId()].push_back(req.name);

    mRequestsByComponent[reg.component->getId()].push_back(req);

    if (req.reason == Reason_Message)
        return;

    for (auto& ent : mEntities)
    {
        if (!ent.second)
            continue;
        if (ent.second->components.count(req.name) == 0)
			continue;

        std::deque<Component*>& comps = ent.second->components.at(req.name);
        for (auto& it : comps)
        {
            if (it->isValid() && reg.component->getId() != it->getId())
            {
                (*reinterpret_cast<ComponentCallback*>(const_cast<std::function<void()>*>(&reg.functional)))(it, Type_Create);
            }
        }
    }
}

void EntitySystem::registerLocalRequest(const ComponentRequested& req, const ComponentRegistered& reg)
{
    RequestId reqid = req.hash;

    if (reqid == 0)
        throw std::runtime_error("Invalid request hash");

    Entity* ent = mEntities[reg.component->getOwnerId()];

    if (req.reason == Reason_Component)
        insertedPush(ent->localComponentRequests[reqid], reg, RequestSort);
    else
        insertedPush(ent->localMessageRequests[reqid], reg, RequestSort);

    if (reg.required && !ent->finalised)
        mRequiredComponents[ent->id].push_back(req.name);

    if (req.reason == Reason_Message)
        return;

    if (ent->components.count(req.name) == 0)
        return;

    std::deque<Component*>& comps = ent->components.at(req.name);
	for (auto& it : comps)
	{
		if (it->isValid() && reg.component->getId() != it->getId())
		{
            (*reinterpret_cast<ComponentCallback*>(const_cast<std::function<void()>*>(&reg.functional)))(it, Type_Create);
		}
	}
}

void EntitySystem::removeGlobalRequest(const ComponentRequested& req, const ComponentRegistered& reg)
{
    if (!reg.component->isValid())
        throw std::runtime_error("Can't remove a request from an invalid component");

    RequestId reqid = req.hash;

    if (reqid == 0)
        throw std::runtime_error("Invalid request hash");

    RequestMap* chooser = &mGlobalComponentRequests;
    if (req.reason == Reason_Message)
        chooser = &mGlobalMessageRequests;

    RequestMap& reqMap = *chooser;

    if (reqMap.count(reqid) > 0)
    {
        std::deque<ComponentRegistered>& regs = reqMap[reqid];
		auto toRemove = std::remove_if(regs.begin(), regs.end(), [this, reg, req, reqid](ComponentRegistered& it) {
			if (it.component != reg.component)
				return false;

			if (req.reason == Reason_Message)
			{
				Entity* ent = mEntities[reg.component->getOwnerId()];

				if (ent->localMessageRequests.count(reqid) > 0)
				{
					std::deque<ComponentRegistered>& regs = ent->localMessageRequests[reqid];
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

        if (reqMap[reqid].empty())
            reqMap.erase(reqid);
    }
}

void EntitySystem::removeLocalRequest(const ComponentRequested& req, const ComponentRegistered& reg)
{
    if (!reg.component->isValid())
        throw std::runtime_error("Can't remove a request from an invalid component");

    RequestId reqid = req.hash;

    if (reqid == 0)
        throw std::runtime_error("Invalid request hash");

    Entity* ent = mEntities[reg.component->getOwnerId()];

    RequestMap* chooser = &ent->localComponentRequests;
    if (req.reason == Reason_Message)
        chooser = &ent->localMessageRequests;

    RequestMap& reqMap = *chooser;

    if (reqMap.count(reqid) > 0)
	{
        std::deque<ComponentRegistered>& regs = reqMap[reqid];
		regs.erase(std::remove_if(regs.begin(), regs.end(), [reg](ComponentRegistered& it) {return it.component == reg.component; }), regs.end());

        if (reqMap[reqid].empty())
            reqMap.erase(reqid);
	}
}

void EntitySystem::reprioritizeRequest(Component* comp, RequestId reqid, int priority)
{
    if (reqid == 0)
        throw std::runtime_error("Invalid request hash");

    Entity* ent = mEntities[comp->getOwnerId()];

	if (ent->localMessageRequests.count(reqid) > 0)
    {
        std::deque<ComponentRegistered>& regs = ent->localMessageRequests.at(reqid);
        for (auto it = regs.begin(); it != regs.end(); ++it)
            if (it->component == comp)
            {
                ComponentRegistered reg = *it;
                regs.erase(it);

                reg.priority = priority;

                insertedPush(regs, reg, RequestSort);
                break;
            }
    }

    if (mGlobalMessageRequests.count(reqid) > 0)
    {
        std::deque<ComponentRegistered>& regs = mGlobalMessageRequests.at(reqid);
        for (auto it = regs.begin(); it != regs.end(); ++it)
            if (it->component == comp)
            {
                ComponentRegistered reg = *it;
                regs.erase(it);

                reg.priority = priority;

                insertedPush(regs, reg, RequestSort);
                break;
            }
    }
}
