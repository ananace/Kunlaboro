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
		mES->componentAttach(comp->getId(), mId);
	}
	template<typename T, typename... Args>
	void Entity::replaceComponent(Args... args)
	{
		auto gen = ComponentFamily<T>::getFamily();
		auto comp = mES->entityGetComponent<T>(gen, mId);
		if (comp)
			mES->componentDetach(comp->getId(), mId);
		comp = mES->componentCreate<T>(std::forward<Args>(args)...);
		mES->componentAttach(comp->getId(), mId);
	}
	template<typename T>
	void Entity::removeComponent()
	{
		auto gen = ComponentFamily<T>::getFamily();

		auto comp = mES->entityGetComponent<T>(gen, mId);
		if (comp)
			mES->componentDetach(comp->getId(), mId);
	}
	template<typename T>
	bool Entity::hasComponent() const
	{
		auto gen = ComponentFamily<T>::getFamily();

		return mES->entityHasComponent(gen, mId);
	}
	template<typename T>
	ComponentHandle<T> Entity::getComponent() const
	{
		auto gen = ComponentFamily<T>::getFamily();
		return mES->entityGetComponent<T>(gen, mId);;
	}
}
