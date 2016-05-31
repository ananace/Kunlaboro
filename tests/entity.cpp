#include <Kunlaboro/Component.inl>
#include <Kunlaboro/Entity.inl>
#include <Kunlaboro/EntitySystem.inl>
#include <Kunlaboro/EventSystem.inl>
#include <Kunlaboro/MessageSystem.inl>
#include <Kunlaboro/Message.inl>
#include <Kunlaboro/Views.inl>
#include "catch.hpp"

class MessagingTestComponent : public Kunlaboro::MessagingComponent
{
public:
	MessagingTestComponent()
		: mData(-1)
	{

	}

	MessagingTestComponent(int data)
		: mData(data)
	{

	}

	void addedToEntity()
	{
		requestMessageId<float>("SetData", &MessagingTestComponent::setData);
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
	ent.addComponent<MessagingTestComponent>(42);

	REQUIRE(ent.hasComponent<MessagingTestComponent>());
	REQUIRE(i == 1);

	auto comp = ent.getComponent<MessagingTestComponent>();

	REQUIRE(comp->getData() == 42);
}

TEST_CASE("Message passing", "[entity][message]")
{
	Kunlaboro::EntitySystem es;
	es.getMessageSystem().messageRegisterId<int>("SetData");

	auto ent = es.entityCreate();
	ent.addComponent<MessagingTestComponent>(42);

	es.getMessageSystem().messageSendId("SetData", 5);

	auto comp = ent.getComponent<MessagingTestComponent>();
	REQUIRE(comp->getData() == 5);
}
