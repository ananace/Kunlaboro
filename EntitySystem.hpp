#ifndef _KUNLABORO_ENTITYSYSTEM_HPP
#define _KUNLABORO_ENTITYSYSTEM_HPP

#include "Defines.hpp"
#ifdef Kunlaboro_BOOST
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#endif
#include <map>
#include <list>
#include <vector>

namespace Kunlaboro
{
    class Component;
    class Template;

    /** \brief A class containing an entire EntitySystem based on RDBMS.
     *
     * Only one instance of this class needs to exist at any given time unless you want
     * to have several different Entity Systems inside of the same application.
     */
    class EntitySystem
    {
    public:
        /// Constructs the EntitySystem.
        /// \param threaded Is the EntitySystem going to be used in threads?
        EntitySystem(bool threaded = false);
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
        /** \brief Creates an entity using a template.
         *
         * This function will create an entity and apply a template to it before returning it.
         *
         * \param templateName The name of the template to apply.
         * \returns The ID of the newly created entity.
         */
        EntityId createEntity(const std::string& templateName);
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
         * \returns If the entity was finalized successfully.
         */
        bool finalizeEntity(EntityId eid);

        /** \brief Register a factory for a specific component type.
         *
         * It is recommended to use this function for registering all the Component types you are
         * going to use during your applications life. Creating your own Components may lead to
         * problems inside the EntitySystem.
         *
         * \param name The name of the Component that the factory creates.
         * \param func A function that will create and return a Component* using the \b new keyword.
         */
        void registerComponent(const std::string& name, ComponentFactory func);
        /** \brief Register an automatic factory for a specific component type.
         *
         * This function will create a simple automated factory that just calls the default constructor
         * on the specified component, which works well for all component that doesn't need initializing.
         * If your components require any form of initializing then use registerComponent(const std::string&, FactoryFunction) instead.
         *
         * \param name The name of the Component that will be created.
         * \tparam T The component type you want to register.
         */
        template<class T>
        void registerComponent(const std::string& name);
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
        /** \brief Add a Component to an entity.
         *
         * This will add an orphaned Component into the specified entity, calling the Components
         * addedToEntity() function and sending out creation messages to all Components that have
         * registered a request for them.
         *
         * \param eid The entity to add the Component to.
         * \param name The name of the Component to add to the entity.
         */
        inline void addComponent(EntityId eid, const std::string& name)
        {
            addComponent(eid, createComponent(name));
        }
        /** \brief Remove a Component from an entity.
         * \warning This function has not been thoroughly tested and could possible break your application,
         * try to use it sparingly.
         * \todo Create a system like the frozen requests to handle safely removing the requests
         * from the component that's being removed, the system in use now will probably crash if
         * a component is removed in the middle of sending a message that the component in question
         * has requested.
         *
         * This function will remove a Component from the specified entity, sending out destruction
         * messages and preparing the Component for being inserted into another entity.
         *
         * \param eid The entity to remove the Component from.
         * \param c The Component to remove.
         */
        void removeComponent(EntityId eid, Component* c);

        /** \brief Returns a list of components on the specified entity.
         *
         * \param eid The entity to check.
         * \param name A specific type of component you're looking for.
         */
        std::vector<Component*> getAllComponentsOnEntity(EntityId eid, const std::string& name = "");

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

        /** \brief Removes a global request from the EntitySystem.
         *
         * This function will remove a given request from the global request queue.
         * \note Note that the arguments must be identical to an earlier call of the registerGlobalRequest()
         * function for this to succeed.
         *
         * \param req The Component request to remove.
         * \param reg The Callback registration to remove.
         */
        void removeGlobalRequest(const ComponentRequested& req, const ComponentRegistered& reg);
        /** \brief Remove a local request from the EntitySystem.
         *
         * This function will remove a local request from inside the entity that created the request.
         * \note Note that the arguments must be identical to an earlier call of the registerLocalRequest()
         * function for this to succeed.
         *
         * \param req The Component request to remove.
         * \param reg The Callback registration to remove.
         */
        void removeLocalRequest(const ComponentRequested& req, const ComponentRegistered& reg);

        /** \brief Changes the priority of the specific request.
         *
         * This function will change the priority of the request from the specified component,
         *
         * \param comp The component that owns the request.
         * \param rid The request to change.
         * \param priority The new priority of the request.
         */
        void reprioritizeRequest(Component* comp, RequestId rid, int priority);

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
        void sendGlobalMessage(RequestId id, Message& msg);
        /** \brief Send a local message to all the Component objects in the specified entity.
         *
         * This function will send a message to the specified entity and all the Component objects
         * contained inside of that entity,
         *
         * \param eid The EntityId to send the message to.
         * \param rid The RequestId to send.
         * \param msg The Message to send.
         */
        void sendLocalMessage(EntityId eid, RequestId rid, Message& msg);

