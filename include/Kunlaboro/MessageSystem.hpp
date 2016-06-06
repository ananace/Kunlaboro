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
		/** The locality of the message.
		 *
		 * This is used to differentiate between global and local
		 * message requests, as well as ones that should accept
		 * both global as well as local messages.
		 */
		enum MessageLocality : uint8_t
		{
			/// The message is only accepted when sent globally.
			Message_Global = 1 << 0,
			/// The message is only accepted when sent locally.
			Message_Local  = 1 << 1,
			/// Accept both global and local messages.
			Message_Either = Message_Global | Message_Local
		};

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
		void messageRegisterId(const char* const name, MessageLocality locality = Message_Either);

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
		void messageRegister(MessageId mId, const char* const name = nullptr, MessageLocality locality = Message_Either);

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

		inline static constexpr MessageId hash(const char* const msg)
		{
			return detail::hash_func<MessageId>::hash(msg);
		}
		inline static MessageId hash(const std::string& msg)
		{
			return detail::hash_func<MessageId>::hashF(msg.c_str(), msg.size());
		}

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


		/** Helper struct for storing registered messages.
		 *
		 */
		struct BaseMessageType
		{
			BaseMessageType(MessageId mId)
				: Id(mId)
			{ }

			/// The ID of the message.
			MessageId Id;
#ifdef _DEBUG
			/// The unhashed name of the message, for Debug-time collision checking.
			std::string Name;
#endif
		};

		template<typename... Args>
		struct MessageType : public BaseMessageType
		{
		private:
			struct can_call_test
			{
				template<typename F>
				static decltype(std::declval<F>()(std::declval<Args>()...), std::true_type())
					f(int);

				template<typename F>
				static std::false_type
					f(...);
			};

			template<typename F>
			using can_call = decltype(can_call_test::template f<F>(0));

		public:
			/** Checks that the given functor can be called with the message arguments.
			 */
			template<typename Functor>
			static constexpr bool isValid(Functor&&) { return can_call<Functor>{}; }

			MessageType(MessageId mId)
				: BaseMessageType(mId)
			{ }
		};

		struct MessageData
		{
			MessageLocality Locality;
			BaseMessageType* Type;

			std::deque<BaseMessageCallback*> Callbacks;
		};
		std::unordered_map<MessageId, MessageData> mMessages;
	};

}
