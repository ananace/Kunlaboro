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

	/** A Kunlaboro Component.
	 *
	 * \todo Look into reducing the base memory footprint.
	 */
	class Component
	{
	public:
		enum
		{
			/** The preferred component count per pool block.
			 * 
			 * Larger values mean bigger memory blocks in allocation.
			 * but faster iteration of the memory.
			 */ 
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

	/** Method for looking up component family IDs.
	 * 
	 * \todo Wrap this up in something nicer, possibly allowing for runtime lookup as well.
	 */
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

	/** Abstract base class for a component handle.
	 */
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

		/** Implicitly check that the handle is valid.
		 *
		 * \todo Check that the component hasn't been destroyed in some other manner.
		 */
		inline operator bool() const { return mPtr != nullptr; }

		/** Gets a constant pointer to the component held by the handle.
		 */
		inline const Component* get() const { return mPtr; }
		/** Gets a pointer to the component held by the handle.
		 */
		inline Component* get() { return mPtr; }

		/** Unlinks the handle from the reference counter.
		 *
		 * This will stop the reference count from decreasing when the handle is destroyed.
		 * \note Still not sure about this functionality.
		 */
		void unlink();

		/** Adds a reference to the counter.
		 *
		 * 
		 */
		void addRef();
		/** Explicitly releases the component reference.
		 *
		 * After this function is called, the reference will no longer be valid.
		 */
		void release();
		/** Gets the reference count for the handle.
		 *
		 * \note Will return UINT32_MAX if the handle is invalid.
		 */
		uint32_t getRefCount() const;

	protected:
		BaseComponentHandle(Component* ptr, std::atomic_ushort* counter);

	private:
		friend class EntitySystem;

		Component* mPtr;
		std::atomic_ushort* mCounter;
	};

	/** Implementation of a component handle from a certain component
	 *
	 * \todo Add in a compile-time check about the validity of T
	 */
	template<typename T>
	class ComponentHandle : public BaseComponentHandle
	{
	public:
		ComponentHandle();
		ComponentHandle(const BaseComponentHandle& copy);
		ComponentHandle(BaseComponentHandle&& move);

		/** Gets a constant pointer to the component held by the handle, of the correct type
		 */
		inline const T* get() const { return static_cast<const T*>(BaseComponentHandle::get()); }

		/** Gets a pointer to the component held by the handle, of the correct type
		*/
		inline T* get() { return static_cast<T*>(BaseComponentHandle::get()); }

		/// Reference operator overloading
		const T* operator->() const;
		/// Reference operator overloading
		T* operator->();
		/// Dereference operator overloading
		const T& operator*() const;
		/// Dereference operator overloading
		T& operator*();

	private:
		friend class EntitySystem;

		ComponentHandle(T* ptr, std::atomic_ushort* counter);
	};
}

