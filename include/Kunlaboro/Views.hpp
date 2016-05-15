#pragma once

#include "ID.hpp"
#include "Entity.hpp"

#include "detail/DynamicBitfield.hpp"

#include <functional>

#ifdef _MSC_VER
#define TEMPLATE(T) T
#else
#define TEMPLATE(T) template T
#endif

namespace Kunlaboro
{
	class Component;
	class EntitySystem;

	template<typename ViewType, typename ViewedType>
	class BaseView
	{
	public:
		typedef std::function<bool(const ViewedType&)> Predicate;
		typedef std::function<void(ViewedType&)> Function;

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

			void nextStep();
			virtual bool basePred() const { return true; }
			virtual void moveNext() = 0;
			virtual uint64_t maxLength() const = 0;

			const typename BaseView<ViewType, ViewedType>::Predicate& mPred;
			const EntitySystem* mES;
			uint64_t mIndex;

		};

		ViewType where(const Predicate& pred) const;
		virtual void forEach(const Function& func) const = 0;

	protected:
		BaseView(const EntitySystem*);

		const EntitySystem* mES;
		Predicate mPred;
	};

	template<typename T>
	class ComponentView : public BaseView<ComponentView<T>, T>
	{
	public:
		static_assert(std::is_base_of<Component, T>::value, "Component Views only work on proper components.");

		ComponentView(const EntitySystem& es);

		typedef std::function<bool(const T&)> Predicate;
		typedef std::function<void(T&)> Function;

		struct Iterator : public BaseView<ComponentView, T>::TEMPLATE(BaseIterator<Iterator>)
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

	class EntityView : public BaseView<EntityView, Entity>
	{
	public:
		EntityView(const EntitySystem& es);

		template<typename T> struct ident { typedef T type; };

		enum MatchType
		{
			Match_Current = -1,

			Match_All,
			Match_Any
		};

		typedef std::function<bool(const Entity&)> Predicate;
		typedef std::function<void(Entity&)> Function;

		struct Iterator : public BaseView<EntityView, Entity>::TEMPLATE(BaseIterator<Iterator>)
		{
			inline Entity* operator->() { return &mCurEntity; }
			inline const Entity* operator->() const { return &mCurEntity; }
			inline Entity& operator*() { return mCurEntity; }
			inline const Entity& operator*() const { return mCurEntity; }

		protected:
			Iterator(const EntitySystem* sys, EntityId::IndexType index, const Predicate& pred, const detail::DynamicBitfield&, MatchType);

			friend class EntityView;

			virtual bool basePred() const;
			virtual void moveNext();
			uint64_t maxLength() const;

		private:
			Entity mCurEntity;
			detail::DynamicBitfield mBitField;
			MatchType mMatchType;
		};

		Iterator begin() const;
		Iterator end() const;

		template<typename... Components>
		EntityView withComponents(MatchType match = Match_All) const;

		template<typename... Components>
		void forEachComponents(const typename ident<std::function<void(Entity&, Components*...)>>::type& func, MatchType match = Match_Current);
		template<typename... Components>
		void forEachComponents(const typename ident<std::function<void(Entity&, Components&...)>>::type& func, MatchType match = Match_All);

		virtual void forEach(const Function& func) const;

	private:
		template<typename T, typename T2, typename... Components>
		inline void addComponents();
		template<typename T>
		inline void addComponents();

		static bool matchBitfield(const detail::DynamicBitfield& entity, const detail::DynamicBitfield& bitField, EntityView::MatchType match);

		friend struct Iterator;

		detail::DynamicBitfield mBitField;
		MatchType mMatchType;
	};
}
