#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

namespace Kunlaboro
{

	class EntitySystem;

	class EventSystem
	{
	public:
		EventSystem(const EventSystem&) = delete;
		EventSystem(EventSystem&&) = delete;
		~EventSystem();

		EventSystem& operator=(const EventSystem&) = delete;

		template<typename Functor, typename Event>
		void eventRegister(Functor&& func);
		template<typename Functor, typename Event>
		void eventUnregister(Functor&& func);
		template<typename Event>
		void eventEmit(const Event& ev) const;
		template<typename Event, typename... Args>
		void eventEmit(Args... args) const;


	private:
		EventSystem(EntitySystem* es);

		friend class EntitySystem;
	};

}
