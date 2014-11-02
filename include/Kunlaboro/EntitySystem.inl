#pragma once

#include "Component.hpp"


template<typename R, typename... Args>
void Kunlaboro::EntitySystem::registerGlobalMessage(Component* component, RequestId rid, const std::function<Optional<R>(Args...)>& callback)
{
    ComponentRegistered reg = { component, *reinterpret_cast<std::function<void()>*>(const_cast<std::function<Optional<R>(Args...)>*>(&callback)), &typeid(callback), false, 0 };

    mGlobalMessageRequests[rid].push_back(std::move(reg));
}

template<typename... Args>
void Kunlaboro::EntitySystem::registerGlobalMessage(Component* component, RequestId rid, const std::function<void(Args...)>& callback)
{
    ComponentRegistered reg = { component, *reinterpret_cast<std::function<void()>*>(const_cast<std::function<void(Args...)>*>(&callback)), &typeid(callback), false, 0 };

    mGlobalMessageRequests[rid].push_back(std::move(reg));
}

template<typename R, typename... Args>
void Kunlaboro::EntitySystem::registerLocalMessage(Component* component, RequestId rid, const std::function<Optional<R>(Args...)>& callback)
{
    ComponentRegistered reg = { component, *reinterpret_cast<std::function<void()>*>(const_cast<std::function<Optional<R>(Args...)>*>(&callback)), &typeid(callback), false, 0 };

    mEntities[component->getOwnerId()]->localMessageRequests[rid].push_back(reg);
}

template<typename... Args>
void Kunlaboro::EntitySystem::registerLocalMessage(Component* component, RequestId rid, const std::function<void(Args...)>& callback)
{
    ComponentRegistered reg = { component, *reinterpret_cast<std::function<void()>*>(const_cast<std::function<void(Args...)>*>(&callback)), &typeid(callback), false, 0 };

    mEntities[component->getOwnerId()]->localMessageRequests[rid].push_back(reg);
}

template<typename R, typename... Args, typename std::enable_if<!std::is_void<R>::value, R>::type*>
Kunlaboro::Optional<R> Kunlaboro::EntitySystem::sendSafeGlobalMessage(RequestId id, Args...arguments)
{
    auto& reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        Optional<R> ret = (*reinterpret_cast<std::function<Optional<R>(Args...)>*>(&it.callback))(std::forward<Args>(arguments)...);

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
        (*reinterpret_cast<std::function<void(Args...)>*>(&it.callback))(std::forward<Args>(arguments)...);
    }
}

template<typename R, typename... Args, typename std::enable_if<!std::is_void<R>::value, R>::type*>
Kunlaboro::Optional<R> Kunlaboro::EntitySystem::sendUnsafeGlobalMessage(RequestId id, Args...arguments)
{
    auto reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        Optional<R> ret = (*reinterpret_cast<std::function<Optional<R>(Args...)>*>(&it.callback))(std::forward<Args>(arguments)...);

        if (ret)
            return ret;
    }

    return nullptr;
}

template<typename R, typename... Args, typename std::enable_if<std::is_void<R>::value, R>::type*>
void Kunlaboro::EntitySystem::sendUnsafeGlobalMessage(RequestId id, Args...arguments)
{
    auto reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        (*reinterpret_cast<std::function<void(Args...)>*>(&it.callback))(std::forward<Args>(arguments)...);
    }
}

template<typename R, typename... Args, typename std::enable_if<!std::is_void<R>::value, R>::type*>
Kunlaboro::Optional<R> Kunlaboro::EntitySystem::sendSafeLocalMessage(EntityId eid, RequestId id, Args...arguments)
{
    auto& reqs = mEntities[eid]->localMessageRequests[id];

    for (auto& it : reqs)
    {
        auto ret = (*reinterpret_cast<std::function<Optional<R>(Args...)>*>(&it.callback))(std::forward<Args>(arguments)...);

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
        (*reinterpret_cast<std::function<void(Args...)>*>(&it.callback))(std::forward<Args>(arguments)...);
    }
}

template<typename R, typename... Args, typename std::enable_if<!std::is_void<R>::value, R>::type*>
Kunlaboro::Optional<R> Kunlaboro::EntitySystem::sendUnsafeLocalMessage(EntityId eid, RequestId id, Args...arguments)
{
    auto reqs = mEntities[eid]->localMessageRequests[id];

    for (auto& it : reqs)
    {
        auto ret = (*reinterpret_cast<std::function<Optional<R>(Args...)>*>(&it.callback))(std::forward<Args>(arguments)...);

        if (ret)
            return ret;
    }

    return nullptr;
}

template<typename R, typename... Args, typename std::enable_if<std::is_void<R>::value, R>::type*>
void Kunlaboro::EntitySystem::sendUnsafeLocalMessage(EntityId eid, RequestId id, Args...arguments)
{
    auto reqs = mEntities[eid]->localMessageRequests[id];

    for (auto& it : reqs)
    {
        (*reinterpret_cast<std::function<void(Args...)>*>(&it.callback))(std::forward<Args>(arguments)...);
    }
}