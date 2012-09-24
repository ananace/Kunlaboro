#ifndef _KUNLABORO_COMPONENT_HPP
#define _KUNLABORO_COMPONENT_HPP

#include "Defines.hpp"
#include <functional>
#include <string>

namespace Kunlaboro
{
    class EntitySystem;

    class Component
    {
    public:
        virtual ~Component();

        /// The function that is called when the component is added to an entity
        virtual void addedToEntity();

        void addLocalComponent(Component*);
        
        void requestMessage(const std::string&, MessageFunction) const;
        void requestComponent(const std::string&, MessageFunction, bool local = false) const;
        void requireComponent(const std::string&, MessageFunction, bool local = true) const;

        RequestId getMessageRequestId(const std::string&) const;

        void sendMessage(RequestId id) const;
        void sendMessage(RequestId id, const Payload&) const;
        void sendMessage(RequestId id, const Message&) const;
        void sendGlobalMessage(RequestId id) const;
        void sendGlobalMessage(RequestId id, const Payload&) const;
        void sendGlobalMessage(RequestId id, const Message&) const;
        void sendMessageToEntity(EntityId, RequestId) const;
        void sendMessageToEntity(EntityId, RequestId, const Payload&) const;
        void sendMessageToEntity(EntityId, RequestId, const Message&) const;

        void sendGlobalMessage(const std::string& id, const Payload& p) const;
        inline EntitySystem* getEntitySystem() const { return mEntitySystem; }

        ComponentId getId() const;
        EntityId getOwnerId() const;

        void destroy();
        bool isDestroyed() const;

        bool isValid() const;

        const std::string& getName() const;
        std::string toString() const;

        template<class T>
        void requestMessage(const std::string& name, void (T::*f)(const Message&)) const;
        template<class T>
        void requestComponent(const std::string&, void (T::*f)(const Message&), bool local = false) const;
        template<class T>
        void requireComponent(const std::string&, void (T::*f)(const Message&), bool local = true) const;

    protected:
        /// Constructor for the component
        Component(const std::string&);

    private:
        void setOwner(EntityId);
        void setDestroyed();

        EntityId mOwner;
        EntitySystem* mEntitySystem;
        ComponentId mId;

        std::string mName;
        bool mDestroyed;

        friend class EntitySystem;
    };


    template<class T>
    void Component::requestMessage(const std::string& name, void (T::*f)(const Message&)) const
    {
        requestMessage(name, std::bind(f, (T*)(this), std::tr1::placeholders::_1));
    }

    template<class T>
    void Component::requestComponent(const std::string& name, void (T::*f)(const Message&), bool local) const
    {
        requestComponent(name, std::bind(f, (T*)(this), std::tr1::placeholders::_1), local);
    }

    template<class T>
    void Component::requireComponent(const std::string& name, void (T::*f)(const Message&), bool local) const
    {
        requireComponent(name, std::bind(f, (T*)(this), std::tr1::placeholders::_1), local);
    }
}

std::ostream& operator<<(std::ostream& os, const Kunlaboro::Component& c);

#endif // _KUNLABORO_COMPONENT_HPP