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
	for (auto& comp : mComponentData)
		if (comp.MemoryPool)
			delete comp.MemoryPool;
}

ComponentHandle<Component> EntitySystem::getComponent(ComponentId id) const
{
	if (!componentAlive(id))
		return ComponentHandle<Component>();

	auto& data = mComponentData[id.getGeneration()];
	return ComponentHandle<Component>(static_cast<Component*>(data.MemoryPool->getData(id.getIndex())), const_cast<uint32_t*>(&data.ReferenceCounter[id.getIndex()]));
}
Entity EntitySystem::getEntity(EntityId id) const
{
	return Entity(const_cast<EntitySystem*>(this), id);
}

Entity EntitySystem::entityCreate()
{
	EntityId::IndexType id;
	if (mFreeIndices.empty())
	{
		mGenerations.push_back(0);
		id = mGenerations.size() - 1;

		assert(id < EntityId::sMaxIndex);
	}
	else
	{
		id = mFreeIndices.front();
		mFreeIndices.pop_front();
	}

	auto eid = EntityId(id, mGenerations[id]);
	return Entity(this, eid);
}
void EntitySystem::entityDestroy(EntityId id)
{
	if (!entityAlive(id))
		return;

	++mGenerations[id.getIndex()];
	mFreeIndices.push_back(id.getIndex());
}

bool EntitySystem::entityAlive(EntityId id) const
{
	return id.getIndex() < mGenerations.size() && mGenerations[id.getIndex()] == id.getGeneration();
}

void EntitySystem::sendMessage(ComponentId id, Component::BaseMessage* msg)
{
	if (mComponentData.size() <= id.getGeneration())
		return;

	auto& data = mComponentData[id.getGeneration()];
	if (!data.MemoryPool->hasBit(id.getIndex()))
		return;

	static_cast<Component*>(data.MemoryPool->getData(id.getIndex()))->onMessage(msg);
}
void EntitySystem::componentDestroy(ComponentId id)
{
	if (mComponentData.size() <= id.getGeneration())
		return;

	auto& data = mComponentData[id.getGeneration()];
	if (!data.MemoryPool->hasBit(id.getIndex()))
		return;

	data.MemoryPool->destroy(id.getIndex());
	data.MemoryPool->resetBit(id.getIndex());
	data.ReferenceCounter[id.getIndex()] = 0;
	data.FreeIndices.push_back(id.getIndex());
}
bool EntitySystem::componentAlive(ComponentId id) const
{
	if (mComponentData.size() <= id.getGeneration())
		return false;

	return mComponentData[id.getGeneration()].MemoryPool->hasBit(id.getIndex());
}
