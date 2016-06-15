#pragma once

#include "ID.hpp"

#include <vector>

namespace Kunlaboro
{
	template<typename T>
	class ComponentHandle;
	class EntitySystem;
	class EntityView;

	/** An entity reference helper.
	 *
	 * This class is just a wrapper around the entity system and a given entity ID.
	 */
	class Entity
	{
	public:
		/** Default constructor.
		 */
		Entity();

		/** Creates an entity reference from a system and an ID.
		 *
		 * \param sys The entity system containing the entity.
		 * \param id The ID of the entity.
		 * \note This constructor does no validity testing of the requested entity.
		 */
		Entity(EntitySystem* sys, EntityId id);
		Entity(const Entity&) = default;
		Entity(Entity&&) = default;
		~Entity() = default;

		Entity& operator=(const Entity&) = default;

		bool operator==(const Entity& other) const;
		bool operator!=(const Entity& other) const;

		/** Gets the underlying entity ID.
		 */
		explicit operator EntityId() const;
		/** Checks if the entity is valid.
		 *
		 * \sa isValid() const
		 */
		operator bool() const;

		/** Gets the underlying entity ID.
		 */
		const EntityId& getId() const;
		/** Checks that the entity is valid.
		 *
		 * \sa EntitySystem::isAlive(EntityId)
		 */
		bool isValid() const;

		/** Adds a component to the entity.
		 *
		 * This method is functionally identical to the following block of code;
		 * \code{.cpp}
		 * auto componentId = EntitySystem.createComponent<T>(Args...);
		 * EntitySystem.attachComponent(entityId, componentId);
		 * \endcode
		 *
		 * \tparam T The component type to add to the entity.
		 * \tparam Args The constructor arguments to create the component with.
		 *
		 * \sa EntitySystem::createComponent()
		 * \sa EntitySystem::attachComponent()
		 */
		template<typename T, typename... Args>
		void addComponent(Args...);
		/** Adds or replaces a component in the entity
		 *
		 * Difference between this and \a addComponent is that this method
		 * will destroy any existing component of the given type, before
		 * creating a new one from the given arguments.
		 *
		 * \tparam T The component type to add/replace in the entity.
		 * \tparam Args The constructor arguments to create the component with.
		 *
		 * \sa EntitySystem::hasComponent()
		 * \sa EntitySystem::detachComponent()
		 * \sa EntitySystem::createComponent()
		 * \sa EntitySystem::attachComponent()
		 */
		template<typename T, typename... Args>
		void replaceComponent(Args...);
		/** Removes a component from the entity.
		 *
		 * This method is functionally identical to the following block of code;
		 * \code{.cpp}
		 * auto componentId = EntitySystem.getComponent<T>(entityId);
		 * EntitySystem.detachComponent(componentId, entityId);
		 * \endcode
		 *
		 * \tparam T The component type to remove.
		 *
		 * \sa EntitySystem::detachComponent()
		 */
		template<typename T>
		void removeComponent();
		/** Checks if the entity contains a component of the given type.
		 *
		 * This method is functionally identical to the following block of code;
		 * \code{.cpp}
		 * return EntitySystem.hasComponent(ComponentFamily<T>::getFamily());
		 * \endcode
		 *
		 * \tparam T The component type to check for.
		 *
		 * \sa EntitySystem::hasComponent()
		 */
		template<typename T>
		bool hasComponent() const;
		/** Gets a handle to a component of the given type.
		 *
		 * This method is functionally identical to the following block of code;
		 * \code{.cpp}
		 * return EntitySystem.getComponent<T>(entityId);
		 * \endcode
		 *
		 * \tparam T The component type to get.
		 * \note Always check the validity of the handle before using it,
		 *       as the entity might not contain a component of the wanted type.
		 *
		 * \sa EntitySystem::getComponent(EntityId)
		 */
		template<typename T>
		ComponentHandle<T> getComponent() const;

		/** Destroys the entity.
		 *
		 * This method is functionally identical to the following block of code;
		 * \code{.cpp}
		 * EntitySystem.destroyEntity(entityId);
		 * \endcode
		 *
		 * \sa EntitySystem::destroyEntity()
		 */
		void destroy();

	private:
		EntitySystem* mES;
		EntityId mId;
	};

}
