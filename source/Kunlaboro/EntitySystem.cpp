#include <Kunlaboro/EntitySystem.hpp>
#include <Kunlaboro/EntitySystem.inl>
#include <Kunlaboro/Entity.hpp>
#include <Kunlaboro/EventSystem.hpp>
#include <Kunlaboro/EventSystem.inl>
#include <Kunlaboro/MessageSystem.hpp>
#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/Component.inl>

#include <Kunlaboro/detail/ComponentPool.hpp>

#include <cassert>

using namespace Kunlaboro;

EntitySystem::EntitySystem()
	: mEventSystem(nullptr)
	, mMessageSystem(nullptr)
{

}
EntitySystem::~EntitySystem()
{
	for (auto& comp : mComponentFamilies)
		if (comp.MemoryPool)
			delete comp.MemoryPool;

	if (mEventSystem)
		delete mEventSystem;
	if (mMessageSystem)
		delete mMessageSystem;
}

ComponentHandle<Component> EntitySystem::getComponent(ComponentId id) const
{
	if (!componentAlive(id))
		return ComponentHandle<Component>();

	auto& data = mComponentFamilies[id.getFamily()];
	return ComponentHandle<Component>(static_cast<Component*>(data.MemoryPool->getData(id.getIndex())), const_cast<std::atomic_ushort*>(data.Components[id.getIndex()].RefCount));
}
Entity EntitySystem::getEntity(EntityId id) const
{
	return Entity(const_cast<EntitySystem*>(this), id);
}

Entity EntitySystem::entityCreate()
{
	EntityId::IndexType id;
	if (mFreeEntityIndices.empty())
	{
		mEntities.push_back({ 0 });
		id = mEntities.size() - 1;

		assert(id < EntityId::sMaxIndex);
	}
	else
	{
		id = mFreeEntityIndices.front();
		mFreeEntityIndices.pop_front();
	}

	auto eid = EntityId(id, mEntities[id].Generation);
	if (mEventSystem)
		mEventSystem->eventEmit<EntityCreatedEvent>(eid, this);

	return Entity(this, eid);
}
void EntitySystem::entityDestroy(EntityId id)
{
	if (!entityAlive(id))
		return;

	auto& entity = mEntities[id.getIndex()];
	const auto* components = entity.Components.data();
	for (ComponentId::FamilyType family = 0; family < entity.Components.size(); ++family)
	{
		if (!entity.ComponentBits.hasBit(family) || !componentAlive(components[family]))
			continue;

		componentDestroy(components[family]);
	}

	++mEntities[id.getIndex()].Generation;
	mFreeEntityIndices.push_back(id.getIndex());

	if (mEventSystem)
		mEventSystem->eventEmit<EntityDestroyedEvent>(id, this);
}

bool EntitySystem::entityAlive(EntityId id) const
{
	return id.getIndex() < mEntities.size() && mEntities[id.getIndex()].Generation == id.getGeneration();
}

ComponentHandle<Component> EntitySystem::entityGetComponent(ComponentId::FamilyType family, EntityId eid) const
{
	if (!entityAlive(eid))
		return ComponentHandle<Component>();

	auto& entity = mEntities[eid.getIndex()];
	if (entity.Components.size() <= family || !entity.ComponentBits.hasBit(family))
		return ComponentHandle<Component>();

	auto cid = entity.Components[family];
	if (!componentAlive(cid))
		return ComponentHandle<Component>();

	return getComponent(cid);
}
bool EntitySystem::entityHasComponent(ComponentId::FamilyType family, EntityId eid) const
{
	if (!entityAlive(eid))
		return false;

	auto& entity = mEntities[eid.getIndex()];
	if (entity.Components.size() <= family || !entity.ComponentBits.hasBit(family))
		return false;

	return componentAlive(entity.Components[family]);
}

