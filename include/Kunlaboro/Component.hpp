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

	/** The Kunlaboro Component base class.
	 *
	 * This is the base class that needs to be used when registering
	 * component types for use in the Kunlaboro entity system.
	 *
	 * Simple example dealing with a cooldown for a trigger;
	 * \code{.cpp}
	 * class CooldownComponent : public Kunlaboro::Component
	 * {
	 * public:
	 * 	CooldownComponent(float time)
	 * 		: Time(time)
	 * 	{ }
	 *
	 * 	void update(float dt)
	 * 	{
	 * 		Time -= dt;
	 *
	 * 		if (Time > 0)
	 * 			return;
	 *
	 * 		getEntitySystem()->componentDestroy(getId());
	 * 	}
	 *
	 * 	// Time remaining on the cooldown
	 * 	float Time;
	 * };
	 * \endcode
	 *
	 * \todo Look into reducing the base memory footprint.
	 */
	class Component
	{
	public:
		enum
		{
			/** The preferred component count per block of pool memory.
			 *
			 * \note Larger values mean bigger memory blocks per allocation,
			 *       reducing number of allocations needed for large amounts
			 *       of components.
			 */
			sPreferredChunkSize = 256
		};

		virtual ~Component() = default;

		/** Gets the ID of the component.
		 *
		 * On creation each component is given an identifier based on its
		 * type, index, and generation of that index. This is guaranteed to
		 * be unique in most cases, while still favouring lower indexes and
		 * therefore faster memory lookups.
		 *
		 * \note
		 * This identifier may no longer be unique after ComponentId::sMaxGeneration
		 * of generations have passed.
		 */
		const ComponentId& getId() const;
		/** Gets the ID of the entity containing the component.
		 *
		 * Each component can only be held by a single entity at a time,
		 * so this is guaranteed to be either a valid entity or EntityId::Invalid()
		 *
		 * \note This function will require iterating the entity table,
		 *       so performance is O(n) on number of existing entities.
		 *       Recommended is to cache the resulting value if it's to
		 *       be used repeatedly.
		 */
		EntityId getEntityId() const;

		/// Gets the pointer to the entity system that owns the component.
		EntitySystem* getEntitySystem();
		/// Gets a const pointer to the entity system that owns the component.
		const EntitySystem* getEntitySystem() const;

	private:
		friend class EntitySystem;

		/** The pointer to the EntitySystem that owns the component.
		 *
		 * \note This might be removed in order to reduce component footprint.
		 */
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

		/** Retrieves the ID of the requested component family.
		 *
		 * \note This is backed by an incrementing global counter for now,
		 *       which might lead to issues when using several separate
		 *       entity systems in the same application.
		 */
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

