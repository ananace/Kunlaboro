#ifndef _KUNLABORO_COMPONENT_HPP
#define _KUNLABORO_COMPONENT_HPP

#include <Kunlaboro/Defines.hpp>
#include <functional>
#include <string>

namespace Kunlaboro
{
    class EntitySystem;

    /** \brief A Component inside of an EntitySystem
     *
     * This class is used as a base class of Components inside of an EntitySystem in Kunlaboro,
     * any class that inherits Component can have messages sent and received from it.
     *
     */
    class Component
    {
    public:
        /// \brief Destructor
        virtual ~Component();

        /// \brief The function that is called when the component is added to an entity
        virtual void addedToEntity();


        // Component methods

        /** \brief Adds a component to the local entity.
         *
         *  \param c The component to add.
         */
        void addLocalComponent(Component* c);


        // Request methods

        /** \brief Adds a request for a specific message.
         *
         * When something sends the requested message then the EntitySystem
         * will make sure to call the provided MessageFunction for all components
         * that have requested the message.
         *
         * \param message The message in question.
         * \param func The function to call when the message is sent.
         * \param local Should this request only listen to messages directed to the local entity?
         */
        void requestMessage(const std::string& message, MessageFunction func, bool local = false) const;
        /** \brief Removes a request for a specific message.
         *
         * This function will remove a request that was created by the requestMessage() function.
         *
         * \param message The message in question.
         * \param func The function that was registered for the request.
         * \param local Should this request only listen to messages directed to the local entity?
         */
        void unrequestMessage(const std::string& message, bool local = false) const;
        /** \brief Add a request to be told whenever a specific component is added.
         *
         * Whenever the requested component is added to the local entity or the global
         * Entity System, then the provided MessageFunction will be called.
         *
         * \param name The name of the component.
         * \param func The function to call whenever the component is added.
         * \param local Should the component only care about locally added components.
         * \sa requireComponent()
         */
        void requestComponent(const std::string& name, MessageFunction func, bool local = true) const;
        /** \brief Add a request to be told whenever a specific component is added, and if it's not then don't create the component.
         *
         * This function takes the same parameters as requestParameter(), the difference
         * being that requireComponent() also marks the request as required for the component
         * to be properly usable.
         * When an object is finalized, the EntitySystem will check that the entity contains
         * all of the required components, and if it doesn't then it will be destroyed.
         *
         * \param name The name of the component.
         * \param func The function to call whenever the component is added.
         * \param local Should the component only care about locally added components.
         * \sa requestComponent()
         */
        void requireComponent(const std::string& name, MessageFunction func, bool local = true) const;

        /** \brief Change the priority of a specific request.
         *
         * \param rid The request to change.
         * \param priority The new priority of the request.
         */
        void changeRequestPriority(RequestId rid, int priority) const;

        /** \brief Get the RequestId for the specified message.
         *
         * This function will ask the EntitySystem for the RequestId of the specified message,
         * you can store the RequestId and use that later on when sending messages to speed up
         * the message-sending process.
         *
         * \param message The message to get a RequestId for.
         */
        RequestId getMessageRequestId(const std::string& message) const;


        // Message sending functions

        /** \brief Send a message to the local object.
         *
         * This function will send a message with the specified RequestId and Message object
         * to the entity that contains the current Component.
         * This is the fastest of the three functions as the EntitySystem can just pass
         * along the specified Message without having to create it's own one.
         *
         * \param id The RequestId to send.
         * \param msg The Message to send.
         * \sa sendMessage(RequestId) const
         * \sa sendMessage(RequestId, const Payload&) const
         */
        void sendMessage(RequestId id, const Message& msg) const;

        /** \brief Send a question to the local entity.
         *
         * This function will send a question with the specified RequestId and Message object
         * to the entity that contains the current Component.
         *
         * \param id The RequestId to send.
         * \param msg The Message to send.
         * \returns The response that was recieved, check if handled is set to true before
         * assuming that something actually responded.
         */
        Message sendQuestion(RequestId id, const Message& msg) const;

