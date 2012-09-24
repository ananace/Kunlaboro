#ifndef _KUNLABORO_ENTITYSYSTEM_HPP
#define _KUNLABORO_ENTITYSYSTEM_HPP

#include "Defines.hpp"
#include <map>

namespace Kunlaboro
{
    class Component;

    class EntitySystem
    {
    public:
        EntitySystem();
        ~EntitySystem();

        EntityId createEntity();
        void destroyEntity(EntityId);
        void finalizeEntity(EntityId);

        void registerComponent(const std::string&, FactoryFunction);
        Component* createComponent(const std::string&);
        void destroyComponent(Component*);

        void addComponent(EntityId, Component*);
        void removeComponent(EntityId, Component*);

        void registerGlobalRequest(const ComponentRequested&, const ComponentRegistered&);
        void registerLocalRequest(const ComponentRequested&, const ComponentRegistered&);

        RequestId getMessageRequestId(MessageReason, const std::string&);

        void sendGlobalMessage(RequestId, const Message&);
        void sendLocalMessage(EntityId, RequestId, const Message&);

        inline void sendGlobalMessage(const std::string& n, const Payload& p = 0)
        {
            sendGlobalMessage(getMessageRequestId(Reason_Message, n), Message(Type_Message, NULL, p));
        }

    private:
        struct Entity
        {
            EntityId id;
            bool finalised;
            ComponentMap components;
            std::unordered_map<RequestId, std::vector<ComponentRegistered> > localRequests;
        };

        RequestId getExistingRequestId(MessageReason, const std::string&);

        ComponentId mComponentCounter;
        RequestId mRequestCounter;
        EntityId mEntityCounter;

        NameToIdMap mNameMap[2];
        IdToNameMap mIdMap[2];

        std::unordered_map<std::string, FactoryFunction> mRegisteredComponents;
        std::unordered_map<EntityId, std::vector<std::string> > mRequiredComponents;
        std::unordered_map<EntityId, std::vector<ComponentRegistered> > mGlobalRequests;
        std::unordered_map<ComponentId, std::vector<ComponentRequested> > mRequestsByComponent;
        std::vector<Entity*> mEntities;
    };
}

#endif