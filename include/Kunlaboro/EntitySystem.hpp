#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "ID.hpp"

#include "detail/DynamicBitfield.hpp"

#include <array>
#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <typeinfo>
#include <vector>

namespace Kunlaboro
{
	namespace detail
	{
		class BaseComponentPool;
	}

	class EventSystem;
	class MessageSystem;

	/** An entity system
	 */
	class EntitySystem
	{
	public:
		/** This event is emitted every time a component is attached to an entity.
		 */
		struct ComponentAttachedEvent
		{
			/// The ID of the component that was attached.
			ComponentId Component;
			/// The ID of the entity it was attached to.
			EntityId Entity;
			/// The entity system from where the event originates.
			EntitySystem* ES;
		};
		/** This event is emitted every time a component is detached from an entity.
		 *
		 * \note When this event is emitted, the component is already dangling
		 *       and may even be destroyed.
		 */
		struct ComponentDetachedEvent
		{
			/// The ID of the component that was detached.
			ComponentId Component;
			/// The ID of the entity it was detached from.
			EntityId Entity;
			/// The entity system from where the event originates.
			EntitySystem* ES;
		};
		/** This event is emitted every time a component is created.
		 *
		 * \note Attaching too many listeners to this event might lead
		 *       to a performance hit in component creation.
		 */
		struct ComponentCreatedEvent
		{
			/// The ID of the component that was created.
			ComponentId Component;
			/// The entity system from where the event originates.
			EntitySystem* ES;
		};
		/** This event is emitted every time a component is destroyed.
		 *
		 * \note This event is emitted after the component is destroyed,
		 *       the destructor has already been called and all references
		 *       released.
		 *
		 * \note Attaching too many listeners to this event might lead
		 *       to decreased component performance.
		 */
		struct ComponentDestroyedEvent
		{
			/// The ID of the component that was destroyed.
			ComponentId Component;
			/// The entity system from where the event originates.
			EntitySystem* ES;
		};
		/** This event is emitted every time an entity is created.
		 */
		struct EntityCreatedEvent
		{
			/// The ID of the entity that was created.
			EntityId Entity;
			/// The entity system from where the event originates.
			EntitySystem* ES;
		};
		/** This event is emitted every time an entity is destroyed.
		 */
		struct EntityDestroyedEvent
		{
			/// The ID of the entity that was destroyed.
			EntityId Entity;
			/// The entity system from where the event originates.
			EntitySystem* ES;
		};

		EntitySystem();
		EntitySystem(const EntitySystem&) = delete;
		~EntitySystem();

		EntitySystem& operator=(const EntitySystem&) = delete;

		// TODO:
		// void setEmitEvents(bool emit = true);
		// bool getEmitEvents() const;

		/** Gets a handle to the given component ID.
		 *
		 * \tparam T The type of the component.
		 * \param id The ID of the component.
		 *
		 * \note This method can return an invalid handle if the
		 *       component doesn't exist or has been destroyed.
		 */
		template<typename T>
		ComponentHandle<T> getComponent(ComponentId id) const;
		/** Gets a generic handle to the given component ID.
		 *
		 * \param id The ID of the component.
		 *
		 * \note This method holds no performance benefits to the
		 *       specialized method.
		 * \sa getComponent<T>(ComponentId) const
		 */
		ComponentHandle<Component> getComponent(ComponentId id) const;
		Entity getEntity(EntityId id) const;

		Entity entityCreate();
		void entityDestroy(EntityId id);
		bool entityAlive(EntityId id) const;
		template<typename T>
		ComponentHandle<T> entityGetComponent(EntityId eid) const;
		ComponentHandle<Component> entityGetComponent(ComponentId::FamilyType family, EntityId eid) const;
		bool entityHasComponent(ComponentId::FamilyType family, EntityId eid) const;

		template<typename T, typename... Args>
		ComponentHandle<T> componentCreate(Args...);
		void componentDestroy(ComponentId);
		bool componentAlive(ComponentId) const;

		bool componentAttached(ComponentId cid, EntityId eid) const;
		void componentAttach(ComponentId cid, EntityId eid, bool checkDetach = true);
		void componentDetach(ComponentId cid, EntityId eid);
		
		EntityId componentGetEntity(ComponentId cid) const;

		EventSystem& getEventSystem();
		const EventSystem& getEventSystem() const;
		MessageSystem& getMessageSystem();
		const MessageSystem& getMessageSystem() const;

	public:
		struct ComponentData
		{
			ComponentData()
				: Generation(0)
				, RefCount(new std::atomic_ushort(0))
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
			std::atomic_ushort* RefCount;
		};
		struct EntityData
		{
			EntityId::GenerationType Generation;
			detail::DynamicBitfield ComponentBits;
			std::vector<ComponentId> Components;
		};

		const detail::BaseComponentPool& componentGetPool(ComponentId::FamilyType family) const;
		const std::vector<ComponentData>& componentGetList(ComponentId::FamilyType family) const;
		const std::vector<EntityData>& entityGetList() const;

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

		EventSystem* mEventSystem;
		MessageSystem* mMessageSystem;
	};
}
