#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>
#include "catch.hpp"

TEST_CASE("Entity system creation")
{
	Kunlaboro::EntitySystem es;

	REQUIRE(es.entityGetList().empty());
	REQUIRE(!es.componentAlive(Kunlaboro::ComponentId(0,0,0)));
	REQUIRE(!es.entityAlive(Kunlaboro::EntityId(0,0)));
}
