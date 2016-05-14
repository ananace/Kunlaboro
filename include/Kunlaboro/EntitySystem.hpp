#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "ID.hpp"

#include "detail/ComponentPool.hpp"

#include <array>
#include <atomic>
#include <list>
#include <memory>
#include <typeinfo>
#include <vector>

namespace Kunlaboro
{

	class EntitySystem
	{
	public:
		EntitySystem();
		EntitySystem(const EntitySystem&) = delete;
		~EntitySystem();

		EntitySystem& operator=(const EntitySystem&) = delete;

		template<typename T>
		ComponentHandle<T> getComponent(ComponentId id) const;
		ComponentHandle<Component> getComponent(ComponentId id) const;
		Entity getEntity(EntityId id) const;

		Entity entityCreate();
		void entityDestroy(EntityId id);
		bool entityAlive(EntityId id) const;
		template<typename T>
		ComponentHandle<T> entityGetComponent(ComponentId::FamilyType family, EntityId eid) const;
		ComponentHandle<Component> entityGetComponent(ComponentId::FamilyType family, EntityId eid) const;

		void componentSendMessage(ComponentId id, Component::BaseMessage* msg);
		void entitySendMessage(EntityId id, Component::BaseMessage* msg);

		template<typename T, typename... Args>
		ComponentHandle<T> componentCreate(Args...);
		void componentDestroy(ComponentId);
		bool componentAlive(ComponentId) const;

		bool componentAttached(ComponentId cid, EntityId eid) const;
		void componentAttach(ComponentId cid, EntityId eid);
		void componentDetach(ComponentId cid, EntityId eid);
		
	private:
		class BaseView
		{
		public:
			BaseView(const EntitySystem*);

			template<typename IteratorType, typename IteratedType>
			class Iterator : public std::iterator<std::input_iterator_tag, ComponentId>
			{
			public:
				virtual ~Iterator() = default;

				IteratorType& operator++();
				bool operator==(const Iterator& rhs) const;
				bool operator!=(const Iterator& rhs) const;

				virtual IteratedType* operator->() = 0;
				virtual const IteratedType* operator->() const = 0;
				virtual IteratedType& operator*() = 0;
				virtual const IteratedType& operator*() const = 0;

			protected:
				Iterator(const EntitySystem* es, uint64_t index);

				virtual void moveNext() = 0;

				const EntitySystem* mES;
				uint64_t mIndex;
			};

		protected:
			const EntitySystem* mES;
		};

	public:
		struct ComponentData
		{
			ComponentData()
				: Generation(0)
				, RefCount(new std::atomic_uint32_t(0))
			{ }
			ComponentData(const ComponentData&) = delete;
			ComponentData(ComponentData&& move)
				: Generation(std::move(move.Generation))
				, RefCount(std::move(move.RefCount))
			{
				move.RefCount = nullptr;
			}
			~ComponentData()
			{
				if (RefCount)
					delete RefCount;
			}

			ComponentId::GenerationType Generation;
			std::atomic_uint32_t* RefCount;
		};
		struct EntityData
		{
			EntityId::GenerationType Generation;
			std::vector<ComponentId> Components;
		};

		const std::vector<ComponentData>& componentGetList(ComponentId::FamilyType family) const;
		const std::vector<EntityData>& entityGetList() const;


		template<typename T>
		class ComponentView : public BaseView
		{
		public:
			struct Iterator : public BaseView::Iterator<Iterator, T>
			{
				inline T* operator->() { return mCurComponent.get(); }
				inline const T* operator->() const { return mCurComponent.get(); }
				inline T& operator*() { return *mCurComponent; }
				inline const T& operator*() const { return *mCurComponent; }

			protected:
				Iterator(const EntitySystem* sys, ComponentId::IndexType index, const std::vector<EntitySystem::ComponentData>* components);

				friend class ComponentView;

				virtual void moveNext();

			private:
				ComponentHandle<T> mCurComponent;
				const std::vector<EntitySystem::ComponentData>* mComponents;
			};

			Iterator begin() const;
			Iterator end() const;

		private:
			ComponentView(const EntitySystem* es);

			friend class EntitySystem;
		};

		class EntityView : public BaseView
		{
		public:
			struct Iterator : public BaseView::Iterator<Iterator, Entity>
			{
				inline Entity* operator->() { return &mCurEntity; }
				inline const Entity* operator->() const { return &mCurEntity; }
				inline Entity& operator*() { return mCurEntity; }
				inline const Entity& operator*() const { return mCurEntity; }

			protected:
				Iterator(EntitySystem* sys, EntityId::IndexType index);
				virtual void moveNext();

			private:
				Entity mCurEntity;
			};
		};

		template<typename T>
		ComponentView<T> components() const;
		//EntityView entities() const;

	private:
		struct ComponentFamily
		{
			ComponentFamily()
				: MemoryPool(nullptr)
			{ }

			std::list<ComponentId::IndexType> FreeIndices;
			std::vector<ComponentData> Components;
			detail::BaseComponentPool* MemoryPool;
		};


		std::list<EntityId::IndexType> mFreeEntityIndices;

		std::vector<ComponentFamily> mComponentFamilies;
		std::vector<EntityData> mEntities;
	};
}
