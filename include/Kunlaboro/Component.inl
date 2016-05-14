#pragma once

#include "Component.hpp"

namespace Kunlaboro
{

	template<typename T>
	ComponentHandle<T>::ComponentHandle()
		: BaseComponentHandle()
	{
	}

	template<typename T>
	ComponentHandle<T>::ComponentHandle(T* ptr, std::vector<uint32_t>* counters)
		: BaseComponentHandle(ptr, counters)
	{
	}

	template<typename T>
	ComponentHandle<T>::ComponentHandle(const BaseComponentHandle& copy)
		: BaseComponentHandle(copy)
	{
	}

	template<typename T>
	ComponentHandle<T>::ComponentHandle(BaseComponentHandle&& move)
		: BaseComponentHandle(std::forward<BaseComponentHandle&&>(move))
	{
	}

	template<typename T>
	const T* ComponentHandle<T>::operator->() const
	{
		return get();
	}

	template<typename T>
	T* ComponentHandle<T>::operator->()
	{
		return get();
	}

	template<typename T>
	const T& ComponentHandle<T>::operator*() const
	{
		return *get();
	}

	template<typename T>
	T& ComponentHandle<T>::operator*()
	{
		return *get();
	}

}