        /** \brief Send a message to the entire EntitySystem that the local entity is a part of.
         *
         * This function will send a RequestId and Message to the entire EntitySystem and all the
         * Components that have registered a request for the message in question,
         * regardless of which entity that contains them.
         *
         * \param id The RequestId to send.
         * \param msg The Message to send.
         * \sa sendGlobalMessage(RequestId) const
         * \sa sendGlobalMessage(RequestId, const Payload&) const
         */
        void sendGlobalMessage(RequestId id, const Message& msg) const;

        /** \brief Send a message to the entire EntitySystem that the local entity is a part of.
         *
         *
         * \param id The RequestId to send.
         * \param msg The Message to send.
         * \returns The response that was recieved, check if handled is set to true before
         * assuming that something actually responded.
         */
        Message sendGlobalQuestion(RequestId id, const Message& msg) const;

        /** \brief Send a message to a specific entity.
         *
         * This function will send a RequestId and Message to a specific entity in the EntitySystem,
         * this is again the fastest function of the three due to the EntitySystem not having to create
         * a Message object for the request.
         *
         * \param eid The EntityId to send the RequestId to.
         * \param rid The RequestId to send.
         * \param m The Message to send.
         * \sa sendMessageToEntity(EntityId, RequestId) const
         * \sa sendMessageToEntity(EntityId, RequestId, const Payload&) const
         */
        void sendMessageToEntity(EntityId eid, RequestId rid, const Message& m) const;

        /** \brief Send a question to a specific entity.
         *
         *
         * \param eid The EntityId to send the RequestId to.
         * \param rid The RequestId to send.
         * \param m The Message to send.
         * \returns The response that was recieved, check if handled is set to true before
         * assuming that something actually responded.
         */
        Message sendQuestionToEntity(EntityId eid, RequestId rid, const Message& m) const;


        // Convenience functions

        /** \brief Change the priority of a specific request.
         * \note Note that this is slightly slower than using a RequestId, as a call will have to be made
         * to look up the RequestId anyway. Store the RequestId somewhere if you're going to be using this often.
         *
         * \param name The request to change.
         * \param priority The new priority of the request.
         * \sa changeRequestPriority(RequestId, int) const
         */
        inline void changeRequestPriority(const std::string& name, int priority) const { changeRequestPriority(getMessageRequestId(name), priority); }

        /** \brief Send a message to the local object with an empty payload.
         * \note Note that this is slightly slower than having a Message object already assembled for sending.
         *
         * \param id The RequestId to send.
         * \sa sendMessage(RequestId, const Message&) const
         */
        inline void sendMessage(RequestId id) const { sendMessage(id, Message(Type_Message, const_cast<Component*>(this), 0)); }


        inline void sendMessage(const std::string& name) const { sendMessage(getMessageRequestId(name), Message(Type_Message, const_cast<Component*>(this), 0)); }

        /** \brief Send a message to the local object.
         * \note Note that this is slightly slower than having a Message object already assembled for sending.
         *
         * \param id The RequestId to send.
         * \param p The payload to send.
         * \sa sendMessage(RequestId, const Message&) const
         */
        inline void sendMessage(RequestId id, const Payload& p) const { sendMessage(id, Message(Type_Message, const_cast<Component*>(this), p)); }

        /** \brief A convenience function for sending a message to the local object.
         * \note Note that this function is slower than using the RequestId as it has to run an ID
         * lookup for every call, if you want to send many messages then you might want to consider grabbing the
         * RequestId in advance and reusing it.
         * \note Also note that this is slightly slower than having a Message object already assembled for sending.
         *
         * \param name The name of the message.
         * \param p The payload to send.
         * \sa sendMessage(RequestId, const Payload&) const
         */
        inline void sendMessage(const std::string& name, const Payload& p) const { sendMessage(getMessageRequestId(name), Message(Type_Message, const_cast<Component*>(this), p)); }

        /** \brief A convenience function for sending a message without first having to look up the RequestId.
         * \note Note that this function is slower than using the RequestId as it has to run an ID
         * lookup, if you want to send many messages then you might want to consider grabbing the
         * RequestId in advance and reusing it.
         *
         * \param name The name of the message.
         * \param msg The Message data to send.
         * \sa sendMessage(RequestId, const Message&) const
         */
        inline void sendMessage(const std::string& name, const Message& msg) const { sendMessage(getMessageRequestId(name), msg); }

