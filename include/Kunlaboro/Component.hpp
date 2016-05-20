#pragma once

#include "ID.hpp"

#include <atomic>
#include <type_traits>
#include <cassert>

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
		enum
		{
			sPreferredChunkSize = 256
		};

		virtual ~Component() = default;

		const ComponentId& getId() const;
		EntityId getEntityId() const;

		EntitySystem* getEntitySystem();
		const EntitySystem* getEntitySystem() const;

	private:
		friend class EntitySystem;

		EntitySystem* mES;
		ComponentId mId;
	};

	template<typename T>
	class ComponentFamily : BaseComponentFamily
	{
	public:
		static_assert(std::is_base_of<Component, T>::value, "Only components have families");

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
		BaseComponentHandle(Component* ptr, std::atomic_ushort* counter);

	private:
		friend class EntitySystem;

		Component* mPtr;
		std::atomic_ushort* mCounter;
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
		ComponentHandle(T* ptr, std::atomic_ushort* counter);
		friend class EntitySystem;
	};
}