void EntitySystem::componentDestroy(ComponentId id)
{
	if (!componentAlive(id))
		return;

	auto& data = mComponentFamilies[id.getFamily()];
	if (!data.MemoryPool->hasBit(id.getIndex()))
		return;

	EntityId::IndexType eid = 0;
	for (auto& ent : mEntities)
	{
		if (ent.ComponentBits.hasBit(id.getFamily()) && ent.Components[id.getFamily()] == id)
		{
			componentDetach(id, EntityId(eid, ent.Generation));
			break;
		}
		++eid;
	}

	if (mEventSystem)
		mEventSystem->eventUnregisterAll(id);
	if (mMessageSystem)
		mMessageSystem->messageUnrequestAll(id);

	data.MemoryPool->destroy(id.getIndex());
	data.MemoryPool->resetBit(id.getIndex());
	auto& comp = data.Components[id.getIndex()];
	comp.RefCount->store(0);
	++comp.Generation;

	data.FreeIndices.push_back(id.getIndex());

	if (mEventSystem)
		mEventSystem->eventEmit<ComponentDestroyedEvent>(id, this);
}
inline bool EntitySystem::componentAlive(ComponentId id) const
{
	if (mComponentFamilies.size() <= id.getFamily())
		return false;

	auto& components = mComponentFamilies[id.getFamily()];
	if (components.Components.size() <= id.getIndex())
		return false;

	auto& component = components.Components[id.getIndex()];
	return component.Generation == id.getGeneration() && components.MemoryPool->hasBit(id.getIndex());
}

bool EntitySystem::componentAttached(ComponentId cid, EntityId eid) const
{
	if (!entityAlive(eid) || !componentAlive(cid))
		return false;

	auto& entity = mEntities[eid.getIndex()];
	if (entity.Components.size() <= cid.getFamily())
		return false;

	return entity.ComponentBits.hasBit(cid.getFamily()) && entity.Components[cid.getFamily()] == cid;
}
void EntitySystem::componentAttach(ComponentId cid, EntityId eid, bool checkDetach)
{
	if (!entityAlive(eid) || !componentAlive(cid))
		return;

	auto& entity = mEntities[eid.getIndex()];
	while (entity.Components.size() <= cid.getFamily())
		entity.Components.push_back(ComponentId::Invalid());

	if (checkDetach)
	{
		auto comp = getComponent(cid);
		if (comp->getEntityId() == eid)
			return;

		if (componentGetEntity(cid) != EntityId())
			componentDetach(cid, comp->getEntityId());

		if (entity.Components[cid.getFamily()] != ComponentId::Invalid())
			componentDetach(entity.Components[cid.getFamily()], eid);

		comp.unlink();
	}

	entity.ComponentBits.setBit(cid.getFamily());
	entity.Components[cid.getFamily()] = cid;

	if (mEventSystem)
		mEventSystem->eventEmit<ComponentAttachedEvent>(cid, eid, this);
}
void EntitySystem::componentDetach(ComponentId cid, EntityId eid)
{
	if (!entityAlive(eid) || !componentAlive(cid))
		return;

	auto& entity = mEntities[eid.getIndex()];
	if (entity.Components.size() <= cid.getFamily())
		return;

	auto comp = getComponent(cid);
	if (comp->getEntityId() != eid)
		return;

	entity.ComponentBits.clearBit(cid.getFamily());
	entity.Components[cid.getFamily()] = ComponentId::Invalid();

	if (mEventSystem)
		mEventSystem->eventEmit<ComponentDetachedEvent>(cid, eid, this);

	comp.release();
}
EntityId EntitySystem::componentGetEntity(ComponentId cid) const
{
	if (!componentAlive(cid))
		return EntityId();

	EntityId::IndexType i = 0;
	for (auto& ent : mEntities)
	{
		if (ent.ComponentBits.hasBit(cid.getFamily()) && ent.Components[cid.getFamily()] == cid)
			return EntityId(i, ent.Generation);

		++i;
	}

	return EntityId();
}

const detail::BaseComponentPool& EntitySystem::componentGetPool(ComponentId::FamilyType family) const
{
	return *mComponentFamilies.at(family).MemoryPool;
}
const std::vector<EntitySystem::ComponentData>& EntitySystem::componentGetList(ComponentId::FamilyType family) const
{
	return mComponentFamilies.at(family).Components;
}
const std::vector<EntitySystem::EntityData>& EntitySystem::entityGetList() const
{
	return mEntities;
}

EventSystem& EntitySystem::getEventSystem()
{
	if (!mEventSystem)
		mEventSystem = new EventSystem(this);
	return *mEventSystem;
}
const EventSystem& EntitySystem::getEventSystem() const
{
	assert(mEventSystem);
	return *mEventSystem;
}
MessageSystem& EntitySystem::getMessageSystem()
{
	if (!mMessageSystem)
		mMessageSystem = new MessageSystem(this);
	return *mMessageSystem;
}
const MessageSystem& EntitySystem::getMessageSystem() const
{
	assert(mMessageSystem);
	return *mMessageSystem;
}
