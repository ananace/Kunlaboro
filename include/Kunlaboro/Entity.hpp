#pragma once

#include "ID.hpp"

#include <vector>

namespace Kunlaboro
{
	template<typename T>
	class ComponentHandle;
	class EntitySystem;

	class Entity
	{
	public:
		Entity();
		Entity(const Entity&) = default;
		Entity(Entity&&) = default;
		~Entity() = default;

		Entity& operator=(const Entity&) = default;

		bool operator==(const Entity& other) const;
		bool operator!=(const Entity& other) const;

		explicit operator EntityId() const;
		operator bool() const;

		const EntityId& getId() const;
		bool isValid() const;

		template<typename T, typename... Args>
		void addComponent(Args...);
		template<typename T, typename... Args>
		void replaceComponent(Args...);
		template<typename T>
		void removeComponent();
		template<typename T>
		bool hasComponent() const;
		template<typename T>
		ComponentHandle<T> getComponent() const;

		void destroy();

		//const EntitySystem::ComponentIterator& getAllComponents() const;

	private:
		Entity(EntitySystem* sys, EntityId id);

		friend class EntitySystem;
		
		EntitySystem* mES;
		EntityId mId;
	};

}
