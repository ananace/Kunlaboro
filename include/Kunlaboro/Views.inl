#pragma once

#include "Views.hpp"
#include "Component.hpp"
#include "EntitySystem.hpp"

namespace Kunlaboro
{
	namespace impl
	{
		bool matchBitfield(const detail::DynamicBitfield& entity, const detail::DynamicBitfield& bitField, EntityView::MatchType match);
	}

	template<typename ViewType, typename ViewedType>
	BaseView<ViewType, ViewedType>::BaseView(const EntitySystem* es)
		: mES(es)
	{

	}
	template<typename ViewType, typename ViewedType>
	ViewType& BaseView<ViewType, ViewedType>::where(const Predicate& pred)
	{
		mPred = pred;
		return static_cast<ViewType&>(*this);
	}

	template<typename ViewType, typename ViewedType>
	template<typename IteratorType>
	BaseView<ViewType, ViewedType>::BaseIterator<IteratorType>::BaseIterator(const EntitySystem* es, uint64_t index, const Predicate& pred)
		: mES(es)
		, mIndex(index)
		, mPred(pred)
	{
	}

	template<typename ViewType, typename ViewedType>
	template<typename IteratorType>
	IteratorType& BaseView<ViewType, ViewedType>::BaseIterator<IteratorType>::operator++()
	{
		const auto maxLen = maxLength();
		if (mIndex == maxLen)
			return *static_cast<IteratorType*>(this);

		do
		{
			++mIndex;
			moveNext();
		} while (mIndex < maxLen && (!basePred() || (mPred && !mPred(**this))));

		return *static_cast<IteratorType*>(this);
	}
	template<typename ViewType, typename ViewedType>
	template<typename IteratorType>
	bool BaseView<ViewType, ViewedType>::BaseIterator<IteratorType>::operator==(const BaseIterator& rhs) const
	{
		return mIndex == rhs.mIndex;
	}
	template<typename ViewType, typename ViewedType>
	template<typename IteratorType>
	bool BaseView<ViewType, ViewedType>::BaseIterator<IteratorType>::operator!=(const BaseIterator& rhs) const
	{
		return mIndex != rhs.mIndex;
	}

	template<typename ViewType, typename ViewedType>
	template<typename IteratorType>
	void BaseView<ViewType, ViewedType>::BaseIterator<IteratorType>::nextStep()
	{
		moveNext();

		const auto maxLen = maxLength();
		while (mIndex < maxLen && (!basePred() || (mPred && !mPred(**this))))
		{
			++mIndex;
			moveNext();
		}
	}

	template<typename T>
	ComponentView<T>::Iterator::Iterator(const EntitySystem* sys, ComponentId componentBase, const Predicate& pred)
		: BaseView<ComponentView, T>::template BaseIterator<Iterator>(sys, componentBase.getIndex(), pred)
		, mComponents(&sys->componentGetList(componentBase.getFamily()))
	{
		Iterator::nextStep();
	}
	template<typename T>
	bool ComponentView<T>::Iterator::basePred() const
	{
		return Iterator::mES->componentAlive(mCurComponent->getId());
	}
	template<typename T>
	void ComponentView<T>::Iterator::moveNext()
	{
		const auto family = Kunlaboro::ComponentFamily<T>::getFamily();
		const auto* components = static_cast<const std::vector<EntitySystem::ComponentData>*>(mComponents);

		if (components->size() > Iterator::mIndex)
			mCurComponent = Iterator::mES->getComponent(ComponentId(static_cast<ComponentId::IndexType>(Iterator::mIndex), components->at(static_cast<size_t>(Iterator::mIndex)).Generation, family));
		else
			mCurComponent = ComponentHandle<T>();
	}
	template<typename T>
	uint64_t ComponentView<T>::Iterator::maxLength() const
	{
		return static_cast<const std::vector<EntitySystem::ComponentData>*>(mComponents)->size();
	}

	template<typename T>
	ComponentView<T>::ComponentView(const EntitySystem* es)
		: BaseView<ComponentView, T>(es)
	{
	}

	template<typename T>
	typename ComponentView<T>::Iterator ComponentView<T>::begin() const
	{
		return Iterator(BaseView<ComponentView, T>::mES, ComponentId(0, 0, Kunlaboro::ComponentFamily<T>::getFamily()), BaseView<ComponentView, T>::mPred);
	}
	template<typename T>
	typename ComponentView<T>::Iterator ComponentView<T>::end() const
	{
		auto& list = BaseView<ComponentView, T>::mES->componentGetList(Kunlaboro::ComponentFamily<T>::getFamily());
		return Iterator(BaseView<ComponentView, T>::mES, ComponentId(list.size(), 0, Kunlaboro::ComponentFamily<T>::getFamily()), BaseView<ComponentView, T>::mPred);
	}
	template<typename T>
	void ComponentView<T>::forEach(const Function& func)
	{
		auto family = Kunlaboro::ComponentFamily<T>::getFamily();
		auto& pool = BaseView<ComponentView, T>::mES->componentGetPool(family);

		for (std::size_t i = 0; i < pool.getSize(); ++i)
			if (pool.hasBit(i))
			{
				auto& comp = const_cast<T&>(*static_cast<const T*>(pool.getData(i)));

				if (!BaseView<ComponentView, T>::mPred || BaseView<ComponentView, T>::mPred(comp))
					func(comp);
			}
	}

	template<typename... Components>
	EntityView& EntityView::withComponents(MatchType type)
	{
		mBitField.clear();
		mMatchType = type;
		addComponents<Components...>();

		return *this;
	}
	template<typename T, typename T2, typename... Components>
	inline void EntityView::addComponents()
	{
		mBitField.setBit(Kunlaboro::ComponentFamily<T>::getFamily());
		addComponents<T2, Components...>();
	}
	template<typename T>
	inline void EntityView::addComponents()
	{
		mBitField.setBit(Kunlaboro::ComponentFamily<T>::getFamily());
	}

	template<typename... Components>
	void EntityView::forEach(const typename ident<std::function<void(Entity&, Components*...)>>::type& func, MatchType type)
	{
		mMatchType = type;
		mBitField.clear();
		addComponents<Components...>();

		auto& list = mES->entityGetList();

		for (size_t i = 0; i < list.size(); ++i)
		{
			auto& entData = list[i];
			EntityId eid(i, entData.Generation);

			Entity ent(const_cast<EntitySystem*>(mES), eid);
			if (BaseView<EntityView, Entity>::mES->entityAlive(eid) && (!mPred || mPred(ent)) && impl::matchBitfield(entData.ComponentBits, mBitField, mMatchType))
			{
				func(ent, (ent.getComponent<Components>().get())...);
			}
		}
	}
	template<typename... Components>
	void EntityView::forEach(const typename ident<std::function<void(Entity&, Components&...)>>::type& func, MatchType type)
	{
		assert(type == Match_All);

		mMatchType = Match_All;
		mBitField.clear();
		addComponents<Components...>();

		auto& list = mES->entityGetList();

		for (size_t i = 0; i < list.size(); ++i)
		{
			auto& entData = list[i];
			EntityId eid(i, entData.Generation);

			Entity ent(const_cast<EntitySystem*>(mES), eid);
			if (BaseView<EntityView, Entity>::mES->entityAlive(eid) && (!mPred || mPred(ent)) && impl::matchBitfield(entData.ComponentBits, mBitField, mMatchType))
			{
				func(ent, *(ent.getComponent<Components>().get())...);
			}
		}
	}
}
