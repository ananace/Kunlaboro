#pragma once

#include "EntitySystem.hpp"
#include "Component.hpp"

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
		auto family = ComponentFamily<T>::getFamily();
		if (mComponents.size() <= family)
			mComponents.push_back({});

		auto& data = mComponents[family];
		if (!data.MemoryPool)
		{
			auto* pool = new detail::ComponentPool<T>();
			data.MemoryPool = pool;
		}

		auto* pool = static_cast<detail::ComponentPool<T>*>(data.MemoryPool);
		ComponentId::IndexType index;
		if (data.FreeIndices.empty())
		{
			index = data.ReferenceCounter.size();
			data.ReferenceCounter.push_back(0);
			data.Generations.push_back(0);
		}
		else
		{
			index = data.FreeIndices.front();
			data.FreeIndices.pop_front();
		}

		pool->ensure(index + 1);
		data.ReferenceCounter[index] = 0;

		pool->setBit(index);
		new(pool->getData(index)) T(std::forward<Args>(args)...);
		auto* comp = static_cast<T*>(pool->getData(index));

		comp->mES = this;
		comp->mId = ComponentId(index, data.Generations[index], family);

		return ComponentHandle<T>(comp, &data.ReferenceCounter);
	}

	template<typename T>
	EntitySystem::ComponentView<T> EntitySystem::components() const
	{
		return ComponentView<T>(this);
	}


	/*//////////
	// Views //
	/////////*/

	template<typename IteratorType, typename IteratedType>
	EntitySystem::BaseView::Iterator<IteratorType, IteratedType>::Iterator(const EntitySystem* es, uint64_t index)
		: mES(es)
		, mIndex(index)
	{
	}

	template<typename IteratorType, typename IteratedType>
	IteratorType& EntitySystem::BaseView::Iterator<IteratorType, IteratedType>::operator++()
	{
		++mIndex;
		moveNext();

		return *static_cast<IteratorType*>(this);
	}
	template<typename IteratorType, typename IteratedType>
	bool EntitySystem::BaseView::Iterator<IteratorType, IteratedType>::operator==(const Iterator& rhs) const
	{
		return mIndex == rhs.mIndex;
	}
	template<typename IteratorType, typename IteratedType>
	bool EntitySystem::BaseView::Iterator<IteratorType, IteratedType>::operator!=(const Iterator& rhs) const
	{
		return mIndex != rhs.mIndex;
	}

	template<typename T>
	EntitySystem::ComponentView<T>::Iterator::Iterator(const EntitySystem* sys, ComponentId::IndexType index, const std::vector<ComponentId::GenerationType>* generations)
		: BaseView::Iterator<Iterator, T>(sys, index)
		, mGenerations(generations)
	{
		moveNext();
	}
	template<typename T>
	void EntitySystem::ComponentView<T>::Iterator::moveNext()
	{
		const auto family = ComponentFamily<T>::getFamily();
		
		if (mGenerations->size() > mIndex)
			mCurComponent = mES->getComponent(ComponentId(static_cast<ComponentId::IndexType>(mIndex), mGenerations->at(static_cast<size_t>(mIndex)), family));
		else
			mCurComponent = ComponentHandle<T>();
	}

	template<typename T>
	EntitySystem::ComponentView<T>::ComponentView(const EntitySystem* es)
		: BaseView(es)
	{
	}

	template<typename T>
	typename EntitySystem::ComponentView<T>::Iterator EntitySystem::ComponentView<T>::begin() const
	{
		return Iterator(mES, 0, componentGenerations(ComponentFamily<T>::getFamily()));
	}
	template<typename T>
	typename EntitySystem::ComponentView<T>::Iterator EntitySystem::ComponentView<T>::end() const
	{
		const auto family = ComponentFamily<T>::getFamily();
		return Iterator(mES, componentCount(family), componentGenerations(family));
	}

}