        /** \brief Send a global message to all the Component objects in the EntitySystem.
         *
         * This is a convenience function for sending a message without having to look up the RequestId or
         * create a Message object, which means it should not be used repeatedly as it is slower than the
         * other sendGlobalMessage() function.
         *
         * \param n The name of the RequestId to send.
         * \param p The Payload to send.
         */
        inline void sendGlobalMessage(const std::string& n, const Payload& p = 0)
        {
            Message msg(Type_Message, NULL, p);
            sendGlobalMessage(getMessageRequestId(Reason_Message, n), msg);
        }

        /** \brief Freezes the EntitySystem, forcing all following modifications to be put on a queue.
         *
         * \param rid The RequestId that's going to be frozen
         */
        void freeze(RequestId rid);
        /** \brief Unfreezes the EntitySystem and lets it process all the queued modifications.
         *
         * \param rid The RequestId that's going to be unfrozen.
         */
        void unfreeze(RequestId rid);

        /** \brief Check if the EntitySystem is frozen
         *
         * \returns If the EntitySystem has any frozen requests.
         */
        inline bool isFrozen() { return mFrozen > 0; }
        /** \brief Check if the EntitySystem is frozen for the specified request.
         *
         * \param rid The RequestId you want to check.
         * \returns If the given RequestId is frozen.
         */
        inline bool isFrozen(RequestId rid) { return mFrozenData.frozenRequests[rid].locked; }

    private:
        /// A helper struct for containing Entity specific information.
        struct Entity
        {
            EntityId id; ///< The EntityId of the Entity.
            bool finalised; ///< If the Entity is finalized.
            ComponentMap components; ///< The Component objects stored in this Entity.
            RequestMap localRequests; ///< The local requests stored inside the Entity.
        };

        /// A container for all the requests that happened during a time when the EntitySystem was frozen.
        struct FrozenData
        {
            /// A container for a locked request.
            struct RequestLock
            {
                bool locked; ///< Is the request locked or not
#ifdef Kunlaboro_BOOST
                boost::mutex mutex; ///< A mutex to allow for threaded access.
                boost::thread::id owner; ///< The thread that owns the lock on the request
#endif
                std::list<std::pair<Component*, std::pair<RequestId, int> > > repriorities; ///< A list of all the locked calls to reprioritize requests.
                std::list<std::pair<ComponentRequested, ComponentRegistered>> localRequests; ///< A list of all the locked calls to register local requests.
                std::list<std::pair<ComponentRequested, ComponentRegistered>> localRequestRemoves; ///< A list of all the locked calls to remove local requests.
                std::list<std::pair<ComponentRequested, ComponentRegistered>> globalRequests; ///< A list of all the locked calls to register global requests.
                std::list<std::pair<ComponentRequested, ComponentRegistered>> globalRequestRemoves; ///< A list of all the locked calls to remove global requests.
                RequestLock() : locked(false) { } ///< The standard constructor
                ~RequestLock() {
#ifdef Kunlaboro_BOOST
                    mutex.destroy();
#endif
                }
                RequestLock(const RequestLock&);
                RequestLock& operator=(const RequestLock&);
            };

            /// A map of all the frozen requests that have been made.
            std::unordered_map<RequestId, RequestLock> frozenRequests;

            /// Component destructions that were frozen.
            std::list<Component*> frozenComponentDestructions;
            /// Entity destructions that were frozen.
            std::list<EntityId> frozenEntityDestructions;

            /// Have any requests been frozen and need processing when the system is unfrozen?
            bool needsProcessing;
        } mFrozenData;

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

        std::unordered_map<std::string, ComponentFactory> mRegisteredComponents; ///< Registered Components in the EntitySystem
        std::unordered_map<std::string, TemplateFactory> mRegisteredTemplates; ///< Registered Templates in the EntitySystem
        std::unordered_map<EntityId, std::vector<std::string> > mRequiredComponents; ///< Required Components in entities.
        RequestMap mGlobalRequests; ///< Globally registered requests.
        std::unordered_map<ComponentId, std::vector<ComponentRequested> > mRequestsByComponent; ///< Requests by component.
        std::unordered_map<EntityId,Entity*> mEntities; ///< List of created entities.

        bool mThreaded; ///< Should this EntitySystem allow for threading?
        int mFrozen; ///< Is the EntitySystem frozen.
    };

    template<class T>
    void EntitySystem::registerComponent(const std::string& name)
    {
        registerComponent(name, [](){ return new T(); });
    }
}

#endif

/** \class Kunlaboro::EntitySystem
*
* Usage example: (Extends the code from the Component usage example)
* \code
* #include <Kunlaboro/Kunlaboro.hpp>
* #include "CalculatorComponent.hpp"
*
* int main()
* {
*     Kunlaboro::EntitySystem entity_system;
*     entity_system.registerComponent<CalculatorComponent>("Calculator");
*
*     Kunlaboro::EntityId calc = entity_system.createEntity();
*     entity_system.addComponent(calc, entity_system.createComponent("Calculator"));
*     if (!entity_system.finalizeEntity(calc)) // This call is only really needed if the component requires another component to work.
*         return 1; // The entity didn't get all of the components that were required for it.
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
