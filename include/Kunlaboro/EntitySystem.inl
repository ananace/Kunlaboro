#pragma once

#include "EntitySystem.hpp"
#include "Component.hpp"

#include "detail/ComponentPool.hpp"

namespace Kunlaboro
{

	template<typename T>
	ComponentHandle<T> EntitySystem::getComponent(ComponentId id) const
	{
		return static_cast<ComponentHandle<T>>(getComponent(id));
	}

	template<typename T>
	ComponentHandle<T> EntitySystem::entityGetComponent(ComponentId::FamilyType family, EntityId eid) const
	{
		return static_cast<ComponentHandle<T>>(entityGetComponent(family, eid));
	}

	template<typename T, typename... Args>
	ComponentHandle<T> EntitySystem::componentCreate(Args... args)
	{
		auto family = Kunlaboro::ComponentFamily<T>::getFamily();
		if (mComponentFamilies.size() <= family)
			mComponentFamilies.resize(family + 1);

		auto& data = mComponentFamilies[family];
		if (!data.MemoryPool)
		{
			auto* pool = new detail::ComponentPool<T>();
			data.MemoryPool = pool;
		}

		auto* pool = static_cast<detail::ComponentPool<T>*>(data.MemoryPool);
		ComponentId::IndexType index;
		if (data.FreeIndices.empty())
		{
			index = data.Components.size();
			data.Components.push_back({ });
		}
		else
		{
			index = data.FreeIndices.front();
			data.FreeIndices.pop_front();
		}

		pool->ensure(index + 1);
		auto& component = data.Components[index];
		component.RefCount->store(0);

		pool->setBit(index);
		auto* comp = static_cast<T*>(pool->getData(index));

		auto id = ComponentId(index, component.Generation, family);

		comp->mES = this;
		comp->mId = id;

		new(pool->getData(index)) T(std::forward<Args>(args)...);

		// TODO: Move more of this out of header
		// if (mEventSystem)
		//     mEventSystem->eventEmit<ComponentCreatedEvent>(comp->mId, this);

		return ComponentHandle<T>(comp, component.RefCount);
	}

}
