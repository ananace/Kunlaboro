#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>

using namespace Kunlaboro;

ComponentId::FamilyType BaseComponentFamily::sFamilyCounter = 0;

const ComponentId& Component::getId() const
{
	return mId;
}
EntityId Component::getEntityId() const
{
	return mES->getEntity(mId);
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
BaseComponentHandle::BaseComponentHandle(Component* ptr, std::atomic_ushort* counter)
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
	if (mCounter && mPtr)
		++(*mCounter);
}
void BaseComponentHandle::release()
{
	if (!mPtr || !mCounter)
		return;

	const auto& id = mPtr->getId();
	auto* es = mPtr->getEntitySystem();

	if (es->isAlive(id))
	{
		auto count = mCounter->fetch_sub(1);
		
		if (count <= 1)
			es->destroyComponent(id);
	}
}
uint32_t BaseComponentHandle::getRefCount() const
{
	if (mCounter && mPtr)
		return mCounter->load();
	return 0;
}
