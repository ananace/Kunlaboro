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

template<typename R, typename... Args>
Kunlaboro::Optional<R> Kunlaboro::EntitySystem::sendSafeGlobalMessage(RequestId id, Args... arguments)
{
    auto& reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        if (typeid(std::function<Optional<R>(Args...)>) != *it.type)
            continue;

        Optional<R> ret = (*reinterpret_cast<std::function<Optional<R>(Args...)>*>(&it.callback))(arguments...);

        if (ret)
            return ret;
    }

    return nullptr;
}

template<typename... Args>
void Kunlaboro::EntitySystem::sendSafeGlobalMessage(RequestId id, Args... arguments)
{
    auto& reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        if (typeid(std::function<void(Args...)>) != *it.type)
            continue;

        (*reinterpret_cast<std::function<void(Args...)>*>(&it.callback))(arguments...);
    }
}

template<typename R, typename... Args>
Kunlaboro::Optional<R> Kunlaboro::EntitySystem::sendUnsafeGlobalMessage(RequestId id, Args... arguments)
{
    auto reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        if (typeid(std::function<Optional<R>(Args...)>) != *it.type)
            continue;

        Optional<R> ret = (*reinterpret_cast<std::function<Optional<R>(Args...)>*>(&it.callback))(arguments...);

        if (ret)
            return ret;
    }

    return nullptr;
}

template<typename... Args>
void Kunlaboro::EntitySystem::sendUnsafeGlobalMessage(RequestId id, Args... arguments)
{
    auto reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        if (typeid(std::function<void(Args...)>) != *it.type)
            continue;

        (*reinterpret_cast<std::function<void(Args...)>*>(&it.callback))(arguments...);
    }
}

template<typename R, typename... Args>
Kunlaboro::Optional<R> Kunlaboro::EntitySystem::sendSafeLocalMessage(EntityId eid, RequestId id, Args... arguments)
{
    auto& reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        if (typeid(std::function<Optional<R>(Args...)>) != *it.type)
            continue;

        auto ret = (*reinterpret_cast<std::function<Optional<R>(Args...)>*>(&it.callback))(arguments...);

        if (ret)
            return ret;
    }

    return nullptr;
}

template<typename... Args>
void Kunlaboro::EntitySystem::sendSafeLocalMessage(EntityId eid, RequestId id, Args... arguments)
{
    auto& reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        if (typeid(std::function<void(Args...)>) != *it.type)
            continue;

        (*reinterpret_cast<std::function<void(Args...)>*>(&it.callback))(arguments...);
    }
}

template<typename R, typename... Args>
Kunlaboro::Optional<R> Kunlaboro::EntitySystem::sendUnsafeLocalMessage(EntityId eid, RequestId id, Args... arguments)
{
    auto reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        if (typeid(std::function<Optional<R>(Args...)>) != *it.type)
            continue;

        auto ret = (*reinterpret_cast<std::function<Optional<R>(Args...)>*>(&it.callback))(arguments...);

        if (ret)
            return ret;
    }

    return nullptr;
}

template<typename... Args>
void Kunlaboro::EntitySystem::sendUnsafeLocalMessage(EntityId eid, RequestId id, Args... arguments)
{
    auto reqs = mGlobalMessageRequests[id];

    for (auto& it : reqs)
    {
        if (typeid(std::function<void(Args...)>) != *it.type)
            continue;

        (*reinterpret_cast<std::function<void(Args...)>*>(&it.callback))(arguments...);
    }
}