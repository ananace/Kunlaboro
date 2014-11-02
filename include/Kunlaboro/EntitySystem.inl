#pragma once

#include "Component.hpp"


template<typename R, typename... Args>
void Kunlaboro::EntitySystem::registerGlobalMessage(Component* component, RequestId rid, const std::function<Optional<R>(Args...)>& callback)
{
    ComponentRegistered reg;
    reg.component = component;
    reg.functional = *reinterpret_cast<std::function<void()>*>(const_cast<std::function<Optional<R>(Args...)>*>(&callback));
    reg.type = &typeid(callback);
    reg.required = false;
    reg.priority = 0;

    //mGlobalMessageRequests[rid].push_back(std::move(reg));
    registerGlobalRequest(Kunlaboro::ComponentRequested { Kunlaboro::Reason_Message, "", rid }, reg);
}

template<typename... Args>
void Kunlaboro::EntitySystem::registerGlobalMessage(Component* component, RequestId rid, const std::function<void(Args...)>& callback)
{
    ComponentRegistered reg;
    reg.component = component;
    reg.functional = *reinterpret_cast<std::function<void()>*>(const_cast<std::function<void(Args...)>*>(&callback));
    reg.type = &typeid(callback);
    reg.required = false;
    reg.priority = 0;

    //mGlobalMessageRequests[rid].push_back(std::move(reg));
    registerGlobalRequest(Kunlaboro::ComponentRequested{ Kunlaboro::Reason_Message, "", rid }, reg);
}

template<typename R, typename... Args>
void Kunlaboro::EntitySystem::registerLocalMessage(Component* component, RequestId rid, const std::function<Optional<R>(Args...)>& callback)
{
    ComponentRegistered reg;
    reg.component = component;
    reg.functional = *reinterpret_cast<std::function<void()>*>(const_cast<std::function<Optional<R>(Args...)>*>(&callback));
    reg.type = &typeid(callback);
    reg.required = false;
    reg.priority = 0;

    //mEntities[component->getOwnerId()]->localMessageRequests[rid].push_back(reg);
    registerLocalRequest(Kunlaboro::ComponentRequested{ Kunlaboro::Reason_Message, "", rid }, reg);
}

template<typename... Args>
void Kunlaboro::EntitySystem::registerLocalMessage(Component* component, RequestId rid, const std::function<void(Args...)>& callback)
{
    ComponentRegistered reg;
    reg.component = component;
    reg.functional = *reinterpret_cast<std::function<void()>*>(const_cast<std::function<void(Args...)>*>(&callback));
    reg.type = &typeid(callback);
    reg.required = false;
    reg.priority = 0;

    //mEntities[component->getOwnerId()]->localMessageRequests[rid].push_back(reg);
    registerLocalRequest(Kunlaboro::ComponentRequested{ Kunlaboro::Reason_Message, "", rid }, reg);
}

template<typename R, typename... Args, typename std::enable_if<!std::is_void<R>::value, R>::type*>
Kunlaboro::Optional<R> Kunlaboro::EntitySystem::sendSafeGlobalMessage(RequestId id, Args...arguments)
{
    auto& reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        Optional<R> ret = (reinterpret_cast<std::function<Optional<R>(Args...)>*>(&it.functional))->operator()(arguments...);

        if (ret)
            return ret;
    }

    return nullptr;
}

template<typename R, typename... Args, typename std::enable_if<std::is_void<R>::value, R>::type*>
void Kunlaboro::EntitySystem::sendSafeGlobalMessage(RequestId id, Args...arguments)
{
    auto& reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        (reinterpret_cast<std::function<void(Args...)>*>(&it.functional))->operator()(arguments...);
    }
}

template<typename R, typename... Args, typename std::enable_if<!std::is_void<R>::value, R>::type*>
Kunlaboro::Optional<R> Kunlaboro::EntitySystem::sendUnsafeGlobalMessage(RequestId id, Args...arguments)
{
    if (mGlobalMessageRequests.count(id) == 0)
        return nullptr;

    auto reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        Optional<R> ret = (reinterpret_cast<std::function<Optional<R>(Args...)>*>(&it.functional))->operator()(arguments...);

        if (ret)
            return ret;
    }

    return nullptr;
}

template<typename R, typename... Args, typename std::enable_if<std::is_void<R>::value, R>::type*>
void Kunlaboro::EntitySystem::sendUnsafeGlobalMessage(RequestId id, Args...arguments)
{
    if (mGlobalMessageRequests.count(id) == 0)
        return;

    auto reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        (reinterpret_cast<std::function<void(Args...)>*>(&it.functional))->operator()(arguments...);
    }
}

template<typename R, typename... Args, typename std::enable_if<!std::is_void<R>::value, R>::type*>
Kunlaboro::Optional<R> Kunlaboro::EntitySystem::sendSafeLocalMessage(EntityId eid, RequestId id, Args...arguments)
{
    auto& reqs = mEntities[eid]->localMessageRequests[id];

    for (auto& it : reqs)
    {
        auto ret = (reinterpret_cast<std::function<Optional<R>(Args...)>*>(&it.functional))->operator()(arguments...);

        if (ret)
            return ret;
    }

    return nullptr;
}

template<typename R, typename... Args, typename std::enable_if<std::is_void<R>::value, R>::type*>
void Kunlaboro::EntitySystem::sendSafeLocalMessage(EntityId eid, RequestId id, Args...arguments)
{
    auto& reqs = mEntities[eid]->localMessageRequests[id];

    for (auto& it : reqs)
    {
        (reinterpret_cast<std::function<void(Args...)>*>(&it.functional))->operator()(arguments...);
    }
}

template<typename R, typename... Args, typename std::enable_if<!std::is_void<R>::value, R>::type*>
Kunlaboro::Optional<R> Kunlaboro::EntitySystem::sendUnsafeLocalMessage(EntityId eid, RequestId id, Args...arguments)
{
    if (mEntities[eid]->localMessageRequests.count(id) == 0)
        return nullptr;

    auto reqs = mEntities[eid]->localMessageRequests[id];

    for (auto& it : reqs)
    {
        auto ret = (reinterpret_cast<std::function<Optional<R>(Args...)>*>(&it.functional))->operator()(arguments...);

        if (ret)
            return ret;
    }

    return nullptr;
}

template<typename R, typename... Args, typename std::enable_if<std::is_void<R>::value, R>::type*>
void Kunlaboro::EntitySystem::sendUnsafeLocalMessage(EntityId eid, RequestId id, Args...arguments)
{
    if (mEntities[eid]->localMessageRequests.count(id) == 0)
        return;

    auto reqs = mEntities[eid]->localMessageRequests[id];

    for (auto& it : reqs)
    {
        (reinterpret_cast<std::function<void(Args...)>*>(&it.functional))->operator()(arguments...);
    }
}