        inline Message sendQuestion(RequestId id) const { return sendQuestion(id, Message(Type_Message, const_cast<Component*>(this), 0)); }
        /** \brief A convenience function for sending a question to the local entity.
         * \note Note that this is slightly slower than having a Message object already assembled for sending.
         *
         * \param id The RequestId to send.
         * \param p The Payload to send.
         * \sa sendQuestion(RequestId, const Message&) const
         */
        inline Message sendQuestion(const std::string& name) const { return sendQuestion(getMessageRequestId(name), Message(Type_Message, const_cast<Component*>(this), 0)); }
        inline Message sendQuestion(RequestId id, const Payload& p) const { return sendQuestion(id, Message(Type_Message, const_cast<Component*>(this), p)); }
        /** \brief A convenience function for sending a question to the local entity.
         * \note Note that this function is slower than using the RequestId as it has to run an ID
         * lookup, if you want to send many messages then you might want to consider grabbing the
         * RequestId in advance and reusing it.
         * \note Also note that this is slightly slower than having a Message object already assembled for sending.
         *
         * \param name The name of the question to send.
         * \param p The Payload to send,
         */
        inline Message sendQuestion(const std::string& name, const Payload& p) const { return sendQuestion(getMessageRequestId(name), Message(Type_Message, const_cast<Component*>(this), p)); }
        /** \brief A convenience function for sending a question to the local entity.
         * \note Note that this function is slower than using the RequestId as it has to run an ID
         * lookup, if you want to send many messages then you might want to consider grabbing the
         * RequestId in advance and reusing it.
         *
         * \param name The name of the question to send.
         * \param msg The Message data to send.
         * \sa sendQuestion(RequestId, const Message&) const
         */
        inline Message sendQuestion(const std::string& name, const Message& msg) const { return sendQuestion(getMessageRequestId(name), msg); }

        inline void sendGlobalMessage(RequestId id) const { sendGlobalMessage(id, Message(Type_Message, const_cast<Component*>(this), 0)); }
        inline void sendGlobalMessage(const std::string& name) const { sendGlobalMessage(getMessageRequestId(name), Message(Type_Message, const_cast<Component*>(this), 0)); }
        /** \brief Send a message to the entire EntitySystem that the local entity is a part of.
         * \note Note that this is slightly slower than having a Message object already assembled for sending.
         *
         * \param id The RequestId to send.
         * \param p The Payload to send.
         * \sa sendGlobalMessage(RequestId) const
         * \sa sendGlobalMessage(RequestId, const Message&) const
         */
        inline void sendGlobalMessage(RequestId id, const Payload& p) const { sendGlobalMessage(id, Message(Type_Message, const_cast<Component*>(this), p)); }
        /** \brief A convenience function for sending a message to the entire EntitySystem that the
         * local entity is part of.
         * \note Note that this function is slower than using the RequestId as it has to run an ID
         * lookup, if you want to send many messages then you might want to consider grabbing the
         * RequestId in advance and reusing it.
         * \note Also note that this is slightly slower than having a Message object already assembled for sending.
         *
         * \param name The name of the message to send.
         * \param p The Payload to send.
         * \sa sendGlobalMessage(RequestId, const Payload&) const
         */
        inline void sendGlobalMessage(const std::string& name, const Payload& p) const { sendGlobalMessage(getMessageRequestId(name), Message(Type_Message, const_cast<Component*>(this), p)); }
        /** \brief A convenience function for sending a message to the entire EntitySystem that the
         * local entity is part of.
         * \note Note that this function is slower than using the RequestId as it has to run an ID
         * lookup, if you want to send many messages then you might want to consider grabbing the
         * RequestId in advance and reusing it.
         *
         * \param name The name of the message to send.
         * \param msg The Message data to send.
         * \sa sendGlobalMessage(RequestId, const Message&) const
         */
        inline void sendGlobalMessage(const std::string& name, const Message& msg) const { sendGlobalMessage(getMessageRequestId(name), msg); }

