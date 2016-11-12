#pragma once

#include "Component.hpp"
#include <functional>

namespace Kunlaboro
{

	class MessageSystem;
	typedef uint32_t MessageId;

	/** Helper component for handling messaging.
	 */
	class MessagingComponent : public Component
	{
	public:
		MessagingComponent();

	protected:
		/** This function is called every time the component is added to an entity.
		 *
		 * In here you want to request the messages you want to receive during the
		 * lifetime of your attachment to the current entity.
		 *
		 * Example of how such functionality can be done is;
		 * \code{.cpp}
		 * void Example::addedToEntity()
		 * {
		 *   requestMessage<float>("System.Tick", &Example::Tick);
		 *   requestMessage("System.Reset", &Example::Reset);
		 *   requestMessage<std::string&>("System.Display", &Example::Display, priority);
		 *   // Can also be changed later on through
		 *   // reprioritizeMessageId("System.Display", priority);
		 * }
		 *
		 * void Example::Tick(float dt)
		 * {
		 *   // Run an update tick
		 * }
		 * void Example::Reset()
		 * {
		 *   // Reset the system state
		 * }
		 * void Example::Display(std::string& out)
		 * {
		 *   // Append state to output variable
		 * }
		 * \endcode
		 */
		virtual void addedToEntity() = 0;

		/** Request to receive messages with the given ID.
		 *
		 * \param id The ID of the message to receive.
		 * \param func The functor to receive the message.
		 * \param prio The priority of the request, in ascending order.
		 * \tparam Args The message arguments, must match the functor input.
		 */
		template<typename... Args, typename Functor>
		void requestMessage(MessageId id, Functor&& func, float prio = 0);
		/** Helper method for requesting against a member method.
		 *
		 * \param id The ID of the message to receive.
		 * \param func The member method to receive the message.
		 * \param prio The priority of the request, in ascending order.
		 * \tparam Args The message arguments, must match the method arguments.
		 */
		template<typename... Args, typename Obj>
		void requestMessage(MessageId id, void (Obj::*func)(Args...), float prio = 0);
		/** Unrequest the message with the given ID.
		 */
		void unrequestMessage(MessageId id);
		/** Change the priority of the message with the given ID.
		 *
		 * \todo Look into possible performance increases.
		 */
		void reprioritizeMessage(MessageId id, float prio);

		/** Sends a global message with the given ID.
		 *
		 * \param id The ID of the message to send.
		 * \param args The message arguments.
		 */
		template<typename... Args>
		void sendMessage(MessageId id, Args... args) const;
		/** Sends a local message with the given ID to the given entity ID.
		 *
		 * \param id The ID of the message to send.
		 * \param ent The ID of the entity to receive the message.
		 * \param args The message arguments.
		 */
		template<typename... Args>
		void sendMessageTo(MessageId id, EntityId ent, Args... args) const;
		/** Sends a local message with the given ID to the given component ID.
		 *
		 * \param id The ID of the message to send.
		 * \param comp The ID of the component to receive the message.
		 * \param args The message arguments.
		 */
		template<typename... Args>
		void sendMessageTo(MessageId id, ComponentId comp, Args... args) const;

		/** Helper method to request a message from an unhashed string.
		 *
		 * \param id The ID string of the message to request.
		 * \param func The functor to receive the call.
		 * \param prio The priority of the request, in ascending order.
		 *
		 * \note Even though these functions hash at compile time,
		 *       using pre-hashed messages will be faster.
		 */
		template<typename... Args, typename Functor>
		void requestMessageId(const char* const id, Functor&& func, float prio = 0);
		/** Helper method to request a message from an unhashed string.
		 *
		 * \param id The ID string of the message to request.
		 * \param func The member function to receive the call.
		 * \param prio The priority of the request, in ascending order.
		 *
		 * \note Even though these functions hash at compile time,
		 *       using pre-hashed messages will be faster.
		 */
		template<typename... Args, typename Obj>
		void requestMessageId(const char* const id, void (Obj::*func)(Args...), float prio = 0);
		/** Helper method to unrequest a message from an unhashed string.
		 *
		 * \param id The ID string of the message to unrequest.
		 *
		 * \note Even though these functions hash at compile time,
		 *       using pre-hashed messages will be faster.
		 */
		void unrequestMessageId(const char* const id);
		/** Helper method to change message priority from an unhashed string.
		 *
		 * \param id The ID string of the message to reprioritize.
		 * \param prio The new priority of the request, in ascending order.
		 *
		 * \note Even though these functions hash at compile time,
		 *       using pre-hashed messages will be faster.
		 */
		void reprioritizeMessageId(const char* const id, float prio);

		/** Helper method to send a global message by an unhashed string.
		 *
		 * \param id The ID string of the message to send.
		 * \param args The arguments of the message.
		 *
		 * \note Even though these functions hash at compile time,
		 *       using pre-hashed messages will be faster.
		 */
		template<typename... Args>
		void sendMessageId(const char* const id, Args... args) const;
		/** Helper method to send a entity-local message by an unhashed string.
		 *
		 * \param id The ID string of the message to send.
		 * \param ent The ID of the entity to receive the message.
		 * \param args The arguments of the message.
		 *
		 * \note Even though these functions hash at compile time,
		 *       using pre-hashed messages will be faster.
		 */
		template<typename... Args>
		void sendMessageIdTo(const char* const id, EntityId ent, Args... args) const;
		/** Helper method to send a component-local message by an unhashed string.
		 *
		 * \param id The ID string of the message to send.
		 * \param comp The ID of the component to receive the message.
		 * \param args The arguments of the message.
		 *
		 * \note Even though these functions hash at compile time,
		 *       using pre-hashed messages will be faster.
		 */
		template<typename... Args>
		void sendMessageIdTo(const char* const id, ComponentId comp, Args... args) const;
	};

}
