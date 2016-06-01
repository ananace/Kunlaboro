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
		 * \sa isValid()const
		 */
		operator bool() const;

		/** Gets the underlying entity ID.
		 */
		const EntityId& getId() const;
		/** Checks that the entity is valid.
		 */
		bool isValid() const;

		/** Adds a component to the entity.
		 *
		 * \tparam T The component type to add to the entity.
		 * \tparam Args The constructor arguments to create the component with.
		 */
		template<typename T, typename... Args>
		void addComponent(Args...);
		/** Adds or replaces a component in the entity
		 *
		 * \tparam T The component type to add/replace in the entity.
		 * \tparam Args The constructor arguments to create the component with.
		 */
		template<typename T, typename... Args>
		void replaceComponent(Args...);
		/** Removes a component from the entity.
		 *
		 * \tparam T The component type to remove.
		 */
		template<typename T>
		void removeComponent();
		/** Checks if the entity contains a component of the given type.
		 *
		 * \tparam T The component type to check for.
		 */
		template<typename T>
		bool hasComponent() const;
		/** Gets a handle to a component of the given type.
		 *
		 * \tparam T The component type to get.
		 * \note Always check the validity of the handle before using it,
		 *       as the entity might not contain a component of the wanted type.
		 */
		template<typename T>
		ComponentHandle<T> getComponent() const;

		/** Destroys the entity.
		 */
		void destroy();

	private:
		EntitySystem* mES;
		EntityId mId;
	};

}
