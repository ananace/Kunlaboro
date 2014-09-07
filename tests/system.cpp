#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>
#include "catch.hpp"

SCENARIO("Entity System creation")
{
    Kunlaboro::EntitySystem es;

    GIVEN("A newly created, empty, Entity System")
    {
        THEN("The Entity System is truly empty")
        {
            CHECK(es.numCom() == 0);
            CHECK(es.numEnt() == 0);
        }
    }

    GIVEN("An Entity System with an empty entity in it")
    {
        auto eid = es.createEntity();

        THEN("The entity system only contains that entity")
        {
            CHECK(es.numCom() == 0);
            CHECK(es.numEnt() == 1);
        }

        THEN("That entity is valid")
        {
            CHECK(es.isValid(eid));
        }
    }

    GIVEN("An entity System with a full entity in it")
    {
        class TestComponent : public Kunlaboro::Component
        {
        public:
            TestComponent() : Kunlaboro::Component("TestComponent") { }
        };

        es.registerComponent<TestComponent>("TestComponent");

        auto component = es.createComponent("TestComponent");

        auto eid = es.createEntity();
        es.addComponent(eid, component);
        es.finalizeEntity(eid);

        THEN("The entity system only contains that entity")
        {
            CHECK(es.numCom() == 1);
            CHECK(es.numEnt() == 1);
        }

        THEN("The entity only contains its component")
        {
            auto comps = es.getAllComponentsOnEntity(eid);

            CHECK(comps.size() == 1);
            CHECK((comps.front() == component));
        }

        WHEN("The entity has its component removed")
        {
            es.removeComponent(eid, component);

            THEN("The entity system still contains the component and the entity")
            {
                CHECK(es.numCom() == 1);
                CHECK(es.numEnt() == 1);
            }
            THEN("The entity is empty")
            {
                CHECK(es.getAllComponentsOnEntity(eid).empty());
            }
        }
    }
}

SCENARIO("Entity System usage")
{
    Kunlaboro::EntitySystem es;

    GIVEN("An empty Entity System")
    {
        for (uint32_t i = 0; i < 10000; ++i)
            es.createEntity();

        THEN("A large amount of entities can be created in it")
        {
            CHECK(es.numEnt() == 10000);
        }
    }

    GIVEN("An Entity System with a few entities in it")
    {
        class TestComponent : public Kunlaboro::Component
        {
        public:
            TestComponent() : Kunlaboro::Component("TestComponent") { }
        };

        es.registerComponent<TestComponent>("TestComponent");

        std::vector<Kunlaboro::EntityId> ents = {
            es.createEntity(), es.createEntity(), es.createEntity(), es.createEntity(), es.createEntity(),
            es.createEntity(), es.createEntity(), es.createEntity(), es.createEntity(), es.createEntity()
        };

        WHEN("Components are added to all entities")
        {
            for (auto eid : ents)
            {
                es.addComponent(eid, "TestComponent");
            }

            THEN("There are an equal amount of components and entities")
            {
                CHECK(es.numCom() == es.numEnt());
            }

            AND_WHEN("The components are destroyed again")
            {
                for (auto eid : ents)
                {
                    auto comp = es.getAllComponentsOnEntity(eid)[0];
                    es.destroyComponent(comp);
                }

                THEN("The only the entities remain")
                {
                    CHECK(es.numCom() == 0);
                    CHECK(es.numEnt() == 10);
                }
            }
        }

        WHEN("A component is moved between all entities")
        {
            auto component = es.createComponent("TestComponent");
            es.addComponent(ents[0], component);

            Kunlaboro::EntityId last = 0;
            for (auto eid : ents)
            {
                if (last != 0)
                {
                    es.removeComponent(last, component);
                    es.addComponent(eid, component);
                }

                last = eid;
            }

            THEN("The component is still valid, and in the correct entity")
            {
                CHECK(component->isValid());
                CHECK(component->getOwnerId() == ents.back());
            }
        }
    }
}
