#pragma once

#include <Kunlaboro/Defines.hpp>

#include <unordered_map>
#include <vector>
#include <mutex>
#include <list>
#include <map>

namespace Kunlaboro
{
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
         * This function will create an entity and apply a template to it before returning it, the
         * function will also finalize the entity to ensure that it's properly created.
         *
         * \param templateName The name of the template to apply.
         * \returns The ID of the newly created entity or 0 if it failed to finalize.
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

        /** \brief Register an entity template.
         *
         * An entity template is a set of components that will automatically be added to an entity
         * when it's created using that template.
         *
         * \param name The name of the entity template
         * \param components The components that the entity will be created with.
         * \sa createEntity(templateName)
         */
        void registerTemplate(const std::string& name, const std::vector<std::string>& components);
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
         * \ingroup convenience
         *
         * This will create and add a Component into the specified entity, calling the Components
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

        template<typename R, typename... Args>
        void registerGlobalMessage(Component* component, RequestId rid, const std::function<Optional<R>(Args...)>& callback);

        template<typename... Args>
        void registerGlobalMessage(Component* component, RequestId rid, const std::function<void(Args...)>& callback);

        template<typename R, typename... Args>
        void registerLocalMessage(Component* component, RequestId rid, const std::function<Optional<R>(Args...)>& callback);

        template<typename... Args>
        void registerLocalMessage(Component* component, RequestId rid, const std::function<void(Args...)>& callback);

        void reprioritizeGlobalMessage(const Component& component, RequestId rid, int priority);
        void reprioritizeLocalMessage(const Component& component, RequestId rid, int priority);
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

        /** \brief Send a global message to all the Component objects in the EntitySystem.
         *
         * This function will send a message to all the Component objects that have been registered
         * anywhere inside the EntitySystem.
         *
         * \param id The RequestId to send.
         * \param msg The Message to send.
         */
        template<typename R, typename... Args, typename std::enable_if<!std::is_void<R>::value, R>::type* = nullptr>
        inline Optional<R> sendSafeGlobalMessage(RequestId id, Args... arguments);

        template<typename R, typename... Args, typename std::enable_if<std::is_void<R>::value, R>::type* = nullptr>
        inline void sendSafeGlobalMessage(RequestId id, Args... arguments);

        template<typename R, typename... Args, typename std::enable_if<!std::is_void<R>::value, R>::type* = nullptr>
        inline Optional<R> sendUnsafeGlobalMessage(RequestId id, Args... arguments);

        template<typename R, typename... Args, typename std::enable_if<std::is_void<R>::value, R>::type* = nullptr>
        inline void sendUnsafeGlobalMessage(RequestId id, Args... arguments);

        template<typename R, typename... Args, typename std::enable_if<!std::is_void<R>::value, R>::type* = nullptr>
        inline Optional<R> sendSafeLocalMessage(EntityId eid, RequestId id, Args... arguments);

        template<typename R, typename... Args, typename std::enable_if<std::is_void<R>::value, R>::type* = nullptr>
        inline void sendSafeLocalMessage(EntityId eid, RequestId id, Args... arguments);

        template<typename R, typename... Args, typename std::enable_if<!std::is_void<R>::value, R>::type* = nullptr>
        inline Optional<R> sendUnsafeLocalMessage(EntityId eid, RequestId id, Args... arguments);

        template<typename R, typename... Args, typename std::enable_if<std::is_void<R>::value, R>::type* = nullptr>
        inline void sendUnsafeLocalMessage(EntityId eid, RequestId id, Args... arguments);

        ///@}

        /** \brief Check if a specified entity is valid
         * 
         * \param eid The EntityId you want to check.
         * \returns If the Entity exists in the Entity System.
         */
        inline bool isValid(Kunlaboro::EntityId eid) { if (eid == 0) return false; return mEntities[eid] != NULL; }

        /// The number of active Entities in the Entity System.
        inline unsigned int numEnt() { return mEntityC; }
        /// The number of active Components in the Entity System.
        inline unsigned int numCom() { return mComponentC; }

    private:

        /// A helper struct containing Entity specific information.
        struct Entity
        {
            EntityId id; ///< The EntityId of the Entity.
            bool finalised; ///< If the Entity is finalised.
            ComponentMap components; ///< The Component objects stored in this Entity.

            RequestMap localComponentRequests; ///< The local requests the Entity listens for.
            RequestMap localMessageRequests; ///< The local requests the Entity listens for.
        };

        ComponentId mComponentCounter; ///< The Component counter.
        EntityId mEntityCounter; ///< The Entity counter.

        std::unordered_map<std::string, ComponentFactory> mRegisteredComponents; ///< Registered Components in the EntitySystem
        std::unordered_map<std::string, std::vector<std::string>> mRegisteredTemplates; ///< Registered Templates in the EntitySystem
        std::unordered_map<EntityId, std::vector<std::string> > mRequiredComponents; ///< Required Components in entities.
        
        RequestMap mGlobalMessageRequests; ///< Globally registered requests.
        RequestMap mGlobalComponentRequests; ///< Globally registered requests.

        std::unordered_map<ComponentId, std::vector<ComponentRequested> > mRequestsByComponent; ///< Requests by component.
        std::unordered_map<EntityId,Entity*> mEntities; ///< List of created entities.

        bool mThreaded; ///< Should this EntitySystem allow for threading?

        unsigned int mEntityC; ///< Entity counter
        unsigned int mComponentC; ///< Component counter

        friend class Component;
    };

    template<class T>
    void EntitySystem::registerComponent(const std::string& name)
    {
        registerComponent(name, [](){ return new T(); });
    }
}

#include "EntitySystem.inl"

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
*         return 1; // The entity couldn't get all of the components that were needed.
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
