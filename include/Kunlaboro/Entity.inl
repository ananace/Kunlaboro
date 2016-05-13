#pragma once

#include "Entity.hpp"
#include "Component.hpp"
#include "EntitySystem.hpp"

namespace Kunlaboro
{
	template<typename T, typename... Args>
	void Entity::addComponent(Args... args)
	{
		auto comp = mES->componentCreate<T>(std::forward<Args>(args)...);
		mES->componentAttach(comp->GetId(), mId);
	}
	template<typename T>
	void Entity::removeComponent()
	{
		typedef typename std::remove_const<T>::type ComponentType;
		auto gen = ComponentHandle<ComponentType>::getGeneration();
/*
		ComponentId id = ComponentId::INVALID;
		for (auto& c : mComponents)
			if (c.GetGeneration() == gen)
			{
				id = c;
				break;
			}

		if (mES->IsAlive(id))
			mES->DetachComponent(id);
			*/
	}
	template<typename T>
	bool Entity::hasComponent() const
	{
		typedef typename std::remove_const<T>::type ComponentType;
		auto gen = ComponentHandle<ComponentType>::getGeneration();

	}
	template<typename T>
	ComponentHandle<T> Entity::getComponent() const
	{
		typedef typename std::remove_const<T>::type ComponentType;
		auto gen = ComponentHandle<ComponentType>::getGeneration();
/*
		for (auto& c : mComponents)
			if (c.GetGeneration() == gen)
				return mES->GetComponent<T>(c);
				*/
	}
}
