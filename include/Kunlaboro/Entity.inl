#pragma once

#include "Entity.hpp"
#include "Component.hpp"
#include "EntitySystem.hpp"

namespace Kunlaboro
{
	template<typename T, typename... Args>
	void Entity::addComponent(Args... args)
	{
		auto comp = mES->createComponent<T>(std::forward<Args>(args)...);
		mES->attachComponent(comp->getId(), mId);
	}
	template<typename T, typename... Args>
	void Entity::replaceComponent(Args... args)
	{
		auto gen = ComponentFamily<T>::getFamily();

		auto comp = mES->getComponent<T>(gen, mId);
		if (comp)
			mES->detachComponent(comp->getId(), mId);

		comp = mES->createComponent<T>(std::forward<Args>(args)...);
		assert(comp);

		mES->attachComponent(comp->getId(), mId);
	}
	template<typename T>
	void Entity::removeComponent()
	{
		auto gen = ComponentFamily<T>::getFamily();

		auto comp = mES->getComponent<T>(gen, mId);
		if (comp)
			mES->detachComponent(comp->getId(), mId);
	}
	template<typename T>
	bool Entity::hasComponent() const
	{
		auto gen = ComponentFamily<T>::getFamily();

		return mES->hasComponent(gen, mId);
	}
	template<typename T>
	ComponentHandle<T> Entity::getComponent() const
	{
		return mES->getComponent<T>(mId);;
	}
}
