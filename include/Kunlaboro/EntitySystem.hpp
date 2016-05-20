#pragma once

#include "Component.hpp"
#include "Entity.hpp"
#include "ID.hpp"

#include "detail/DynamicBitfield.hpp"

#include <array>
#include <atomic>
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
		bool entityHasComponent(ComponentId::FamilyType family, EntityId eid) const;

		template<typename T, typename... Args>
		ComponentHandle<T> componentCreate(Args...);
		void componentDestroy(ComponentId);
		bool componentAlive(ComponentId) const;

		bool componentAttached(ComponentId cid, EntityId eid) const;
		void componentAttach(ComponentId cid, EntityId eid);
		void componentDetach(ComponentId cid, EntityId eid);
		
		EntityId componentGetEntity(ComponentId cid) const;

	private:
		

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
