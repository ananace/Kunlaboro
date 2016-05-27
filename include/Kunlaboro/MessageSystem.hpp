#pragma once

#include "Message.hpp"

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

		EntitySystem* mES;

		struct MessageData
		{
#ifdef _DEBUG
			std::string Name;
#endif
			std::deque<BaseMessage*> Callbacks;
		};
		std::unordered_map<uint32_t, MessageData> mMessages;
	};
/*
	class MessagingComponent : public Component
	{
	public:

	protected:
		virtual void addedToEntity() = 0;

		template<typename... Args>
		void requestMessage(uint32_t id, const std::function<void(Args...)>& func, float prio = 0);
		void unrequestMessage(uint32_t id);
		void reprioritizeMessage(uint32_t id, float prio);

		template<typename... Args>
		void sendMessage(uint32_t id, Args... args);
		template<typename... Args>
		void sendMessageTo(EntityId ent, uint32_t id, Args... args);
		template<typename... Args>
		void sendMessageTo(ComponentId comp, uint32_t id, Args... args);

	private:
		MessageSystem* mMS;
	};
*/
}
