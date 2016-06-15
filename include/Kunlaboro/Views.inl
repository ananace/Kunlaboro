#pragma once

#include "Views.hpp"
#include "Component.hpp"
#include "EntitySystem.hpp"

#include "detail/JobQueue.hpp"

namespace Kunlaboro
{

	template<typename ViewType, typename ViewedType>
	impl::BaseView<ViewType, ViewedType>::BaseView(const EntitySystem* es)
		: mES(es)
		, mQueue(nullptr)
		, mParallelOwned(false)
	{

	}

	template<typename ViewType, typename ViewedType>
	impl::BaseView<ViewType, ViewedType>::~BaseView()
	{
		if (mParallelOwned)
			delete mQueue;
	}

	template<typename ViewType, typename ViewedType>
	ViewType impl::BaseView<ViewType, ViewedType>::parallel(bool parallel) const
	{
		ViewType ret(static_cast<const ViewType&>(*this));
		ret.mParallelOwned = parallel;
		if (parallel)
			ret.mQueue = new detail::JobQueue();
		else
			ret.mQueue = nullptr;
		return ret;
	}

	template<typename ViewType, typename ViewedType>
	ViewType impl::BaseView<ViewType, ViewedType>::parallel(detail::JobQueue& queue) const
	{
		ViewType ret(static_cast<const ViewType&>(*this));
		ret.mParallelOwned = false;
		ret.mQueue = &queue;
		return ret;
	}

	template<typename ViewType, typename ViewedType>
	ViewType impl::BaseView<ViewType, ViewedType>::where(const Predicate& pred) const
	{
		ViewType ret(static_cast<const ViewType&>(*this));
		ret.mPred = pred;
		return ret;
	}

	template<typename ViewType, typename ViewedType>
	const EntitySystem& impl::BaseView<ViewType, ViewedType>::getEntitySystem() const
	{
		return *mES;
	}
	template<typename ViewType, typename ViewedType>
	const typename impl::BaseView<ViewType, ViewedType>::Predicate& impl::BaseView<ViewType, ViewedType>::getPredicate() const
	{
		return mPred;
	}
	template<typename ViewType, typename ViewedType>
	void impl::BaseView<ViewType, ViewedType>::setPredicate(const Predicate& pred)
	{
		mPred = pred;
	}

	template<typename IteratorType, typename ViewedType>
	impl::BaseIterator<IteratorType, ViewedType>::BaseIterator(const EntitySystem* es, uint64_t index, const Predicate& pred)
		: mES(es)
		, mIndex(index)
		, mPred(pred)
	{
	}

	template<typename IteratorType, typename ViewedType>
	IteratorType& impl::BaseIterator<IteratorType, ViewedType>::operator++()
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
	template<typename IteratorType, typename ViewedType>
	bool impl::BaseIterator<IteratorType, ViewedType>::operator==(const BaseIterator& rhs) const
	{
		return mIndex == rhs.mIndex;
	}
	template<typename IteratorType, typename ViewedType>
	bool impl::BaseIterator<IteratorType, ViewedType>::operator!=(const BaseIterator& rhs) const
	{
		return mIndex != rhs.mIndex;
	}

