#pragma once

#include "ID.hpp"
#include <cassert>

namespace Kunlaboro
{
	class EntitySystem;

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

	protected:
		BaseComponentHandle(Component* ptr, uint32_t* counter);

		static ComponentId::GenerationType sGenerationCounter;

	private:
		friend class EntitySystem;

		Component* mPtr;
		uint32_t* mCounter;
	};

	template<typename T>
	class ComponentHandle : public BaseComponentHandle
	{
	public:
		ComponentHandle();

		inline const T* get() const { return static_cast<const T*>(BaseComponentHandle::get()); }
		inline T* get() { return static_cast<T*>(BaseComponentHandle::get()); }

		const T* operator->() const;
		T* operator->();
		const T& operator*() const;
		T& operator*();

		static ComponentId::GenerationType getGeneration()
		{
			static ComponentId::GenerationType sGeneration = sGenerationCounter++;
			assert(sGenerationCounter != 0);
			return sGeneration;
		}

	private:
		ComponentHandle(T* ptr, uint32_t* counter);
		friend class EntitySystem;
	};
}

