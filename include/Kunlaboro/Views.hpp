#pragma once

#include "ID.hpp"
#include "Entity.hpp"

#include "detail/DynamicBitfield.hpp"

#include <functional>
#include <type_traits>

namespace Kunlaboro
{
	class Component;
	class EntitySystem;

	enum MatchType
	{
		Match_All,
		Match_Any
	};

	namespace impl
	{
		template<typename ViewType, typename ViewedType>
		class BaseView
		{
		public:
			typedef std::function<bool(const ViewedType&)> Predicate;
			typedef std::function<void(ViewedType&)> Function;

			ViewType where(const Predicate& pred) const;
			virtual void forEach(const Function& func) const = 0;

			const EntitySystem& getEntitySystem() const;
			const Predicate& getPredicate() const;
			void setPredicate(const Predicate& pred);

		protected:
			BaseView(const EntitySystem*);

			const EntitySystem* mES;
			Predicate mPred;
		};

		template<typename IteratorType, typename ViewedType>
		class BaseIterator : public std::iterator<std::input_iterator_tag, ViewedType>
		{
		public:
			typedef std::function<bool(const ViewedType&)> Predicate;
			virtual ~BaseIterator() = default;

			IteratorType& operator++();
			bool operator==(const BaseIterator& rhs) const;
			bool operator!=(const BaseIterator& rhs) const;

			virtual ViewedType* operator->() = 0;
			virtual const ViewedType* operator->() const = 0;
			virtual ViewedType& operator*() = 0;
			virtual const ViewedType& operator*() const = 0;

		protected:
			BaseIterator(const EntitySystem* es, uint64_t index, const Predicate& pred);

			void nextStep();
			virtual bool basePred() const { return true; }
			virtual void moveNext() = 0;
			virtual uint64_t maxLength() const = 0;

			const Predicate& mPred;
			const EntitySystem* mES;
			uint64_t mIndex;

		};

		bool matchBitfield(const detail::DynamicBitfield& entity, const detail::DynamicBitfield& bitField, MatchType match);
	}

	template<typename T>
	class ComponentView : public impl::BaseView<ComponentView<T>, T>
	{
	public:
		static_assert(std::is_base_of<Component, T>::value, "Component Views only work on proper components.");

		ComponentView(const EntitySystem& es);

		typedef std::function<bool(const T&)> Predicate;
		typedef std::function<void(T&)> Function;

		struct Iterator : public impl::BaseIterator<Iterator, T>
		{
			inline T* operator->() { return mCurComponent.get(); }
			inline const T* operator->() const { return mCurComponent.get(); }
			inline T& operator*() { return *mCurComponent; }
			inline const T& operator*() const { return *mCurComponent; }

		protected:
			Iterator(const EntitySystem* sys, ComponentId componentBase, const Predicate& pred);

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

		virtual void forEach(const Function& func) const;
	};

	template<MatchType MT, typename... Components>
	class TypedEntityView;

	class EntityView : public impl::BaseView<EntityView, Entity>
	{
	public:
		EntityView(const EntitySystem& es);

		typedef std::function<bool(const Entity&)> Predicate;
		typedef std::function<void(Entity&)> Function;

		struct Iterator : public impl::BaseIterator<Iterator, Entity>
		{
			inline Entity* operator->() { return &mCurEntity; }
			inline const Entity* operator->() const { return &mCurEntity; }
			inline Entity& operator*() { return mCurEntity; }
			inline const Entity& operator*() const { return mCurEntity; }

		protected:
			Iterator(const EntitySystem* sys, EntityId::IndexType index, const Predicate& pred);

			friend class EntityView;

			virtual bool basePred() const;
			virtual void moveNext();
			uint64_t maxLength() const;

		private:
			Entity mCurEntity;
		};

		Iterator begin() const;
		Iterator end() const;

		template<MatchType match = Match_All, typename... Components>
		TypedEntityView<match, Components...> withComponents() const;

		virtual void forEach(const Function& func) const;
	};

	template<MatchType MT, typename... Components>
	class TypedEntityView : public impl::BaseView<TypedEntityView<MT, Components...>, Entity>
	{
	public:
		TypedEntityView(const EntitySystem& es);

		typedef std::function<bool(const Entity&)> Predicate;
		typedef std::function<void(Entity&)> Function;

		template<typename T> struct ident { typedef T type; };

		typedef EntityView::Iterator Iterator;

		Iterator begin() const;
		Iterator end() const;

		void forEach(const typename ident<std::function<void(Entity&, Components*...)>>::type& func) const;
		void forEach(const typename ident<std::function<void(Entity&, Components&...)>>::type& func) const;

		virtual void forEach(const Function& func) const;

	private:
		template<typename T, typename T2, typename... ComponentsToAdd>
		inline void addComponents();
		template<typename T>
		inline void addComponents();

		detail::DynamicBitfield mBitField;
	};
}
