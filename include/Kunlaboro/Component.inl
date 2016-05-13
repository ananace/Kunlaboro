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
	ComponentHandle<T>::ComponentHandle(T* ptr, uint32_t* counter)
		: BaseComponentHandle(ptr, counter)
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
