#pragma once

#include "Component.hpp"
#include "EntitySystem.hpp"

namespace Kunlaboro
{
	template<typename T>
	ComponentHandle<T> EntitySystem::getComponent(ComponentId id) const
	{
		return static_cast<ComponentHandle<T>>(getComponent(id));
	}

	template<typename T, typename... Args>
	ComponentHandle<T> EntitySystem::componentCreate(Args... args)
	{
		typedef std::remove_const<T>::type ComponentType;
		auto gen = ComponentHandle<ComponentType>::getGeneration();
		if (mComponentData.size() <= gen)
			mComponentData.resize(gen + 1);
		auto& data = mComponentData[gen];
		if (!data.MemoryPool)
		{
			auto* pool = new detail::ComponentPool<ComponentType>();
			data.MemoryPool = pool;
		}

		auto* pool = static_cast<detail::ComponentPool<ComponentType>*>(data.MemoryPool);
		ComponentId index;
		if (data.FreeIndices.empty())
			index = ComponentId(gen, data.IndexCounter++);
		else
		{
			index = ComponentId(gen, data.FreeIndices.front());
			data.FreeIndices.pop_front();
		}

		pool->ensure(index.getIndex() + 1);
		if (data.ReferenceCounter.size() <= index.getIndex())
			data.ReferenceCounter.resize(index.getIndex() + 1, 0);

		pool->setBit(index.getIndex());
		new(pool->getData(index.getIndex())) T(std::forward<Args>(args)...);
		auto* comp = static_cast<T*>(pool->getData(index.getIndex()));

		comp->mES = this;
		comp->mId = index;

		return ComponentHandle<T>(comp, &data.ReferenceCounter[index.getIndex()]);
	}
}