        inline Message sendGlobalQuestion(RequestId id) const { return sendGlobalQuestion(id, Message(Type_Message, const_cast<Component*>(this), 0)); }
        inline Message sendGlobalQuestion(const std::string& name) const { return sendGlobalQuestion(getMessageRequestId(name), Message(Type_Message, const_cast<Component*>(this), 0)); }
        inline Message sendGlobalQuestion(RequestId id, const Payload& p) const { return sendGlobalQuestion(id, Message(Type_Message, const_cast<Component*>(this), p)); }
        inline Message sendGlobalQuestion(const std::string& name, const Payload& p) const { return sendGlobalQuestion(getMessageRequestId(name), Message(Type_Message, const_cast<Component*>(this), p)); }
        inline Message sendGlobalQuestion(const std::string& name, const Message& msg) const { return sendGlobalQuestion(getMessageRequestId(name), msg); }
        /** \brief Send a message to a specific entity.
         *
         * \param eid The EntityId to send the RequestId to.
         * \param rid The RequestId to send.
         * \sa sendMessageToEntity(EntityId, RequestId, const Payload&) const
         * \sa sendMessageToEntity(EntityId, RequestId, const Message&) const
         */
        inline void sendMessageToEntity(EntityId eid, RequestId rid) const { sendMessageToEntity(eid, rid, Message(Type_Message, const_cast<Component*>(this), 0)); }
        inline void sendMessageToEntity(EntityId eid, const std::string& name) const { sendMessageToEntity(eid, getMessageRequestId(name), Message(Type_Message, const_cast<Component*>(this), 0)); }
        /** \brief Send a message to a specific entity.
         *
         * \param eid The EntityId to send the RequestId to.
         * \param rid The RequestId to send.
         * \param p The Payload to send.
         * \sa sendMessageToEntity(EntityId, RequestId) const
         * \sa sendMessageToEntity(EntityId, RequestId, const Message&) const
         */
        inline void sendMessageToEntity(EntityId eid, RequestId rid, const Payload& p) const { sendMessageToEntity(eid, rid, Message(Type_Message, const_cast<Component*>(this), p)); }
        /** \brief A convenience function for sending a message to a specific entity.
         *
         * \param eid The EntityId to send the message to.
         * \param name The name of the message to send.
         * \param p The Payload to send
         */
        inline void sendMessageToEntity(EntityId eid, const std::string& name, const Payload& p) const { sendMessageToEntity(eid, getMessageRequestId(name), Message(Type_Message, const_cast<Component*>(this), p)); }

        /** \brief A convenience function for sending a message to an entity.
         * \note Note that this function is slower than using the RequestId as it has to run an ID
         * lookup, if you want to send many messages then you might want to consider grabbing the
         * RequestId in advance and reusing it.
         *
         * \param eid The EntityId to send the message to.
         * \param name The name of the message to send.
         * \param m The Message data to send.
         * \sa sendMessageToEntity(EntityId, RequestId, const Message&) const
         */
        inline void sendMessageToEntity(EntityId eid, const std::string& name, const Message& m) const { sendMessageToEntity(eid, getMessageRequestId(name), m); }

        inline Message sendQuestionToEntity(EntityId eid, const std::string& name, const Message& m) const { return sendQuestionToEntity(eid, getMessageRequestId(name), m); }
        inline Message sendQuestionToEntity(EntityId eid, RequestId rid, const Payload& p) const { return sendQuestionToEntity(eid, rid, Message(Type_Message, const_cast<Component*>(this), p)); }
        inline Message sendQuestionToEntity(EntityId eid, const std::string& name, const Payload& p) const { return sendQuestionToEntity(eid, getMessageRequestId(name), Message(Type_Message, const_cast<Component*>(this), p)); }
        inline Message sendQuestionToEntity(EntityId eid, RequestId rid) const { return sendQuestionToEntity(eid, rid, Message(Type_Message, const_cast<Component*>(this), 0)); }
        inline Message sendQuestionToEntity(EntityId eid, const std::string& name) const { return sendQuestionToEntity(eid, getMessageRequestId(name), Message(Type_Message, const_cast<Component*>(this), 0)); }


