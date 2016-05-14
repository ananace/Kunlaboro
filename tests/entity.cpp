#include <Kunlaboro/Component.inl>
#include <Kunlaboro/Entity.inl>
#include <Kunlaboro/EntitySystem.inl>
#include <Kunlaboro/Views.inl>
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

struct NumberComponent : public Kunlaboro::Component
{
	NumberComponent(int i)
		: Number(i)
	{ }

	int Number;
};

struct NameComponent : public Kunlaboro::Component
{
	NameComponent(const std::string& name)
		: Name(name)
	{ }

	std::string Name;
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

TEST_CASE("fizzbuzz", "[entity][view]")
{
	Kunlaboro::EntitySystem es;

	for (int i = 1; i <= 15; ++i)
	{
		auto ent = es.entityCreate();

		if (i % 3 == 0 && i % 5 == 0)
			ent.addComponent<NameComponent>("fizzbuzz");
		else if (i % 3 == 0)
			ent.addComponent<NameComponent>("fizz");
		else if (i % 5 == 0)
			ent.addComponent<NameComponent>("buzz");
		ent.addComponent<NumberComponent>(i);
	}

	SECTION("Range-based for")
	{
		std::string result;

		auto view = es.entities();
		for (auto& ent : view)
		{
			if (ent.hasComponent<NameComponent>())
				result += ent.getComponent<NameComponent>()->Name + " ";
			else if (ent.hasComponent<NumberComponent>())
				result += std::to_string(ent.getComponent<NumberComponent>()->Number) + " ";
		}

		REQUIRE(result == "1 2 fizz 4 buzz fizz 7 8 fizz buzz 11 fizz 13 14 fizzbuzz ");
	}

	SECTION("forEach - match any")
	{
		std::string result;

		auto view = es.entities();
		view.forEach<NumberComponent, NameComponent>([&result](Kunlaboro::Entity& ent, NumberComponent* number, NameComponent* name) {
			if (name)
				result += name->Name + " ";
			else
				result += std::to_string(number->Number) + " ";
		}, Kunlaboro::EntityView::Match_Any);

		REQUIRE(result == "1 2 fizz 4 buzz fizz 7 8 fizz buzz 11 fizz 13 14 fizzbuzz ");
	}

	SECTION("forEach - match all")
	{
		std::string result;

		auto view = es.entities();
		view.forEach<NumberComponent, NameComponent>([&result](Kunlaboro::Entity& ent, NumberComponent* number, NameComponent* name) {
			result += std::to_string(number->Number);
			result += name->Name + " ";
		}, Kunlaboro::EntityView::Match_All);

		REQUIRE(result == "3fizz 5buzz 6fizz 9fizz 10buzz 12fizz 15fizzbuzz ");
	}
}
