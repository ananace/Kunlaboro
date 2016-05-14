#include <Kunlaboro/Component.inl>
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

	SECTION("POD creation")
	{
		for (int i = 0; i < 1000000; ++i)
			es.componentCreate<PODComponent>().unlink();
	}
	SECTION("large chunk POD creation")
	{
		for (int i = 0; i < 1000000; ++i)
			es.componentCreate<PODComponentLargeChunks>().unlink();
	}
}

TEST_CASE("POD component destruction - 1 000 000", "[.performance][component]")
{
	Kunlaboro::EntitySystem es;

	for (int i = 0; i < 1000000; ++i)
		es.componentCreate<PODComponent>().unlink();

	SECTION("POD destruction")
	{
		auto family = Kunlaboro::ComponentFamily<PODComponent>::getFamily();
		for (int i = 0; i < 1000000; ++i)
			es.componentDestroy(Kunlaboro::ComponentId(i, 0, family));
	}

	for (int i = 0; i < 1000000; ++i)
		es.componentCreate<PODComponentLargeChunks>().unlink();

	SECTION("large chunk POD destruction")
	{
		auto family = Kunlaboro::ComponentFamily<PODComponentLargeChunks>::getFamily();
		for (int i = 0; i < 1000000; ++i)
			es.componentDestroy(Kunlaboro::ComponentId(i, 0, family));
	}
}

TEST_CASE("POD component iteration - 1 000 000", "[.performance][component]")
{
	Kunlaboro::EntitySystem es;

	for (int i = 0; i < 1000000; ++i)
		es.componentCreate<PODComponent>().unlink();

	SECTION("POD iteration - range based")
	{
		auto view = es.components<PODComponent>();
		for (auto& comp : view)
		{
			(void)comp.getId();
		}
	}

	SECTION("POD iteration - forEach")
	{
		auto view = es.components<PODComponent>();
		view.forEach([](PODComponent& comp) {
			(void)comp.getId();
		});
	}

	for (int i = 0; i < 1000000; ++i)
		es.componentCreate<PODComponentLargeChunks>().unlink();

	SECTION("large chunk POD iteration - range based")
	{
		auto view = es.components<PODComponentLargeChunks>();
		for (auto& comp : view)
		{
			(void)comp.getId();
		}
	}

	SECTION("large chunk POD iteration - forEach")
	{
		auto view = es.components<PODComponentLargeChunks>();
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
			es.componentCreate<NonPODComponent>().unlink();
	}

	for (int i = 0; i < 1000000; ++i)
		es.componentCreate<NonPODComponent>().unlink();

	SECTION("non-POD destruction")
	{
		auto family = Kunlaboro::ComponentFamily<NonPODComponent>::getFamily();
		for (int i = 0; i < 1000000; ++i)
			es.componentDestroy(Kunlaboro::ComponentId(i, 0, family));
	}

	SECTION("non-POD iteration - range based")
	{
		auto view = es.components<NonPODComponent>();
		for (auto& comp : view)
		{
			(void)comp.getId();
		}
	}

	SECTION("non-POD iteration - forEach")
	{
		auto view = es.components<NonPODComponent>();
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
			es.entityCreate();

		SECTION("entity destruction")
		{
			for (int i = 0; i < 1000000; ++i)
				es.entityDestroy(Kunlaboro::EntityId(i, 0));
		}

		SECTION("entity component addition")
		{
			for (int i = 0; i < 1000000; ++i)
			{
				auto comp = es.componentCreate<PODComponent>();
				es.componentAttach(comp->getId(), Kunlaboro::EntityId(i, 0));
			}


			SECTION("entity destruction")
			{
				for (int i = 0; i < 1000000; ++i)
					es.entityDestroy(Kunlaboro::EntityId(i, 0));
			}
		}
	}

	/*
	SECTION("iteration - range based")
	{
		auto view = es.entities();
		for (auto& ent : view)
		{
			(void)ent.getId();
		}
	}
	SECTION("iteration - forEach")
	{
		auto view = es.entities();
		view.forEach([](Entity& ent) {
			(void)ent.getId();
		});
	}
	*/
}