        // Utility functions

        inline EntitySystem* getEntitySystem() const { return mEntitySystem; }

        /** \brief Get the ComponentId of the current Component.
         *
         * \returns The ComponentId of the current Component.
         */
        inline ComponentId getId() const { return mId; }
        /** \brief Get the EntityId of the entity that contains the current Component.
         *
         * \returns The EntityId of the entity that contains the current Component, or 0 if the
         * Component has not been added to an entity yet.
         */
        inline EntityId getOwnerId() const { return mOwner; }

        /** \brief Destroys the Component
         *
         * This function will tell the EntitySystem that the Component requests to be destroyed,
         * any entities containing it will have the Component removed from them. Unless the entity
         * in question requires the component to exist, at which point the entire entity will be destroyed.
         */
        void destroy();

        /** \brief Returns if the Component is destroyed
         *
         * Checks if the component has been marked as destroyed, generally this function will only return
         * true when a component has been destroyed while the EntitySystem is frozen and before the EntitySystem
         * is unfrozen again.
         *
         * \returns Is the Component destroyed, or waiting to be destroyed.
         */
        inline bool isDestroyed() const { return mDestroyed; }

        /** \brief Checks if the Component is valid.
         *
         * This will check that the Component is in a valid state and is ready for use in the EntitySystem.
         *
         * \returns Is the Component valid.
         */
        inline bool isValid() const { return mOwner != 0 && mId != 0 && mEntitySystem != NULL && !mName.empty()/* && !mDestroyed*/; }

        /** \brief Get the name of the Component.
         *
         * \returns The name of the Component.
         */
        inline const std::string& getName() const { return mName; }

        /** \brief Get a string representation of the Component.
         *
         * This function will return the ComponentId, Name, and EntityId of the entity that owns it.
         *
         * \returns The textual representation of the Component.
         */
        std::string toString() const;


        // Templated request functions

        /** \brief Adds a request for a specific message.
         *
         * This is a convenience function that lets you use a class method as a MessageFunction
         * for the requestMessage(const std::string&, MessageFunction) const function.
         *
         * \param name The message in question.
         * \param f The function to call when the message is sent.
         * \param local Should this request only listen to local message?
         */
        template<class T>
        inline void requestMessage(const std::string& name, void (T::*f)(Message&), bool local = false) const;
        /** \brief Adds a request for a specific message.
         *
         * This is a convenience function that lets you use a class method as a MessageFunction
         * for the requestMessage(const std::string&, MessageFunction) const function.
         *
         * \param name The message in question.
         * \param f The function to call when the message is sent.
         * \param local Should this request only listen to local message?
         */
        template<class T>
        inline void requestMessage(const std::string& name, void (T::*f)(const Message&), bool local = false) const;
        /** \brief Removes a request for a specific message.
         *
         * This is a convenience function that lets you use a class method as a MessageFunction
         * for the unrequestMessage(const std::string&, MessageFunction) const function.
         *
         * \param name The message in question.
         * \param f The function that was registered for the request.
         * \param local Should this request only listen to local message?
         */
        template<class T>
        inline void unrequestMessage(const std::string& name, void (T::*f)(Message&), bool local = false) const;
        /** \brief Removes a request for a specific message.
         *
         * This is a convenience function that lets you use a class method as a MessageFunction
         * for the unrequestMessage(const std::string&, MessageFunction) const function.
         *
         * \param name The message in question.
         * \param f The function that was registered for the request.
         * \param local Should this request only listen to local message?
         */
        template<class T>
        inline void unrequestMessage(const std::string& name, void (T::*f)(const Message&), bool local = false) const;
        /** \brief Add a request to be told whenever a specific component is added.
         *
         * This is a convenience function that lets you use a class method as a MessageFunction
         * for the requestComponent(const std::string&, MessageFunction, bool) const function.
         *
         * \param name The name of the component.
         * \param f The function to call whenever the component is added.
         * \param local Should the component only care about locally added components.
         */
        template<class T>
        inline void requestComponent(const std::string& name, void (T::*f)(const Message&), bool local = true) const;
        /** \brief Add a request to be told whenever a specific component is added, and if it's not then don't create the component.
         *
         * This is a convenience function that lets you use a class method as a MessageFunction
         * for the requireComponent(const std::string&, MessageFunction, bool) const function.
         *
         * \param name The name of the component.
         * \param f The function to call whenever the component is added.
         * \param local Should the component only care about locally added components.
         */
        template<class T>
        inline void requireComponent(const std::string& name, void (T::*f)(const Message&), bool local = true) const;

