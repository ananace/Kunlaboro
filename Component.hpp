/*  Kunlaboro - Component

    The MIT License (MIT)
    Copyright (c) 2012 Alexander Olofsson (ace@haxalot.com)
 
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _KUNLABORO_COMPONENT_HPP
#define _KUNLABORO_COMPONENT_HPP

#include "Defines.hpp"
#include <functional>
#include <string>

#ifndef _WIN32
#include <tr1/functional>
#endif

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

        /** \brief Adds a component to the local entity.
         *   
         *  \param c The component to add.
         */
        void addLocalComponent(Component* c);
        
        /** \brief Adds a request for a specific message.
         *
         * When something sends the requested message then the EntitySystem 
         * will make sure to call the provided MessageFunction for all components
         * that have requested the message.
         *
         * \param message The message in question.
         * \param func The function to call when the message is sent.
         * \param local Should this request only listen to local message?
         */
        void requestMessage(const std::string& message, MessageFunction func, bool local = false) const;
        /** \brief Removes a request for a specific message.
         *
         * This function will remove a request that was created by the requestMessage() function, note that the
         * arguments must be identical to the requestMessage() call.
         *
         * \param message The message in question.
         * \param func The function that was registered for the request.
         * \param local Should this request only listen to local message?
         */
        void unrequestMessage(const std::string& message, MessageFunction func, bool local = false) const;
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
        /** \brief Change the priority of a specific request.
         *
         * \param name The request to change.
         * \param priority The new priority of the request.
         */
        inline void changeRequestPriority(const std::string& name, int priority) const { changeRequestPriority(getMessageRequestId(name), priority); }

        /** \brief Get the RequestId for the specified message.
         *
         * This function will ask the EntitySystem for the RequestId of the specified message,
         * you can store the RequestId and use that later on when sending messages to speed up
         * the message-sending process.
         *
         * \param message The message to get a RequestId for.
         */
        RequestId getMessageRequestId(const std::string& message) const;

        /** \brief Send a message to the local object.
         *
         * This function will send a message with the specified RequestId to the entity that
         * contains the current Component.
         *
         * \param id The RequestId to send.
         * \sa sendMessage(RequestId, const Payload&) const
         * \sa sendMessage(RequestId, const Message&) const
         */
        void sendMessage(RequestId id) const;
        /** \brief Send a message to the local object.
         *
         * This function will send a message with the specified RequestId and Payload to the
         * entity that contains the current Component.
         *
         * \param id The RequestId to send.
         * \param p The payload to send.
         * \sa sendMessage(RequestId) const
         * \sa sendMessage(RequestId, const Message&) const
         */
        void sendMessage(RequestId id, const Payload& p) const;
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
        /** \brief Send a question to the local object.
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
         * This function will send a RequestId to the entire EntitySystem and all the
         * Components that have registered a request for the message in question,
         * regardless of which entity that contains them.
         * 
         * \param id The RequestId to send.
         * \sa sendGlobalMessage(RequestId, const Payload&) const
         * \sa sendGlobalMessage(RequestId, const Message&) const
         */
        void sendGlobalMessage(RequestId id) const;
        /** \brief Send a message to the entire EntitySystem that the local entity is a part of.
         *
         * This function will send a RequestId and Payload to the entire EntitySystem and all the
         * Components that have registered a request for the message in question,
         * regardless of which entity that contains them.
         * 
         * \param id The RequestId to send.
         * \param p The Payload to send.
         * \sa sendGlobalMessage(RequestId) const
         * \sa sendGlobalMessage(RequestId, const Message&) const
         */
        void sendGlobalMessage(RequestId id, const Payload& p) const;
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
         * This function will send a RequestId to a specific entity in the EntitySystem.
         *
         * \param eid The EntityId to send the RequestId to.
         * \param rid The RequestId to send.
         * \sa sendMessageToEntity(EntityId, RequestId, const Payload&) const
         * \sa sendMessageToEntity(EntityId, RequestId, const Message&) const
         */
        void sendMessageToEntity(EntityId eid, RequestId rid) const;
        /** \brief Send a message to a specific entity.
         *
         * This function will send a RequestId and Payload to a specific entity in the EntitySystem.
         *
         * \param eid The EntityId to send the RequestId to.
         * \param rid The RequestId to send.
         * \param p The Payload to send.
         * \sa sendMessageToEntity(EntityId, RequestId) const
         * \sa sendMessageToEntity(EntityId, RequestId, const Message&) const
         */
        void sendMessageToEntity(EntityId eid, RequestId rid, const Payload& p) const;
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
        /** \brief Send a message to a specific entity.
         *
         * 
         * \param eid The EntityId to send the RequestId to.
         * \param rid The RequestId to send.
         * \param m The Message to send.
         * \returns The response that was recieved, check if handled is set to true before
         * assuming that something actually responded.
         */
        Message sendQuestionToEntity(EntityId eid, RequestId rid, const Message& m) const;

        /** \brief Send a message to the entire EntitySystem that the local entity is a part of.
         *
         * This function is a convenience function that lets you quickly send out a global message
         * without having to get the RequestId in advance, and while not a function you should use
         * repeatedly, it is good if you just need to send a quick message.
         *
         * \param id The name of the request to send.
         * \param p The payload to send.
         */
        void sendGlobalMessage(const std::string& id, const Payload& p = 0) const;
        /** \brief Send a message to the entire EntitySystem that the local entity is a part of.
         *
         * This function is a convenience function that lets you quickly send out a global message
         * without having to get the RequestId in advance, and while not a function you should use
         * repeatedly, it is good if you just need to send a quick message.
         *
         * \param id The name of the request to send.
         * \param p The payload to send.
         */
        Message sendGlobalQuestion(const std::string& id, const Payload& p = 0) const;
        /** \brief Get a pointer to the EntitySystem that this Component is a part of.
         *
         * This function returns a pointer to the EntitySystem that governs the current component,
         * this function will never return a NULL pointer.
         *
         * \returns An EntitySystem pointer.
         */
        inline EntitySystem* getEntitySystem() const { return mEntitySystem; }

        /** \brief Get the ComponentId of the current Component.
         *
         * \returns The ComponentId of the current Component.
         */
        ComponentId getId() const;
        /** \brief Get the EntityId of the entity that contains the current Component.
         *
         * \returns The EntityId of the entity that contains the current Component, or 0 if the
         * Component has not been added to an entity yet.
         */
        EntityId getOwnerId() const;

        /** \brief Destroys the Component
         * 
         * This function will tell the EntitySystem that the Component requests to be destroyed,
         * any entities containing it will have the Component removed from them. Unless the entity
         * in question requires the component to exist, at which point the entire entity will be destroyed.
         */
        void destroy();
        /** \brief Returns if the Component is destroyed
         *
         * \returns Is the Component destroyed, or waiting to be destroyed.
         */
        bool isDestroyed() const;

        /** \brief Checks if the Component is valid.
         *
         * This will check that the Component has a valid owner, is not destroyed, and has a valid name.
         *
         * \returns Is the Component valid.
         */
        bool isValid() const;

        /** \brief Get the name of the Component.
         *
         * \returns The name of the Component.
         */
        const std::string& getName() const;
        /** \brief Get a string representation of the Component.
         *
         * This function will return the ComponentId, Name, and EntityId of the entity that owns it.
         *
         * \returns The textual representation of the Component.
         */
        std::string toString() const;

        /** \brief Adds a request for a specific message.
         *
         * This is a convenience function that lets you use a class method as a MessageFunction
         * for the requestMessage(const std::string&, MessageFunction) const function.
         *
         * \param name The message in question.
         * \param f The function to call when the message is sent.
         * \param priority The priority of this component, lower priorities will get the message before higher priorities.
         * \param local Should this request only listen to local message?
         */
        template<class T>
        void requestMessage(const std::string& name, void (T::*f)(Message&), bool local = false) const;
        /** \brief Adds a request for a specific message.
         *
         * This is a convenience function that lets you use a class method as a MessageFunction
         * for the requestMessage(const std::string&, MessageFunction) const function.
         *
         * \param name The message in question.
         * \param f The function to call when the message is sent.
         * \param priority The priority of this component, lower priorities will get the message before higher priorities.
         * \param local Should this request only listen to local message?
         */
        template<class T>
        void requestMessage(const std::string& name, void (T::*f)(const Message&), bool local = false) const;
        /** \brief Removes a request for a specific message.
         *
         * This is a convenience function that lets you use a class method as a MessageFunction
         * for the unrequestMessage(const std::string&, MessageFunction) const function.
         *
         * \param name The message in question.
         * \param f The function that was registered for the request.
         * \param priority The priority that was used when registering the request.
         * \param local Should this request only listen to local message?
         */
        template<class T>
        void unrequestMessage(const std::string& name, void (T::*f)(Message&), bool local = false) const;
        /** \brief Removes a request for a specific message.
         *
         * This is a convenience function that lets you use a class method as a MessageFunction
         * for the unrequestMessage(const std::string&, MessageFunction) const function.
         *
         * \param name The message in question.
         * \param f The function that was registered for the request.
         * \param priority The priority that was used when registering the request.
         * \param local Should this request only listen to local message?
         */
        template<class T>
        void unrequestMessage(const std::string& name, void (T::*f)(const Message&), bool local = false) const;
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
        void requestComponent(const std::string& name, void (T::*f)(const Message&), bool local = true) const;
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
        void requireComponent(const std::string& name, void (T::*f)(const Message&), bool local = true) const;

    protected:
        /// Constructor for the component
        Component(const std::string&);

    private:
        /// Set the owner of the Component
        void setOwner(EntityId);
        /// Set the Component to be destroyed
        void setDestroyed();

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
        requestMessage(name, std::tr1::bind(f, (T*)(this), std::tr1::placeholders::_1), local);
    }

    template<class T>
    void Component::requestMessage(const std::string& name, void (T::*f)(const Message&), bool local) const
    {
        requestMessage(name, std::tr1::bind(f, (T*)(this), std::tr1::placeholders::_1), local);
    }

    template<class T>
    void Component::unrequestMessage(const std::string& name, void (T::*f)(Message&), bool local) const
    {
        unrequestMessage(name, std::tr1::bind(f, (T*)(this), std::tr1::placeholders::_1), local);
    }

    template<class T>
    void Component::unrequestMessage(const std::string& name, void (T::*f)(const Message&), bool local) const
    {
        unrequestMessage(name, std::tr1::bind(f, (T*)(this), std::tr1::placeholders::_1), local);
    }

    template<class T>
    void Component::requestComponent(const std::string& name, void (T::*f)(const Message&), bool local) const
    {
        requestComponent(name, std::tr1::bind(f, (T*)(this), std::tr1::placeholders::_1), local);
    }

    template<class T>
    void Component::requireComponent(const std::string& name, void (T::*f)(const Message&), bool local) const
    {
        requireComponent(name, std::tr1::bind(f, (T*)(this), std::tr1::placeholders::_1), local);
    }
}

std::ostream& operator<<(std::ostream& os, const Kunlaboro::Component& c);

#endif // _KUNLABORO_COMPONENT_HPP

/** \class Kunlaboro::Component
*
* Usage example:
* \code
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