#include <Kunlaboro/Entity.hpp>
#include <Kunlaboro/Entity.inl>
#include <Kunlaboro/EntitySystem.hpp>

using namespace Kunlaboro;

Entity::Entity()
	: mES(nullptr)
{

}

Entity::Entity(EntitySystem* es, EntityId eid)
	: mES(es)
	, mId(eid)
{

}

bool Entity::operator==(const Entity& other) const
{
	return mId == other.mId;
}
bool Entity::operator!=(const Entity& other) const
{
	return mId != other.mId;
}

Entity::operator EntityId() const
{
	return mId;
}

Entity::operator bool() const
{
	return isValid();
}

const EntityId& Entity::getId() const
{
	return mId;
}

bool Entity::isValid() const
{
	if (!mES)
		return false;
	return mES->entityAlive(mId);
}

void Entity::destroy()
{
	mES->entityDestroy(mId);
	mES = nullptr;
}
