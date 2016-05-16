#include <Kunlaboro/Component.inl>
#include <Kunlaboro/Entity.inl>
#include <Kunlaboro/EntitySystem.inl>
#include <Kunlaboro/Views.inl>
#include "catch.hpp"

#include <atomic>
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

		auto view = Kunlaboro::EntityView(es);
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

		auto view = Kunlaboro::EntityView(es).withComponents<Kunlaboro::Match_Any, NumberComponent, NameComponent>();
		view.forEach([&result](Kunlaboro::Entity&, NumberComponent* number, NameComponent* name) {
			if (name)
				result += name->Name + " ";
			else
				result += std::to_string(number->Number) + " ";
		});

		REQUIRE(result == "1 2 fizz 4 buzz fizz 7 8 fizz buzz 11 fizz 13 14 fizzbuzz ");
	}

	SECTION("forEach - match all")
	{
		std::string result;

		auto view = Kunlaboro::EntityView(es).withComponents<Kunlaboro::Match_All, NumberComponent, NameComponent>();
		view.forEach([&result](Kunlaboro::Entity&, NumberComponent& number, NameComponent& name) {
			result += std::to_string(number.Number);
			result += name.Name + " ";
		});

		REQUIRE(result == "3fizz 5buzz 6fizz 9fizz 10buzz 12fizz 15fizzbuzz ");
	}
}

struct Position : public Kunlaboro::Component
{
	Position(float x, float y)
		: X(x)
		, Y(y)
	{ }

	volatile float X, Y;
};
struct Velocity : public Kunlaboro::Component
{
	Velocity(float x, float y)
		: X(x)
		, Y(y)
	{ }

	volatile float X, Y;
};

TEST_CASE("Simple n-body simulation", "[performance][view]")
{
	Kunlaboro::EntitySystem es;

	SECTION("Creation, 100 particles")
	{
		const int ParticleCount = 100;

		std::random_device rand;
		std::uniform_real_distribution<float> ang(0, 3.14159f * 2);
		std::uniform_real_distribution<float> mag(0, 100);

		for (int i = 0; i < ParticleCount; ++i)
		{
			auto ent = es.entityCreate();
			float angle = ang(rand);
			float magnitude = mag(rand);

			ent.addComponent<Position>(std::cos(angle) * magnitude, std::sin(angle) * magnitude);
			ent.addComponent<Velocity>((mag(rand) - 50) / 5, (mag(rand) - 50) / 5);
		}

		REQUIRE(es.componentGetPool(Kunlaboro::ComponentFamily<Position>::getFamily()).countBits() == ParticleCount);
		REQUIRE(es.componentGetPool(Kunlaboro::ComponentFamily<Velocity>::getFamily()).countBits() == ParticleCount);

		SECTION("Iteration, 100 steps, 100 000 calls per step")
		{
			const int IterationCount = 100;

			std::atomic<uint32_t> gravityIterations(0)
			                    , velocityIterations(0);

			auto entityView = Kunlaboro::EntityView(es).withComponents<Kunlaboro::Match_All, Position,Velocity>();
			auto particleList = Kunlaboro::EntityView(es).withComponents<Kunlaboro::Match_All, Position>();

			for (int step = 0; step < IterationCount; ++step)
			{
				entityView.forEach([&gravityIterations, &velocityIterations, &particleList](Kunlaboro::Entity& ent, Position& pos, Velocity& vel) {
					particleList.forEach([&gravityIterations, &ent, &pos, &vel](Kunlaboro::Entity& ent2, Position& pos2) {
						if (ent == ent2)
							return;

						const float xDelta = (pos2.X - pos.X);
						const float yDelta = (pos2.Y - pos.Y);
						const float deltaSqrt = std::sqrt(xDelta*xDelta + yDelta*yDelta + 1e-9f);
						const float invDist = 1 / deltaSqrt;
						const float invDist2 = invDist * invDist;

						vel.X += xDelta * invDist2;
						vel.Y += yDelta * invDist2;

						gravityIterations.fetch_add(1);
					});

					pos.X += vel.X;
					pos.Y += vel.Y;

					velocityIterations.fetch_add(1);
				});
			}

			REQUIRE(gravityIterations  == ParticleCount * (ParticleCount - 1) * IterationCount);
			REQUIRE(velocityIterations == ParticleCount * IterationCount);
		}

		SECTION("Parallel gravity iteration, 100 steps, 100 000 calls per step")
		{
			const int IterationCount = 100;

			std::atomic<uint32_t> gravityIterations(0)
			                    , velocityIterations(0);

			Kunlaboro::detail::JobQueue queue;
			auto entityView = Kunlaboro::EntityView(es).withComponents<Kunlaboro::Match_All, Position,Velocity>();
			auto particleList = Kunlaboro::EntityView(es).withComponents<Kunlaboro::Match_All, Position>().parallel(queue);

			for (int step = 0; step < IterationCount; ++step)
			{
				entityView.forEach([&gravityIterations, &velocityIterations, &particleList](Kunlaboro::Entity& ent, Position& pos, Velocity& vel) {
					particleList.forEach([&gravityIterations, &ent, &pos, &vel](Kunlaboro::Entity& ent2, Position& pos2) {
						if (ent == ent2)
							return;

						const float xDelta = (pos2.X - pos.X);
						const float yDelta = (pos2.Y - pos.Y);
						const float deltaSqrt = std::sqrt(xDelta*xDelta + yDelta*yDelta + 1e-9f);
						const float invDist = 1 / deltaSqrt;
						const float invDist2 = invDist * invDist;

						vel.X += xDelta * invDist2;
						vel.Y += yDelta * invDist2;

						gravityIterations.fetch_add(1, std::memory_order_relaxed);
					});

					pos.X += vel.X;
					pos.Y += vel.Y;

					velocityIterations.fetch_add(1, std::memory_order_relaxed);
				});
			}

			REQUIRE(gravityIterations  == ParticleCount * (ParticleCount - 1) * IterationCount);
			REQUIRE(velocityIterations == ParticleCount * IterationCount);
		}

	}
}
