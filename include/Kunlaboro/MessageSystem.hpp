#pragma once

#include <functional>
#include <unordered_map>
#include <deque>

namespace Kunlaboro
{

	class EntitySystem;

	class MessageSystem
	{
	public:
		MessageSystem(const MessageSystem&) = delete;
		MessageSystem(MessageSystem&&) = delete;
		~MessageSystem();

		MessageSystem& operator=(const MessageSystem&) = delete;



	private:
		MessageSystem(EntitySystem* es);

		friend class EntitySystem;
	};

}
