#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>

using namespace Kunlaboro;

const ComponentId ComponentId::INVALID(UINT32_MAX);
ComponentId::GenerationType BaseComponentHandle::sGenerationCounter = 0;

Component::Component()
	: mES(nullptr)
	, mId(ComponentId::INVALID)
	, mOwnerId(EntityId::INVALID)
{

}

const ComponentId& Component::getId() const
{
	return mId;
}
const EntityId& Component::getEntityId() const
{
	return mOwnerId;
}
const EntitySystem* Component::getEntitySystem() const
{
	return mES;
}
EntitySystem* Component::getEntitySystem()
{
	return mES;
}

BaseComponentHandle::BaseComponentHandle()
	: mPtr(nullptr)
	, mCounter(nullptr)
{

}
BaseComponentHandle::BaseComponentHandle(Component* ptr, uint32_t* counter)
	: mPtr(ptr)
	, mCounter(counter)
{
	addRef();
}
BaseComponentHandle::BaseComponentHandle(const BaseComponentHandle& copy)
	: mPtr(copy.mPtr)
	, mCounter(copy.mCounter)
{
	addRef();
}
BaseComponentHandle::BaseComponentHandle(BaseComponentHandle&& move)
	: mPtr(std::move(move.mPtr))
	, mCounter(std::move(move.mCounter))
{
	move.mPtr = nullptr;
	move.mCounter = nullptr;
}
BaseComponentHandle::~BaseComponentHandle()
{
	release();
}

BaseComponentHandle& BaseComponentHandle::operator=(const BaseComponentHandle& assign)
{
	if (this == &assign)
		return *this;

	release();
	mPtr = assign.mPtr;
	mCounter = assign.mCounter;
	addRef();

	return *this;
}

bool BaseComponentHandle::operator==(const BaseComponentHandle& rhs) const
{
	return mPtr == rhs.mPtr;
}
bool BaseComponentHandle::operator!=(const BaseComponentHandle& rhs) const
{
	return mPtr != rhs.mPtr;
}

void BaseComponentHandle::unlink()
{
	mCounter = nullptr;
}

void BaseComponentHandle::addRef()
{
	if (mCounter)
		++(*mCounter);
}
void BaseComponentHandle::release()
{
	bool release = false;
	if (mCounter && *mCounter > 0)
	{
		auto count = --(*mCounter);
		release = count == 0;
	}

	if (release && mPtr)
		mPtr->getEntitySystem()->componentDestroy(mPtr->getId());
}
