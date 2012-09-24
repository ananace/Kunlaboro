/*  Kunlaboro - EntitySystem

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

#ifndef _KUNLABORO_ENTITYSYSTEM_HPP
#define _KUNLABORO_ENTITYSYSTEM_HPP

#include "Defines.hpp"
#include <map>

namespace Kunlaboro
{
    class Component;

    /** \brief A class containing an entire Entity System
     *
     * Only one instance of this class needs to exist at any given time unless you want
     * to have several different Entity Systems inside of the same application.
     */
    class EntitySystem
    {
    public:
        /// Constructs the EntitySystem.
        EntitySystem();
        /// Destroys the EntitySystem.
        ~EntitySystem();

        /** \brief Create an entity and return its GUID
         *
         * This function will create an entity inside of the EntitySystem and return a EntityId that
         * you can use to access the entity.
         *
         * \returns The EntityId of the newly created entity.
         */
        EntityId createEntity();
        /** \brief Destroys a previously created entity.
         *
         * This function will destroy an entity created by the EntitySystem, cleaning up and removing
         * any Components that the entity contains as well as all registered requests.
         *
         * \param eid The EntityId to destroy, must be a valid entity.
         */
        void destroyEntity(EntityId eid);
        /** \brief Finalize an entity.
         *
         * This function will finalize the specified entity, making sure that the entity contains
         * all the required Components and is ready to be used inside of the application.
         * Entities can be used without being finalized, but they are not required to be complete
         * until the finalizeEntitity() function has been called without destroying the entity.
         *
         * \param eid The EntityId to finalize.
         */
        void finalizeEntity(EntityId eid);

        /** \brief Register a factory for a specific component type.
         *
         * It is recommended to use this function for registering all the Component types you are
         * going to use during your applications life. Creating your own Components may lead to
         * problems inside the EntitySystem.
         *
         * \param name The name of the Component that the factory creates.
         * \param func Function that will create and return a Component* using new.
         */
        void registerComponent(const std::string& name, FactoryFunction func);
        /** \brief Create an instance of a Component.
         *
         * This function will use the FactoryFunction that was registered through registerComponent()
         * to create a new instance of the specified Component and set it up for usage inside of the
         * EntitySystem.
         *
         * \param name The name of the Component to create.
         * \returns A pointer to the newly created Component.
         */
        Component* createComponent(const std::string& name);
        /** \brief Destroy a previously created Component.
         *
         * This function will completely destroy a Component, removing all of that components registered
         * requests and sending out a Message to all Components that have registered a request for
         * the Component in question.
         *
         * \param c The Component to destroy.
         */
        void destroyComponent(Component* c);

        /** \brief Add a Component to an entity.
         *
         * This will add an orphaned Component into the specified entity, calling the Components
         * addedToEntity() function and sending out creation messages to all Components that have
         * registered a request for them.
         *
         * \param eid The entity to add the Component to.
         * \param c The Component to add to the entity.
         */
        void addComponent(EntityId eid, Component* c);
        /** \brief Remove a Component from an entity.
         * \warning Not really tested, at all. Use at own risk.
         *
         * This function will remove a Component from the specified entity, sending out destruction
         * messages and preparing the Component for being inserted into another entity.
         *
         * \param eid The entity to remove the Component from.
         * \param c The Component to remove.
         */
        void removeComponent(EntityId eid, Component* c);

        /** \brief Register a global request into the EntitySystem.
         *
         * This function will register a given request into the global request queue, any messages
         * that are sent globally in the EntitySystem will be forwarded if they match the request.
         *
         * \param req The Component request to register.
         * \param reg The Callback registration to store.
         */
        void registerGlobalRequest(const ComponentRequested& req, const ComponentRegistered& reg);
        /** \brief Register a local request into the EntitySystem.
         *
         * This function will register a local request inside the entity that created the request.
         * This will only catch messages that are sent to that entity specifically, global message
         * will not be caught by local requests.
         *
         * \param req The Component request to register.
         * \param reg The Callback registration to store.
         */
        void registerLocalRequest(const ComponentRequested& req, const ComponentRegistered& reg);

        /** \brief Get a RequestId for the specified reason and string.
         *
         * This will return the RequestId for the specified message, you can use such a RequestId
         * to speed up the calling of message sending.
         *
         * \param reason The reason for the request.
         * \param name The name of the request.
         */
        RequestId getMessageRequestId(MessageReason reason, const std::string& name);

        /** \brief Send a global message to all the Component objects in the EntitySystem.
         *
         * This function will send a message to all the Component objects that have been registered
         * anywhere inside the EntitySystem.
         *
         * \param id The RequestId to send.
         * \param msg The Message to send.
         */
        void sendGlobalMessage(RequestId id, const Message& msg);
        /** \brief Send a local message to all the Component objects in the specified entity.
         *
         * This function will send a message to the specified entity and all the Component objects
         * contained inside of that entity, 
         *
         * \param eid The EntityId to send the message to.
         * \param rid The RequestId to send.
         * \param msg The Message to send.
         */
        void sendLocalMessage(EntityId eid, RequestId rid, const Message& msg);

        /** \brief Send a global message to all the Component objects in the EntitySystem.
         *
         * This is a convenience function for sending a message without having to look up the RequestId or
         * create a Message object, which means it will run much slower than the other sendGlobalMessage() function.
         *
         * \param n The name of the RequestId to send.
         * \param p The Payload to send.
         */
        inline void sendGlobalMessage(const std::string& n, const Payload& p = 0)
        {
            sendGlobalMessage(getMessageRequestId(Reason_Message, n), Message(Type_Message, NULL, p));
        }

    private:
        /// A helper struct for containing Entity specific information.
        struct Entity
        {
            EntityId id; ///< The EntityId of the Entity.
            bool finalised; ///< If the Entity is finalized.
            ComponentMap components; ///< The Component objects stored in this Entity. 
            std::unordered_map<RequestId, std::vector<ComponentRegistered> > localRequests; ///< The local requests stored inside the Entity.
        };

        /** \brief Get an existing RequestId for a message.
         *
         * If a RequestId does not exist for the specified reason and name, then this function will return 0.
         * 
         * \param reason The reason for the request.
         * \param name The name of the request.
         * \returns The RequestId of the existing request.
         */
        RequestId getExistingRequestId(MessageReason reason, const std::string& name);

        ComponentId mComponentCounter; ///< The Component counter.
        RequestId mRequestCounter; ///< The Request counter.
        EntityId mEntityCounter; ///< The Entity counter.

        NameToIdMap mNameMap[2]; ///< Maps for getting GUIDs from names.
        IdToNameMap mIdMap[2]; ///< Maps for converting GUIDs back to names.

        std::unordered_map<std::string, FactoryFunction> mRegisteredComponents; ///< Registered Components in the EntitySystem
        std::unordered_map<EntityId, std::vector<std::string> > mRequiredComponents; ///< Required Components in entities.
        std::unordered_map<EntityId, std::vector<ComponentRegistered> > mGlobalRequests; ///< Globally registered requests.
        std::unordered_map<ComponentId, std::vector<ComponentRequested> > mRequestsByComponent; ///< Requests by component.
        std::vector<Entity*> mEntities; ///< List of created entities.
    };
}

#endif

/** \class Kunlaboro::EntitySystem
*
* Usage example: (Extends the code from the Component usage example)
* \code
* #include <Kunlaboro/Kunlaboro.hpp>
* #include "CalculatorComponent.hpp"
* 
* Component* createCalculator()
* {
*     return new CalculatorComponent();
* }
*
* int main()
* {
*     Kunlaboro::EntitySystem entity_system;
*     entity_system.registerComponent("Calculator", &createCalculator);
*     
*     Kunlaboro::EntityId calc = entity_system.createEntity();
*     entity_system.addComponent(calc, entity_system.createComponent("Calculator"));
*     entity_system.finalizeEntity(calc);
*
*     entity_system.sendGlobalMessage("Calculator.Add", 500);
*     entity_system.sendGlobalMessage("Calculator.Sub", 460);
*     entity_system.sendGlobalMessage("Calculator.Add", 2);
*     entity_system.sendGlobalMessage("Calculator.Print");
*
*     return 0;
* }
* \endcode
*/