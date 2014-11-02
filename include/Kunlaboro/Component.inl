#pragma once

#include "EntitySystem.hpp"

#include <functional>
#include <type_traits>

namespace
{
    template<int...> struct int_sequence {};

    template<int N, int... Is> struct make_int_sequence
        : make_int_sequence < N - 1, N - 1, Is... >
    {
    };
    template<int... Is> struct make_int_sequence < 0, Is... >
        : int_sequence < Is... >
        {
        };

        template<int> // begin with 0 here!
        struct placeholder_template
        {};
}

namespace std
{
    template<int N>
    struct is_placeholder< placeholder_template<N> >
        : integral_constant<int, N + 1> // the one is important
    {};
}

namespace
{
    template<typename Ret, typename... Params>
    struct MessageRequestBinder
    {
        MessageRequestBinder(Kunlaboro::EntitySystem* es, Kunlaboro::RequestId rid, bool local) : mES(es), mRID(rid), mLocal(local) { }

        template<typename CLASS, int... Is>
        void RequestMessage(CLASS* object_ref,
            Ret(CLASS::*member_function)(Params...),
            int_sequence<Is...>)
        {
            auto func = std::bind(member_function,
                std::ref(*object_ref),
                placeholder_template < Is > {}...
            );

            if (mLocal)
                mES->registerLocalMessage(object_ref, mRID, std::function<Ret(Params...)>(func));
            else
                mES->registerGlobalMessage(object_ref, mRID, std::function<Ret(Params...)>(func));
        }

        template<typename CLASS, int... Is>
        void RequestMessage(CLASS* object_ref,
            Ret(CLASS::*member_function)(Params...) const,
            int_sequence<Is...>)
        {
            auto func = std::bind(member_function,
                std::ref(*object_ref),
                placeholder_template < Is > {}...
                );

            if (mLocal)
                mES->registerLocalMessage(object_ref, mRID, std::function<Ret(Params...)>(func));
            else
                mES->registerGlobalMessage(object_ref, mRID, std::function<Ret(Params...)>(func));
        }

    private:
        Kunlaboro::EntitySystem* mES;
        Kunlaboro::RequestId mRID;
        bool mLocal;
    };
}

template<typename R, typename std::enable_if<!std::is_void<R>::value, R>::type*, typename... Args>
Kunlaboro::Optional<R> Kunlaboro::Component::sendMessage(RequestId id, Args... args) const
{
    return getEntitySystem()->sendUnsafeLocalMessage<R>(mOwner, id, args...);
}

template<typename R, typename std::enable_if<std::is_void<R>::value, R>::type*, typename... Args>
void Kunlaboro::Component::sendMessage(RequestId id, Args... args) const
{
    getEntitySystem()->sendUnsafeLocalMessage<void>(mOwner, id, args...);
}

template<typename R, typename std::enable_if<!std::is_void<R>::value, R>::type*, typename... Args>
Kunlaboro::Optional<R> Kunlaboro::Component::sendGlobalMessage(RequestId id, Args... args) const
{
    return getEntitySystem()->sendUnsafeGlobalMessage<R>(id, args...);
}

template<typename R, typename std::enable_if<std::is_void<R>::value, R>::type*, typename... Args>
void Kunlaboro::Component::sendGlobalMessage(RequestId id, Args... args) const
{
    getEntitySystem()->sendUnsafeGlobalMessage<void>(id, args...);
}

template<typename R, typename std::enable_if<!std::is_void<R>::value, R>::type*, typename... Args>
Kunlaboro::Optional<R> Kunlaboro::Component::sendMessageToEntity(EntityId eid, RequestId id, Args... args) const
{
    return getEntitySystem()->sendUnsafeLocalMessage<R>(eid, id, args...);
}

template<typename R, typename std::enable_if<std::is_void<R>::value, R>::type*, typename... Args>
void Kunlaboro::Component::sendMessageToEntity(EntityId eid, RequestId id, Args... args) const
{
    return getEntitySystem()->sendUnsafeLocalMessage<void>(eid, id, args...);
}

template<typename R, typename... Args>
void Kunlaboro::Component::requestMessage(RequestId rid, const std::function<R(Args...)>& func, bool local) const
{
    if (local)
        getEntitySystem()->registerLocalMessage(const_cast<Component*>(this), rid, func);
    else
        getEntitySystem()->registerGlobalMessage(const_cast<Component*>(this), rid, func);
}

template<class T, class R, typename... Args>
void Kunlaboro::Component::requestMessage(const std::string& name, R(T::*f)(Args...), bool local)
{
    MessageRequestBinder<R, Args...> binder(mEntitySystem, hash::hashString(name), local);

    binder.RequestMessage((T*)this, f, make_int_sequence<sizeof...(Args)>{});
}

template<class T, class R, typename... Args>
void Kunlaboro::Component::requestMessage(const std::string& name, R(T::*f)(Args...) const, bool local)
{
    MessageRequestBinder<R, Args...> binder(mEntitySystem, hash::hashString(name), local);

    binder.RequestMessage((T*)this, f, make_int_sequence<sizeof...(Args)>{});
}

template<class T>
void Kunlaboro::Component::requestComponent(const std::string& name, void (T::*f)(Component*, MessageType), bool local)
{
    requestComponent(hash::hashString(name), std::bind(f, std::ref(*this), std::placeholders::_1, std::placeholders::_2), local);
}

template<class T>
void Kunlaboro::Component::requestComponent(const std::string& name, void (T::*f)(Component*, MessageType) const, bool local)
{
    requestComponent(hash::hashString(name), std::bind(f, std::ref(*this), std::placeholders::_1, std::placeholders::_2), local);
}

template<class T>
void Kunlaboro::Component::requireComponent(const std::string& name, void (T::*f)(Component*, MessageType), bool local)
{
    requireComponent(hash::hashString(name), std::bind(f, std::ref(*this), std::placeholders::_1, std::placeholders::_2), local);
}

template<class T>
void Kunlaboro::Component::requireComponent(const std::string& name, void (T::*f)(Component*, MessageType) const, bool local)
{
    requireComponent(hash::hashString(name), std::bind(f, std::ref(*this), std::placeholders::_1, std::placeholders::_2), local);
}