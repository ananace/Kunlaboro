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
		auto family = Kunlaboro::ComponentFamily<T>::getFamily();
		if (mComponentFamilies.size() <= family)
			mComponentFamilies.push_back({});

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
		new(pool->getData(index)) T(std::forward<Args>(args)...);
		auto* comp = static_cast<T*>(pool->getData(index));

		comp->mES = this;
		comp->mId = ComponentId(index, component.Generation, family);

		return ComponentHandle<T>(comp, component.RefCount);
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
	EntitySystem::ComponentView<T>::Iterator::Iterator(const EntitySystem* sys, ComponentId::IndexType index, const std::vector<EntitySystem::ComponentData>* components)
		: BaseView::Iterator<Iterator, T>(sys, index)
		, mComponents(components)
	{
		moveNext();
	}
	template<typename T>
	void EntitySystem::ComponentView<T>::Iterator::moveNext()
	{
		const auto family = Kunlaboro::ComponentFamily<T>::getFamily();
		
		if (mComponents->size() > mIndex)
			mCurComponent = mES->getComponent(ComponentId(static_cast<ComponentId::IndexType>(mIndex), mComponents->at(static_cast<size_t>(mIndex)).Generation, family));
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
		return Iterator(mES, 0, &mES->componentGetList(Kunlaboro::ComponentFamily<T>::getFamily()));
	}
	template<typename T>
	typename EntitySystem::ComponentView<T>::Iterator EntitySystem::ComponentView<T>::end() const
	{
		auto& list = mES->componentGetList(Kunlaboro::ComponentFamily<T>::getFamily());
		return Iterator(mES, list.size(), &list);
	}

}