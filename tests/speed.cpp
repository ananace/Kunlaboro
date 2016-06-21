#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.inl>
#include <Kunlaboro/Views.inl>
#include "catch.hpp"

#include <ctime>

uint64_t calls;

struct PODComponent : public Kunlaboro::Component
{
	int data;
};

struct PODComponentLargeChunks : public Kunlaboro::Component
{
	enum
	{
		sPreferredChunkSize = 8192
	};

	int data;
};

struct NonPODComponent : public Kunlaboro::Component
{
	NonPODComponent()
		: mCreation(time(nullptr))
	{

	}

	time_t mCreation;
};

TEST_CASE("POD component creation - 1 000 000", "[.performance][component]")
{
	Kunlaboro::EntitySystem es;

	uint8_t family = -1;

	SECTION("POD creation")
	{
		family = Kunlaboro::ComponentFamily<PODComponent>::getFamily();

		for (int i = 0; i < 1000000; ++i)
			es.createComponent<PODComponent>().unlink();

		REQUIRE(es.componentGetPool(family).countBits() == 1000000);
	}
	SECTION("large chunk POD creation")
	{
		family = Kunlaboro::ComponentFamily<PODComponentLargeChunks>::getFamily();

		for (int i = 0; i < 1000000; ++i)
			es.createComponent<PODComponentLargeChunks>().unlink();

		REQUIRE(es.componentGetPool(family).countBits() == 1000000);
	}

	if (family != -1)
	{
		for (int i = 0; i < 1000000; ++i)
			es.destroyComponent(Kunlaboro::ComponentId(i, 0, family));

		es.cleanComponents();
		REQUIRE(es.componentGetPool(family).getSize() == 0);
	}
}

TEST_CASE("POD component destruction - 1 000 000", "[.performance][component]")
{
	Kunlaboro::EntitySystem es;

	for (int i = 0; i < 1000000; ++i)
		es.createComponent<PODComponent>().unlink();

	CHECK(es.componentGetPool(Kunlaboro::ComponentFamily<PODComponent>::getFamily()).countBits() == 1000000);

	SECTION("POD destruction")
	{
		auto family = Kunlaboro::ComponentFamily<PODComponent>::getFamily();
		for (int i = 0; i < 1000000; ++i)
			es.destroyComponent(Kunlaboro::ComponentId(i, 0, family));

		CHECK(es.componentGetPool(Kunlaboro::ComponentFamily<PODComponent>::getFamily()).countBits() == 0);
	}

	for (int i = 0; i < 1000000; ++i)
		es.createComponent<PODComponentLargeChunks>().unlink();

	CHECK(es.componentGetPool(Kunlaboro::ComponentFamily<PODComponentLargeChunks>::getFamily()).countBits() == 1000000);

	SECTION("large chunk POD destruction")
	{
		auto family = Kunlaboro::ComponentFamily<PODComponentLargeChunks>::getFamily();
		for (int i = 0; i < 1000000; ++i)
			es.destroyComponent(Kunlaboro::ComponentId(i, 0, family));

		CHECK(es.componentGetPool(Kunlaboro::ComponentFamily<PODComponentLargeChunks>::getFamily()).countBits() == 0);
	}
}

TEST_CASE("POD component iteration - 1 000 000", "[.performance][component]")
{
	Kunlaboro::EntitySystem es;

	for (int i = 0; i < 1000000; ++i)
		es.createComponent<PODComponent>().unlink();

	CHECK(es.componentGetPool(Kunlaboro::ComponentFamily<PODComponent>::getFamily()).countBits() == 1000000);

	SECTION("POD iteration - range based")
	{
		auto view = Kunlaboro::ComponentView<PODComponent>(es);
		for (auto& comp : view)
		{
			(void)comp.getId();
		}
	}

	SECTION("POD iteration - forEach")
	{
		auto view = Kunlaboro::ComponentView<PODComponent>(es);
		view.forEach([](PODComponent& comp) {
			(void)comp.getId();
		});
	}

	for (int i = 0; i < 1000000; ++i)
		es.createComponent<PODComponentLargeChunks>().unlink();

	CHECK(es.componentGetPool(Kunlaboro::ComponentFamily<PODComponentLargeChunks>::getFamily()).countBits() == 1000000);

	SECTION("large chunk POD iteration - range based")
	{
		auto view = Kunlaboro::ComponentView<PODComponentLargeChunks>(es);
		for (auto& comp : view)
		{
			(void)comp.getId();
		}
	}

	SECTION("large chunk POD iteration - forEach")
	{
		auto view = Kunlaboro::ComponentView<PODComponentLargeChunks>(es);
		view.forEach([](PODComponentLargeChunks& comp) {
			(void)comp.getId();
		});
	}
}

TEST_CASE("non-POD component performance - 1 000 000", "[.performance][component]")
{
	Kunlaboro::EntitySystem es;

	SECTION("non-POD creation")
	{
		for (int i = 0; i < 1000000; ++i)
			es.createComponent<NonPODComponent>().unlink();

		CHECK(es.componentGetPool(Kunlaboro::ComponentFamily<NonPODComponent>::getFamily()).countBits() == 1000000);
	}

	for (int i = 0; i < 1000000; ++i)
		es.createComponent<NonPODComponent>().unlink();

	SECTION("non-POD destruction")
	{
		auto family = Kunlaboro::ComponentFamily<NonPODComponent>::getFamily();
		for (int i = 0; i < 1000000; ++i)
			es.destroyComponent(Kunlaboro::ComponentId(i, 0, family));

		CHECK(es.componentGetPool(Kunlaboro::ComponentFamily<NonPODComponent>::getFamily()).countBits() == 0);
	}

	SECTION("non-POD iteration - range based")
	{
		auto view = Kunlaboro::ComponentView<NonPODComponent>(es);
		for (auto& comp : view)
		{
			(void)comp.getId();
		}
	}

	SECTION("non-POD iteration - forEach")
	{
		auto view = Kunlaboro::ComponentView<NonPODComponent>(es);
		view.forEach([](NonPODComponent& comp) {
			(void)comp.getId();
		});
	}
}

TEST_CASE("entity performance - 1 000 000", "[.performance][entity]")
{
	Kunlaboro::EntitySystem es;

	SECTION("entity creation")
	{
		for (int i = 0; i < 1000000; ++i)
			es.createEntity();

		SECTION("entity destruction")
		{
			for (int i = 0; i < 1000000; ++i)
				es.destroyEntity(Kunlaboro::EntityId(i, 0));
		}

		SECTION("entity component addition")
		{
			for (int i = 0; i < 1000000; ++i)
			{
				auto comp = es.createComponent<PODComponent>();
				es.attachComponent(comp->getId(), Kunlaboro::EntityId(i, 0));
			}

			SECTION("entity destruction")
			{
				for (int i = 0; i < 1000000; ++i)
					es.destroyEntity(Kunlaboro::EntityId(i, 0));
			}
		}

		SECTION("iteration - range based")
		{
			auto view = Kunlaboro::EntityView(es);
			for (auto& ent : view)
			{
				(void)ent.getId();
			}
		}
		SECTION("iteration - forEach")
		{
			auto view = Kunlaboro::EntityView(es);
			view.forEach([](const Kunlaboro::Entity& ent) {
				(void)ent.getId();
			});
		}
	}
}
