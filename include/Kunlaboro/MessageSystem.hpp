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

		/** Register a message with the given arguments and name.
		 *
		 * \tparam Args The arguments for the message.
		 * \param name The name of the message.
		 *
		 * \todo Hash and add possible debug-time call argument checking,
		 *       to prevent passing messages with invalid arguments.
		 */
		template<typename... Args>
		void messageRegisterId(const char* const name);

		/** Add a request for messages with the given name and arguments.
		 *
		 * \tparam Args The arguments of the message.
		 * \param cId The ID of the component to receive the message.
		 * \param message The name of the message to request.
		 * \param func The functor to be called when the message is received.
		 * \param prio The priority of the request, in ascending order.
		 *
		 * \todo Check that the arguments match the ones the message
		 *       was originally registered with.
		 * \todo Add distinction between global and local messages.
		 */
		template<typename... Args, typename Functor>
		void messageRequestId(ComponentId cId, const char* const message, Functor&& func, float prio = 0);
		/** Remove a request for messages with the given name.
		 *
		 * \param cId The ID of the component that posted the original request.
		 * \param message The name of the message that was requested.
		 */
		void messageUnrequestId(ComponentId cId, const char* const message);

		/** Change the request priority for the message with the given name.
		 *
		 * \param cId The ID of the component that posted the original request.
		 * \param message The name of the message that was requested.
		 * \param prio The new priority of the message request.
		 *
		 * \todo Reduce performance impact of this functionality, preferrably
		 *       it should be zero-copy.
		 */
		void messageReprioritizeId(ComponentId cId, const char* const message, float prio);

		/** Send a global message with the given name and arguments.
		 *
		 * \param message The name of the message to send.
		 * \param args The arguments of the message to send.
		 *
		 * \todo Check that the message can accept the given arguments,
		 *       preferrably through run/compile-time switch.
		 */
		template<typename... Args>
		void messageSendId(const char* const message, Args... args) const;
		/** Send a local message to the given component, with name and arguments.
		 *
		 * \param message The name of the message to send.
		 * \param cId The ID of the component to receive the message.
		 * \param args The arguments of the message.
		 *
		 * \todo Check that the message can accept the given arguments,
		 *       preferrably through run/compile-time switch.
		 */
		template<typename... Args>
		void messageSendIdTo(const char* const message, ComponentId cId, Args... args) const;

		// TODO:
		// template<typename... Args>
		// void messageSendIdTo(const char* const message, EntityId eId, Args... args) const;

		/** Register a message with the given ID and arguments.
		 *
		 * \tparam Args The arguments for the message.
		 * \param mId The ID of the message to register.
		 * \param name The name of the message, used for debugging if provided.
		 */
		template<typename... Args>
		void messageRegister(MessageId mId, const char* const name = nullptr);

		/** Request a message by ID.
		 *
		 * \tparam Args The arguments of the message.
		 * \param cId The ID of the component to receive the message.
		 * \param mId The ID of the message to request.
		 * \param func The functor to call upon receiving the message.
		 * \param prio The priority of the request, in ascending order.
		 *
		 * \todo Check that the message can accept the given arguments,
		 *       preferrably through run/compile-time switch.
		 */
		template<typename... Args, typename Functor>
		void messageRequest(ComponentId cId, MessageId mId, Functor&& func, float prio = 0);
		/** Remove a request for a message by ID.
		 *
		 * \param cId The ID of the component that posted the original request.
		 * \param mId The ID of the message that was requested.
		 */
		void messageUnrequest(ComponentId cId, MessageId mId);
		/** Change the request priority for the message with the given ID.
		 *
		 * \param cId The ID of the component that posted the original request.
		 * \param mId The ID of the message that was requested.
		 * \param prio The new priority of the message request.
		 *
		 * \todo Reduce performance impact of this functionality, preferrably
		 *       it should be zero-copy.
		 */
		void messageReprioritize(ComponentId cId, MessageId mId, float prio);

		/** Send a global message with the given ID and arguments.
		 *
		 * \param mId The ID of the message to send.
		 * \param args The arguments of the message to send.
		 *
		 * \todo Check that the message can accept the given arguments,
		 *       preferrably through run/compile-time switch.
		 */
		template<typename... Args>
		void messageSend(MessageId mId, Args... args) const;
		/** Send a local message with the given ID and arguments.
		 *
		 * \param mId The ID of the message to send.
		 * \param cId The ID of the component to receive the message.
		 * \param args The arguments of the message to send.
		 *
		 * \todo Check that the message can accept the given arguments,
		 *       preferrably through run/compile-time switch.
		 */
		template<typename... Args>
		void messageSendTo(MessageId mId, ComponentId cId, Args... args) const;
		// TODO:
		// template<typename... Args>
		// void messageSendTo(MessageId mId, EntityId eId, Args... args) const;

		/** Remove all messages requested by the given component ID.
		 *
		 * \param cId The ID of the component to remove all requests from.
		 */
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
