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
		/** Gets a reference to the entity with the given ID.
		 *
		 * \param id The ID of the entity.
		 */
		Entity getEntity(EntityId id) const;

		/** Creates a new entity.
		 *
		 * \returns A reference to the newly created entity.
		 */
		Entity entityCreate();
		/** Destroys an entity with the given ID.
		 *
		 * \param id The ID of the entity to destroy.
		 * \note This method will not do anything when given invalid IDs.
		 */
		void entityDestroy(EntityId id);
		/** Checks if the given entity ID is alive.
		 *
		 * This method will validate that an entity with the given ID
		 * exists and hasn't been destroyed.
		 */
		bool entityAlive(EntityId id) const;
		/** Gets a handle to a component of the given type in an entity.
		 *
		 * \tparam T The type of the component to find.
		 * \param eid The ID of the entity to look in.
		 *
		 * \sa Entity::getComponent()
		 */
		template<typename T>
		ComponentHandle<T> entityGetComponent(EntityId eid) const;
		/** Gets a generic handle to a component of the given type in an entity.
		*
		* \param family The family ID of the component to look for.
		* \param eid The ID of the entity to look in.
		*
		* \sa ComponentFamily::getFamily()
		* \sa Entity::getComponent()
		*/
		ComponentHandle<Component> entityGetComponent(ComponentId::FamilyType family, EntityId eid) const;
		/** Checks if an entity contains a component of the given type.
		 *
		 * \param family The component family ID to look for.
		 * \param eid The ID of the entity to look in.
		 *
		 * \sa ComponentFamily::getFamily()
		 * \sa Entity::hasComponent()
		 */
		bool entityHasComponent(ComponentId::FamilyType family, EntityId eid) const;

		/** Creates a component and returns a handle to it.
		 *
		 * \tparam T The type of component to create.
		 * \param args The arguments to pass to the constructor of the component.
		 */
		template<typename T, typename... Args>
		ComponentHandle<T> componentCreate(Args... args);
		/** Destroys the component with the given ID.
		 *
		 * \param cid The ID of the component to destroy.
		 * \note When given an invalid ID, this method does nothing.
		 */
		void componentDestroy(ComponentId cid);
		/** Checks if the given component ID is alive.
		 *
		 * \param cid The ID of the component to check.
		 */
		bool componentAlive(ComponentId cid) const;

		/** Checks if the given component ID is attached to the given entity ID.
		 *
		 * \param cid The ID of the component to check.
		 * \param eid The ID of the entity to look in.
		 */
		bool componentAttached(ComponentId cid, EntityId eid) const;
		/** Attaches the component with the given ID to the given entity ID.
		 *
		 * \param cid The ID of the component to attach.
		 * \param eid The ID of the entity to attach it to.
		 * \param checkDetach
		 * \parblock
		 * Should the entity system check and detach any already
		 * existing components that might conflict.
		 *
		 * This sanity check takes a while to run, so it might be
		 * desired to skip it when collisions are guaranteed not to occur.
		 * \endparblock
		 */
		void componentAttach(ComponentId cid, EntityId eid, bool checkDetach = true);
		/** Detaches the component with the given ID from the given entity ID.
		 *
		 * \param cid The ID of the component to detach.
		 * \param eid The ID of the entity to detach it from.
		 *
		 * \note If the entity is the only thing keeping a handle to the component
		 *       then this method might result in the component being destroyed.
		 */
		void componentDetach(ComponentId cid, EntityId eid);

		/** Gets the ID of the entity that the given component ID is attached to.
		 *
		 * \param cid The ID of the component to check.
		 * \note This method is O(n) on number of entities,
		 *       so caching the return value is recommended.
		 */
		EntityId componentGetEntity(ComponentId cid) const;

		/** Gets or creates the EventSystem.
		 */
		EventSystem& getEventSystem();
		/** Gets a reference to the EventSystem.
		 *
		 * \warning This method will null-reference if an EventSystem
		 *          has not been created beforehand.
		 */
		const EventSystem& getEventSystem() const;
		/** Gets or creates the MessageSystem.
		 */
		MessageSystem& getMessageSystem();
		/** Gets a reference to the MessageSystem.
		 *
		 * \warning This method will null-reference if a MessageSystem
		 *          has not been created beforehand.
		 */
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
