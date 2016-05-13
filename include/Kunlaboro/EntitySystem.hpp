#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "ID.hpp"

#include "detail/ComponentPool.hpp"

#include <array>
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

		void sendMessage(ComponentId id, Component::BaseMessage* msg);

		template<typename T, typename... Args>
		ComponentHandle<T> componentCreate(Args...);
		void componentDestroy(ComponentId);
		bool componentAlive(ComponentId) const;

		bool componentAttached(ComponentId cid, EntityId eid) const;
		void componentAttach(ComponentId cid, EntityId eid);
		void componentDetach(ComponentId cid, EntityId eid);
		
	private:
		struct ComponentData
		{
			ComponentData()
				: IndexCounter(0), MemoryPool(nullptr)
			{ }

			ComponentId::IndexType IndexCounter;
			std::list<ComponentId::IndexType> FreeIndices;
			std::vector<uint32_t> ReferenceCounter;
			detail::BaseComponentPool* MemoryPool;
		};

		std::vector<EntityId::GenerationType> mGenerations;
		std::list<EntityId::IndexType> mFreeIndices;

		std::vector<ComponentData> mComponentData;
	};
}

#include "EntitySystem.inl"
