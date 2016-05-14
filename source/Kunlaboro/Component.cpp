#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>

using namespace Kunlaboro;

ComponentId::FamilyType BaseComponentFamily::sFamilyCounter = 0;

Component::Component()
	: mES(nullptr)
	, mId()
	, mOwnerId()
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
	, mCounters(nullptr)
{

}
BaseComponentHandle::BaseComponentHandle(Component* ptr, std::vector<uint32_t>* counters)
	: mPtr(ptr)
	, mCounters(counters)
{
	addRef();
}
BaseComponentHandle::BaseComponentHandle(const BaseComponentHandle& copy)
	: mPtr(copy.mPtr)
	, mCounters(copy.mCounters)
{
	addRef();
}
BaseComponentHandle::BaseComponentHandle(BaseComponentHandle&& move)
	: mPtr(std::move(move.mPtr))
	, mCounters(std::move(move.mCounters))
{
	move.mPtr = nullptr;
	move.mCounters = nullptr;
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
	mCounters = assign.mCounters;
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
	mCounters = nullptr;
}

void BaseComponentHandle::addRef()
{
	if (mCounters && mPtr)
		++(*mCounters)[mPtr->getId().getIndex()];
}
void BaseComponentHandle::release()
{
	if (!mPtr || !mCounters)
		return;

	auto& id = mPtr->getId();
	auto* es = mPtr->getEntitySystem();

	if (es->componentAlive(id))
	{
		auto count = --(*mCounters)[id.getIndex()];
		
		if (count == 0)
			es->componentDestroy(id);
	}
}
uint32_t BaseComponentHandle::getRefCount() const
{
	if (mCounters && mPtr)
		return (*mCounters)[mPtr->getId().getIndex()];
	return 0;
}