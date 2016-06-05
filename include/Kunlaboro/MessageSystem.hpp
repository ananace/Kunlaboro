#pragma once

#include "ID.hpp"
#include "Message.hpp"

#include <unordered_map>
#include <deque>

namespace Kunlaboro
{

	class EntitySystem;

	/** Message passing system
	 *
	 * \note Uses hashed strings as message IDs
	 * \todo Do collision testing on debug builds
	 */
	class MessageSystem
	{
	public:
		MessageSystem(const MessageSystem&) = delete;
		MessageSystem(MessageSystem&&) = delete;
		~MessageSystem();

		MessageSystem& operator=(const MessageSystem&) = delete;

		template<typename... Args>
		void messageRegisterId(const char* const name);

		template<typename... Args, typename Functor>
		void messageRequestId(ComponentId cId, const char* const message, Functor&& func, float prio = 0);
		void messageUnrequestId(ComponentId cId, const char* const message);

		void messageReprioritizeId(ComponentId cId, const char* const message, float prio);

		template<typename... Args>
		void messageSendId(const char* const message, Args... args) const;
		template<typename... Args>
		void messageSendIdTo(const char* const message, ComponentId cId, Args... args) const;

		template<typename... Args>
		void messageRegister(MessageId mId);

		template<typename... Args, typename Functor>
		void messageRequest(ComponentId cId, MessageId mId, Functor&& func, float prio = 0);
		void messageUnrequest(ComponentId cId, MessageId mId);

		void messageReprioritize(ComponentId cId, MessageId mId, float prio);

		template<typename... Args>
		void messageSend(MessageId mId, Args... args) const;
		template<typename... Args>
		void messageSendTo(MessageId mId, ComponentId cId, Args... args) const;
		// template<typename... Args>
		// void messageSendTo(MessageId mId, EntityId eId, Args... args) const;

		void messageUnrequestAll(ComponentId cId);

	private:
		MessageSystem(EntitySystem* es);

		friend class EntitySystem;

		EntitySystem* mES;

		struct BaseMessageCallback
		{
			BaseMessageCallback(ComponentId cId, float p)
				: Component(cId)
				, Priority(p)
			{ }

			ComponentId Component;
			float Priority;

			inline bool operator<(const BaseMessageCallback& cb) const
			{
				return Priority < cb.Priority;
			}
		};
		template<typename... Args>
		struct MessageCallback : public BaseMessageCallback
		{
			MessageCallback(ComponentId cId, float p, std::function<void(Args...)>&& func)
				: BaseMessageCallback(cId, p)
				, Func(func)
			{ }

			std::function<void(Args...)> Func;
		};

		struct MessageData
		{
			BaseMessageType* Type;

			std::deque<BaseMessageCallback*> Callbacks;
		};
		std::unordered_map<MessageId, MessageData> mMessages;
	};

}
