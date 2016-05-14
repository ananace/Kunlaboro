#pragma once

#include "ID.hpp"

#include <cassert>
#include <vector>

namespace Kunlaboro
{
	class EntitySystem;

	struct BaseComponentFamily
	{
	protected:
		static ComponentId::FamilyType sFamilyCounter;
	};

	class Component
	{
	public:
		struct BaseMessage
		{
			uint32_t MessageId;
		};

		struct IdentifiedMessage : public BaseMessage
		{
			ComponentId SenderComponent;
			EntityId SenderEntity;
		};

		const ComponentId& getId() const;
		const EntityId& getEntityId() const;

		virtual void onMessage(BaseMessage*) = 0;

		EntitySystem* getEntitySystem();
		const EntitySystem* getEntitySystem() const;

	protected:
		Component();

	private:
		friend class EntitySystem;

		EntitySystem* mES;
		ComponentId mId;
		EntityId mOwnerId;
	};

	template<typename T>
	class ComponentFamily : BaseComponentFamily
	{
	public:
		static const ComponentId::FamilyType getFamily()
		{
			static ComponentId::FamilyType sFamily = sFamilyCounter++;
			assert(sFamilyCounter != 0 && sFamily <= ComponentId::sMaxFamily);
			return sFamily;
		}
	};

	class BaseComponentHandle
	{
	public:
		BaseComponentHandle();
		BaseComponentHandle(const BaseComponentHandle& copy);
		BaseComponentHandle(BaseComponentHandle&& move);
		virtual ~BaseComponentHandle();

		BaseComponentHandle& operator=(const BaseComponentHandle& assign);

		bool operator==(const BaseComponentHandle& rhs) const;
		bool operator!=(const BaseComponentHandle& rhs) const;

		inline operator bool() const { return mPtr != nullptr; }

		inline const Component* get() const { return mPtr; }
		inline Component* get() { return mPtr; }

		void unlink();

		void addRef();
		void release();
		uint32_t getRefCount() const;

	protected:
		BaseComponentHandle(Component* ptr, std::vector<uint32_t>* counter);

	private:
		friend class EntitySystem;

		Component* mPtr;
		std::vector<uint32_t>* mCounters;
	};

	template<typename T>
	class ComponentHandle : public BaseComponentHandle
	{
	public:
		ComponentHandle();
		ComponentHandle(const BaseComponentHandle& copy);
		ComponentHandle(BaseComponentHandle&& move);

		inline const T* get() const { return static_cast<const T*>(BaseComponentHandle::get()); }
		inline T* get() { return static_cast<T*>(BaseComponentHandle::get()); }

		const T* operator->() const;
		T* operator->();
		const T& operator*() const;
		T& operator*();

	private:
		ComponentHandle(T* ptr, std::vector<uint32_t>* counter);
		friend class EntitySystem;
	};
}

