#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/Entity.inl>
#include <Kunlaboro/EntitySystem.inl>
#include <Kunlaboro/EventSystem.inl>
#include <Kunlaboro/MessageSystem.inl>
#include <Kunlaboro/Message.inl>
#include <Kunlaboro/Views.inl>
#include "catch.hpp"

TEST_CASE("Message system creation")
{
	Kunlaboro::EntitySystem es;

	auto& ms = es.getMessageSystem();
	REQUIRE(true);
}

class MessagingTestComponent : public Kunlaboro::MessagingComponent
{
public:
	MessagingTestComponent()
		: mValue(0)
	{ }

	void addedToEntity()
	{
		requestMessageId("Either.SetValue", &MessagingTestComponent::setValue);
		requestMessageId("Global.SetValue", &MessagingTestComponent::setValue);
		requestMessageId("Local.SetValue", &MessagingTestComponent::setValue);
	}

	void setValue(int value)
	{
		mValue = value;
	}
	int getValue()
	{
		return mValue;
	}

private:
	int mValue;
};

TEST_CASE("Message passing locality", "[message]")
{
	Kunlaboro::EntitySystem es;

	auto& ms = es.getMessageSystem();

	ms.registerMessage<int>("Either.SetValue", Kunlaboro::MessageSystem::Message_Either);
	ms.registerMessage<int>("Global.SetValue", Kunlaboro::MessageSystem::Message_Global);
	ms.registerMessage<int>("Local.SetValue", Kunlaboro::MessageSystem::Message_Local);

	auto ent = es.createEntity();
	ent.addComponent<MessagingTestComponent>();

	SECTION("Either locality messages")
	{
		auto comp = ent.getComponent<MessagingTestComponent>();

		CHECK(comp->getValue() == 0);

		ms.sendMessage("Either.SetValue", 10);
		REQUIRE(comp->getValue() == 10);

		ms.sendMessageTo("Either.SetValue", comp->getId(), -5);
		REQUIRE(comp->getValue() == -5);
	}

	SECTION("Global locality messages")
	{
		auto comp = ent.getComponent<MessagingTestComponent>();

		CHECK(comp->getValue() == 0);

		ms.sendMessage("Global.SetValue", 10);
		REQUIRE(comp->getValue() == 10);

		ms.sendMessageTo("Global.SetValue", comp->getId(), -5);
		REQUIRE(comp->getValue() != -5);
	}

	SECTION("Local locality messages")
	{
		auto comp = ent.getComponent<MessagingTestComponent>();

		CHECK(comp->getValue() == 0);

		ms.sendMessage("Local.SetValue", 10);
		REQUIRE(comp->getValue() != 10);

		ms.sendMessageTo("Local.SetValue", comp->getId(), -5);
		REQUIRE(comp->getValue() == -5);
	}
}