    protected:
        /// Constructor for the component
        Component(const std::string&);

    private:
        /// Set the owner of the Component
        inline void setOwner(EntityId id) { mOwner = id; }
        /// Set the Component to be destroyed
        inline void setDestroyed() { mDestroyed = true; }

        /// The owner of the Component
        EntityId mOwner;
        /// The EntitySystem that the Component is a part of
        EntitySystem* mEntitySystem;
        /// The ComponentId of the Component
        ComponentId mId;

        /// The name of the Component
        std::string mName;
        /// Is the Component destroyed or not
        bool mDestroyed;

        friend class EntitySystem;
    };


    template<class T>
    void Component::requestMessage(const std::string& name, void (T::*f)(Message&), bool local) const
    {
        requestMessage(name, std::bind(f, (T*)(this), std::placeholders::_1), local);
    }

    template<class T>
    void Component::requestMessage(const std::string& name, void (T::*f)(const Message&), bool local) const
    {
        requestMessage(name, std::bind(f, (T*)(this), std::placeholders::_1), local);
    }

    template<class T>
    void Component::unrequestMessage(const std::string& name, void (T::*f)(Message&), bool local) const
    {
        unrequestMessage(name, std::bind(f, (T*)(this), std::placeholders::_1), local);
    }

    template<class T>
    void Component::unrequestMessage(const std::string& name, void (T::*f)(const Message&), bool local) const
    {
        unrequestMessage(name, std::bind(f, (T*)(this), std::placeholders::_1), local);
    }

    template<class T>
    void Component::requestComponent(const std::string& name, void (T::*f)(const Message&), bool local) const
    {
        requestComponent(name, std::bind(f, (T*)(this), std::placeholders::_1), local);
    }

    template<class T>
    void Component::requireComponent(const std::string& name, void (T::*f)(const Message&), bool local) const
    {
        requireComponent(name, std::bind(f, (T*)(this), std::placeholders::_1), local);
    }
}

std::ostream& operator<<(std::ostream& os, const Kunlaboro::Component& c);

#endif // _KUNLABORO_COMPONENT_HPP

/** \class Kunlaboro::Component
*
* Usage example (Needs Kunlaboro to be compiled with Boost):
* \code
* #include <Kunlaboro/Component.hpp>
* #include <iostream>
*
* class CalculatorComponent : public Kunlaboro::Component
* {
* public:
*     CalculatorComponent() : Kunlaboro::Component("Calculator"), number(0) { }
*
*     void Add(const Kunlaboro::Message& msg)
*     {
*         std::cout << number << " + " << msg.payload << " = ";
*         number += boost::any_cast<int>(msg.payload);
*         std::cout << number << std::endl;
*     }
*
*     void Sub(const Kunlaboro::Message& msg)
*     {
*         std::cout << number << " - " << msg.payload << " = ";
*         number -= boost::any_cast<int>(msg.payload);
*         std::cout << number << std::endl;
*     }
*
*     void Print(const Kunlaboro::Message&)
*     {
*         std::cout << "Calculator contains: " << number << std::endl;
*     }
*
*     virtual void addedToEntity()
*     {
*         requestMessage("Calculator.Add",   &CalculatorComponent::Add);
*         requestMessage("Calculator.Sub",   &CalculatorComponent::Sub);
*         requestMessage("Calculator.Print", &CalculatorComponent::Print);
*     }
*
* private:
*     int number;
* }
* \endcode
*/
