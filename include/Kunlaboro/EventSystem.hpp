#pragma once

#include "ID.hpp"

#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Kunlaboro
{

	class EntitySystem;

	/** A low-level event emitting system.
	 *
	 * Useful for events where the higher-level message
	 * passing holds too much overhead.
	 *
	 * \todo See if performance can be improved further.
	 */
	class EventSystem
	{
	public:
		/** The base type of all registered events.
		 *
		 * Should never be used for anything, is only
		 * public to help code locality later on.
		 */
		struct BaseEvent {
			/// The type of the event, Component or Loose.
			std::uint8_t Type;
		};

		typedef std::size_t ListenerId;

		EventSystem(const EventSystem&) = delete;
		EventSystem(EventSystem&&) = delete;
		~EventSystem();

		EventSystem& operator=(const EventSystem&) = delete;

		/** Register a component-based listener to the given event.
		 *
		 * This will automatically unregister the event when
		 * the component is destroyed.
		 *
		 * \tparam Event The event to listen for.
		 * \param func The functor to call when the event is emitted.
		 *
		 * \sa unregisterEvent(ComponentId)
		 */
		template<typename Event, typename Functor>
		void registerEvent(ComponentId cId, Functor&& func);
		/** Register a loose listener for the given event.
		 *
		 * \tparam Event The event to listen for.
		 * \param func The functor to call when the event is emitted.
		 * \returns The ID of the registered listener, this value is
		 *          needed for future unregistering.
		 *
		 * \sa unregisterEvent(ListenerId)
		 */
		template<typename Event, typename Functor>
		ListenerId registerEvent(Functor&& func);
		/** Unregisters all events attached to the given component.
		 *
		 * \param cId The ID of the component to unregister from.
		 */
		void unregisterAllEvents(ComponentId cId);
		/** Unregister an event from a component.
		 *
		 * \tparam Event The event to unregister.
		 * \param cId The ID of the component to unregister from.
		 *
		 * \note This function is O(n) on number of listeners registered
		 *       of the given event type.
		 */
		template<typename Event>
		void unregisterEvent(ComponentId cId);
		/** Unregister a loose event.
		 *
		 * \tparam Event The event to unregister.
		 * \param id The ID of the loose listener to unregister.
		 *
		 * \note This function is O(n) on number of listeners registered
		 *       of the given event type.
		 */
		template<typename Event>
		void unregisterEvent(ListenerId id);
		/** Emits an already created copy of the given event.
		 *
		 * \param ev The pre-created event and data to emit.
		 */
		template<typename Event>
		void emitEvent(const Event& ev) const;
		/** Emits an event with the given arguments.
		 *
		 * \tparam Event The event type to emit.
		 * \param args The arguments to create the event with.
		 */
		template<typename Event, typename... Args>
		void emitEvent(Args... args) const;

	private:
		enum
		{
			sComponentEvent = 1,
			sLooseEvent = 2
		};

		EventSystem(EntitySystem* es);

		friend class EntitySystem;

		EntitySystem* mES;

		std::unordered_map<std::type_index, std::vector<BaseEvent*>> mEvents;
	};

}
