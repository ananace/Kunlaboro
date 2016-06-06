#include <Kunlaboro/Component.inl>
#include <Kunlaboro/Entity.inl>
#include <Kunlaboro/EntitySystem.inl>
#include <Kunlaboro/EventSystem.inl>
#include <Kunlaboro/MessageSystem.inl>
#include <Kunlaboro/Message.inl>
#include <Kunlaboro/Views.inl>
#include "catch.hpp"

class EntityMessagingTestComponent : public Kunlaboro::MessagingComponent
{
public:
	EntityMessagingTestComponent()
		: mData(-1)
	{

	}

	EntityMessagingTestComponent(int data)
		: mData(data)
	{

	}

	void addedToEntity()
	{
		requestMessageId<float>("SetData", &EntityMessagingTestComponent::setData);
	}

	int getData() const { return mData; }
	void setData(int data) { mData = data; }

private:
	int mData;
};

TEST_CASE("entity creation", "[entity]")
{
	Kunlaboro::EntitySystem es;

	int i = 0;
	auto func = [&i](const Kunlaboro::EntitySystem::EntityCreatedEvent& ev) {
		++i;
	};
	es.getEventSystem().eventRegister<Kunlaboro::EntitySystem::EntityCreatedEvent>(func);

	auto ent = es.entityCreate();
	ent.addComponent<EntityMessagingTestComponent>(42);

	REQUIRE(ent.hasComponent<EntityMessagingTestComponent>());
	REQUIRE(i == 1);

	auto comp = ent.getComponent<EntityMessagingTestComponent>();

	REQUIRE(comp->getData() == 42);
}

TEST_CASE("Message passing", "[entity][message]")
{
	Kunlaboro::EntitySystem es;
	es.getMessageSystem().messageRegisterId<int>("SetData");

	auto ent = es.entityCreate();
	ent.addComponent<EntityMessagingTestComponent>(42);

	es.getMessageSystem().messageSendId("SetData", 5);

	auto comp = ent.getComponent<EntityMessagingTestComponent>();
	REQUIRE(comp->getData() == 5);
}
