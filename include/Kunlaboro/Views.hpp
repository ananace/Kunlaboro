#pragma once

#include "ID.hpp"
#include "Entity.hpp"

#include <functional>

namespace Kunlaboro
{
	class EntitySystem;

	template<typename ViewType, typename ViewedType>
	class BaseView
	{
	public:
		typedef std::function<bool(const ViewedType&)> Predicate;
		typedef std::function<void(ViewedType&)> Function;

		BaseView(const EntitySystem*);

		template<typename IteratorType>
		class BaseIterator : public std::iterator<std::input_iterator_tag, ComponentId>
		{
		public:
			virtual ~BaseIterator() = default;

			IteratorType& operator++();
			bool operator==(const BaseIterator& rhs) const;
			bool operator!=(const BaseIterator& rhs) const;

			virtual ViewedType* operator->() = 0;
			virtual const ViewedType* operator->() const = 0;
			virtual ViewedType& operator*() = 0;
			virtual const ViewedType& operator*() const = 0;

		protected:
			BaseIterator(const EntitySystem* es, uint64_t index, const typename BaseView<ViewType, ViewedType>::Predicate& pred);

			virtual bool basePred() const { return true; }
			virtual void moveNext() = 0;
			virtual uint64_t maxLength() const = 0;

			const typename BaseView<ViewType, ViewedType>::Predicate& mPred;
			const EntitySystem* mES;
			uint64_t mIndex;
		};

		ViewType& where(const Predicate& pred);
		virtual void forEach(const Function& func) = 0;

	protected:
		const EntitySystem* mES;
		Predicate mPred;
	};

	template<typename T>
	class ComponentView : public BaseView<ComponentView<T>, T>
	{
	public:
		struct Iterator : public BaseView<ComponentView, T>::template BaseIterator<Iterator>
		{
			inline T* operator->() { return mCurComponent.get(); }
			inline const T* operator->() const { return mCurComponent.get(); }
			inline T& operator*() { return *mCurComponent; }
			inline const T& operator*() const { return *mCurComponent; }

		protected:
			Iterator(const EntitySystem* sys, ComponentId componentBase, const typename BaseView<ComponentView<T>, T>::Predicate& pred);

			friend class ComponentView;

			virtual bool basePred() const;
			virtual void moveNext();
			uint64_t maxLength() const;

		private:
			ComponentHandle<T> mCurComponent;
			const void* mComponents;
		};

		Iterator begin() const;
		Iterator end() const;

		virtual void forEach(const typename BaseView<ComponentView<T>, T>::Function& func);

	private:
		ComponentView(const EntitySystem* es);

		friend class EntitySystem;
	};

	class EntityView : public BaseView<EntityView, Entity>
	{
	public:
		struct Iterator : public BaseView<EntityView, Entity>::template BaseIterator<Iterator>
		{
			inline Entity* operator->() { return &mCurEntity; }
			inline const Entity* operator->() const { return &mCurEntity; }
			inline Entity& operator*() { return mCurEntity; }
			inline const Entity& operator*() const { return mCurEntity; }

		protected:
			Iterator(EntitySystem* sys, EntityId::IndexType index);

			virtual bool basePred() const;
			virtual void moveNext();
			uint64_t maxLength() const;

		private:
			Entity mCurEntity;
		};

		template<typename... Components>
		EntityView& withComponents();
		template<typename... Components>
		void forEach(const std::function<void(Entity&, Components&...)>& func);
		virtual void forEach(const Function& func);

	private:
		EntityView(const EntitySystem* es);

		friend class EntitySystem;
	};
}