	template<typename IteratorType, typename ViewedType>
	void impl::BaseIterator<IteratorType, ViewedType>::nextStep()
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
		: impl::BaseIterator<Iterator, T>(sys, componentBase.getIndex(), pred)
		, mComponents(&sys->componentGetList(componentBase.getFamily()))
	{
		Iterator::nextStep();
	}
	template<typename T>
	bool ComponentView<T>::Iterator::basePred() const
	{
		return Iterator::mES->isAlive(mCurComponent->getId());
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
	ComponentView<T>::ComponentView(const EntitySystem& es)
		: impl::BaseView<ComponentView, T>(&es)
	{
	}

	template<typename T>
	typename ComponentView<T>::Iterator ComponentView<T>::begin()
	{
		return Iterator(impl::BaseView<ComponentView, T>::mES, ComponentId(0, 0, Kunlaboro::ComponentFamily<T>::getFamily()), impl::BaseView<ComponentView, T>::mPred);
	}
	template<typename T>
	typename ComponentView<T>::Iterator ComponentView<T>::end()
	{
		auto& list = impl::BaseView<ComponentView, T>::mES->componentGetList(Kunlaboro::ComponentFamily<T>::getFamily());
		return Iterator(impl::BaseView<ComponentView, T>::mES, ComponentId(list.size(), 0, Kunlaboro::ComponentFamily<T>::getFamily()), impl::BaseView<ComponentView, T>::mPred);
	}
	template<typename T>
	void ComponentView<T>::forEach(const Function& func)
	{
		auto family = Kunlaboro::ComponentFamily<T>::getFamily();
		auto& pool = impl::BaseView<ComponentView, T>::mES->componentGetPool(family);
		auto* queue = impl::BaseView<ComponentView, T>::mQueue;

		for (std::size_t i = 0; i < pool.getSize(); ++i)
			if (pool.hasBit(i))
			{
				auto& comp = const_cast<T&>(*static_cast<const T*>(pool.getData(i)));

				if (!impl::BaseView<ComponentView, T>::mPred || impl::BaseView<ComponentView, T>::mPred(comp))
				{
					if (queue)
						queue->submit(Function(func), std::move(comp));
					else
						func(comp);
				}
			}

		if (queue)
			queue->wait();
	}

	template<MatchType mt, typename... Components>
	TypedEntityView<mt,Components...> EntityView::withComponents() const
	{
		TypedEntityView<mt,Components...> ret(*mES);
		ret.setPredicate(mPred);
		return ret;
	}
	template<MatchType MT, typename... ViewComponents>
	template<typename T, typename T2, typename... Components>
	inline void TypedEntityView<MT, ViewComponents...>::addComponents()
	{
		mBitField.setBit(Kunlaboro::ComponentFamily<T>::getFamily());
		addComponents<T2, Components...>();
	}
	template<MatchType MT, typename... ViewComponents>
	template<typename T>
	inline void TypedEntityView<MT, ViewComponents...>::addComponents()
	{
		mBitField.setBit(Kunlaboro::ComponentFamily<T>::getFamily());
	}

	template<MatchType MT, typename... Components>
	TypedEntityView<MT, Components...>::TypedEntityView(const EntitySystem& es)
		: impl::BaseView<TypedEntityView<MT, Components...>, Entity>(&es)
	{
		addComponents<Components...>();
	}

	template<MatchType MT, typename... Components>
	void TypedEntityView<MT, Components...>::forEach(const Function& func)
	{
		const auto* es = impl::BaseView<TypedEntityView<MT,Components...>, Entity>::mES;
		const auto& pred = impl::BaseView<TypedEntityView<MT,Components...>, Entity>::mPred;
		auto* queue = impl::BaseView<TypedEntityView<MT,Components...>, Entity>::mQueue;

		auto& list = es->entityGetList();

		for (size_t i = 0; i < list.size(); ++i)
		{
			auto& entData = list[i];
			EntityId eid(i, entData.Generation);

			Entity ent = es->getEntity(eid);
			if (es->isAlive(eid) && (!pred || pred(ent)))
			{
				if (queue)
					queue->submit(Function(func), std::move(ent));
				else
					func(ent);
			}
		}

		if (queue)
			queue->wait();
	}

	template<MatchType MT, typename... Components>
	void TypedEntityView<MT, Components...>::forEach(const typename ident<std::function<void(Entity&, Components*...)>>::type& func)
	{
		typedef typename ident<std::function<void(Entity&, Components*...)>>::type TypedFunction;

		const auto* es = impl::BaseView<TypedEntityView<MT,Components...>, Entity>::mES;
		const auto& pred = impl::BaseView<TypedEntityView<MT,Components...>, Entity>::mPred;
		auto* queue = impl::BaseView<TypedEntityView<MT,Components...>, Entity>::mQueue;

		auto& list = es->entityGetList();

		for (size_t i = 0; i < list.size(); ++i)
		{
			auto& entData = list[i];
			EntityId eid(i, entData.Generation);

			Entity ent = es->getEntity(eid);
			if (ent && (!pred || pred(ent)) && impl::matchBitfield(entData.ComponentBits, mBitField, MT))
			{
				if (queue)
					queue->submit(TypedFunction(func), std::move(ent), std::move(ent.getComponent<Components>().get())...);
				else
					func(ent, (ent.getComponent<Components>().get())...);
			}
		}

		if (queue)
			queue->wait();
	}

	template<MatchType MT, typename... Components>
	void TypedEntityView<MT, Components...>::forEach(const typename ident<std::function<void(Entity&, Components&...)>>::type& func)
	{
		static_assert(MT == Match_All, "Can't use references unless matching all components.");

		typedef typename ident<std::function<void(Entity&, Components&...)>>::type TypedFunction;

		const auto* es = impl::BaseView<TypedEntityView<MT,Components...>, Entity>::mES;
		const auto& pred = impl::BaseView<TypedEntityView<MT,Components...>, Entity>::mPred;
		auto* queue = impl::BaseView<TypedEntityView<MT,Components...>, Entity>::mQueue;

		auto& list = es->entityGetList();

		for (size_t i = 0; i < list.size(); ++i)
		{
			auto& entData = list[i];
			EntityId eid(i, entData.Generation);

			Entity ent = es->getEntity(eid);
			if (es->isAlive(eid) && (!pred || pred(ent)) && impl::matchBitfield(entData.ComponentBits, mBitField, MT))
			{
				if (queue)
					queue->submit(TypedFunction(func), std::move(ent), std::move(*(ent.getComponent<Components>().get()))...);
				else
					func(ent, *(ent.getComponent<Components>().get())...);
			}
		}

		if (queue)
			queue->wait();
	}
}
