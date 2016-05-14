#include <Kunlaboro/Component.inl>
#include <Kunlaboro/Entity.inl>
#include <Kunlaboro/EntitySystem.inl>
#include "catch.hpp"

#include <random>

class TestComponent : public Kunlaboro::Component
{
public:
	struct Message : public BaseMessage
	{
		static constexpr uint16_t Id = 0x0532;

		Message()
			: BaseMessage{ Id }
			, NewData(0)
		{}
		Message(int data)
			: BaseMessage{ Id }
			, NewData(data)
		{}

		int NewData;
	};

	TestComponent()
		: mData(-1)
	{

	}

	TestComponent(int data)
		: mData(data)
	{

	}

	int getData() const { return mData; }

	virtual void onMessage(BaseMessage* msg) override
	{
		if (msg->MessageId == Message::Id)
			mData = static_cast<Message*>(msg)->NewData;
	}

private:
	int mData;
};

TEST_CASE("entity creation", "[entity]")
{
	Kunlaboro::EntitySystem es;

	auto ent = es.entityCreate();
	ent.addComponent<TestComponent>(42);

	REQUIRE(ent.hasComponent<TestComponent>());

	auto comp = ent.getComponent<TestComponent>();

	REQUIRE(comp->getData() == 42);

	TestComponent::Message newData(23);
	es.entitySendMessage(ent.getId(), &newData);

	REQUIRE(comp->getData() == 23);
}