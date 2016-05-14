#include <Kunlaboro/EntitySystem.hpp>
#include <Kunlaboro/Entity.hpp>
#include <Kunlaboro/Component.inl>

#include <cassert>

using namespace Kunlaboro;

EntitySystem::EntitySystem()
{

}
EntitySystem::~EntitySystem()
{
	for (auto& comp : mComponents)
		if (comp.MemoryPool)
			delete comp.MemoryPool;
}

ComponentHandle<Component> EntitySystem::getComponent(ComponentId id) const
{
	if (!componentAlive(id))
		return ComponentHandle<Component>();

	auto& data = mComponents[id.getFamily()];
	return ComponentHandle<Component>(static_cast<Component*>(data.MemoryPool->getData(id.getIndex())), const_cast<std::vector<uint32_t>*>(&data.ReferenceCounter));
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
	return Entity(this, eid);
}
void EntitySystem::entityDestroy(EntityId id)
{
	if (!entityAlive(id))
		return;

	++mEntities[id.getIndex()].Generation;
	mFreeEntityIndices.push_back(id.getIndex());
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
	if (entity.Components.size() <= family)
		return ComponentHandle<Component>();

	auto cid = entity.Components[family];
	if (!componentAlive(cid))
		return ComponentHandle<Component>();

	return getComponent(cid);
}

void EntitySystem::componentSendMessage(ComponentId id, Component::BaseMessage* msg)
{
	if (!componentAlive(id))
		return;

	auto& data = mComponents[id.getFamily()];
	static_cast<Component*>(data.MemoryPool->getData(id.getIndex()))->onMessage(msg);
}
void EntitySystem::entitySendMessage(EntityId id, Component::BaseMessage* msg)
{
	if (!entityAlive(id))
		return;

	auto& entity = mEntities[id.getIndex()];
	const auto* components = entity.Components.data();
	for (ComponentId::FamilyType family = 0; family < entity.Components.size(); ++family)
	{
		if (components[family] == ComponentId(family) || !componentAlive(components[family]))
			continue;

		auto& data = mComponents[family];
		static_cast<Component*>(data.MemoryPool->getData(components[family].getIndex()))->onMessage(msg);
	}
}
void EntitySystem::componentDestroy(ComponentId id)
{
	if (!componentAlive(id))
		return;

	auto& data = mComponents[id.getFamily()];
	if (!data.MemoryPool->hasBit(id.getIndex()))
		return;

	data.MemoryPool->destroy(id.getIndex());
	data.MemoryPool->resetBit(id.getIndex());
	data.ReferenceCounter[id.getIndex()] = 0;
	++data.Generations[id.getIndex()];
	data.FreeIndices.push_back(id.getIndex());
}
inline bool EntitySystem::componentAlive(ComponentId id) const
{
	if (mComponents.size() <= id.getFamily())
		return false;

	auto& components = mComponents[id.getFamily()];
	return components.Generations[id.getIndex()] == id.getGeneration() && components.MemoryPool->hasBit(id.getIndex());
}

bool EntitySystem::componentAttached(ComponentId cid, EntityId eid) const
{
	if (!entityAlive(eid) || !componentAlive(cid))
		return false;

	auto& entity = mEntities[eid.getIndex()];
	if (entity.Components.size() <= cid.getFamily())
		return false;

	return entity.Components[cid.getFamily()] == cid.getIndex();
}
void EntitySystem::componentAttach(ComponentId cid, EntityId eid)
{
	if (!entityAlive(eid) || !componentAlive(cid))
		return;

	auto& entity = mEntities[eid.getIndex()];
	if (entity.Components.size() <= cid.getFamily())
		entity.Components.resize(cid.getFamily() + 1, ComponentId(cid.getFamily()));

	auto comp = getComponent(cid);
	if (comp->getEntityId() == eid)
		return;

	if (comp->getEntityId() != EntityId())
		componentDetach(cid, comp->getEntityId());

	if (entity.Components[cid.getFamily()] != ComponentId(cid.getFamily()))
		componentDetach(entity.Components[cid.getFamily()], eid);

	entity.Components[cid.getFamily()] = cid.getIndex();
	comp->mOwnerId = eid;
	comp.unlink();
}
void EntitySystem::componentDetach(ComponentId cid, EntityId eid)
{
	if (!entityAlive(eid) || !componentAlive(cid))
		return;

	auto& entity = mEntities[eid.getIndex()];
	if (entity.Components.size() <= cid.getFamily())
		return;

	auto comp = getComponent(cid);
	if (comp->getEntityId() == eid)
		return;

	comp->mOwnerId = EntityId();
	comp.release();
}

EntitySystem::BaseView::BaseView(const EntitySystem* es)
	: mES(es)
{

}
const std::vector<ComponentId::GenerationType>* EntitySystem::BaseView::componentGenerations(ComponentId::FamilyType family) const
{
	if (mES->mComponents.size() <= family)
		return nullptr;

	return &mES->mComponents[family].Generations;
}
size_t EntitySystem::BaseView::componentCount(ComponentId::FamilyType family) const
{
	if (mES->mComponents.size() <= family)
		return 0;

	return mES->mComponents[family].ReferenceCounter.size();
}
size_t EntitySystem::BaseView::entityCount() const
{
	return mES->mEntities.size();
}
