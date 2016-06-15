#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>
#include "catch.hpp"

TEST_CASE("Entity system creation")
{
	Kunlaboro::EntitySystem es;

	REQUIRE(es.entityGetList().empty());
	REQUIRE(!es.isAlive(Kunlaboro::ComponentId(0,0,0)));
	REQUIRE(!es.isAlive(Kunlaboro::EntityId(0,0)));
}
