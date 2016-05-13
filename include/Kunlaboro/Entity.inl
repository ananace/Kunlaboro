#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "EntitySystem.hpp"

namespace Kunlaboro
{
	template<typename T, typename... Args>
	void Entity::addComponent(Args... args)
	{
		auto comp = mES->CreateComponent<T>(std::forward<Args>(args)...);
		mES->AttachComponent(comp->GetId(), mId);
	}
	template<typename T>
	void Entity::removeComponent()
	{
		typedef std::remove_const<T>::type ComponentType;
		auto gen = ComponentHandle<ComponentType>::getGeneration();

		ComponentId id = ComponentId::INVALID;
		for (auto& c : mComponents)
			if (c.GetGeneration() == gen)
			{
				id = c;
				break;
			}

		if (mES->IsAlive(id))
			mES->DetachComponent(id);
	}
	template<typename T>
	bool Entity::hasComponent() const
	{
		typedef std::remove_const<T>::type ComponentType;
		auto gen = ComponentHandle<ComponentType>::getGeneration();

	}
	template<typename T>
	ComponentHandle<T> Entity::getComponent() const
	{
		typedef std::remove_const<T>::type ComponentType;
		auto gen = ComponentHandle<ComponentType>::getGeneration();

		for (auto& c : mComponents)
			if (c.GetGeneration() == gen)
				return mES->GetComponent<T>(c);
	}
}