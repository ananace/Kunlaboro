#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>
#include "catch.hpp"

uint64_t calls;

SCENARIO("Message passing benchmark",
    "[benchmark]")
{
    calls = 0;

    class BasicComponent : public Kunlaboro::Component
    {
    public:
        BasicComponent() : Kunlaboro::Component("Basic") { }
        void addedToEntity() {
            requestMessage("Ping", &BasicComponent::ping);
            requestMessage("Pong", &BasicComponent::pong);
        }

    private:
        void ping()
        {
            sendGlobalMessage<void>("Pong");

            ++calls;
        }

        void pong()
        {
            ++calls;
        }
    };

    class LambdaComponent : public Kunlaboro::Component
    {
    public:
        LambdaComponent() : Kunlaboro::Component("Lambda") { }
        void addedToEntity() {
            requestMessage("Ping", std::function<void()>([=]() { sendGlobalMessage<void>("Pong"); ++calls; }));
            requestMessage("Pong", std::function<void()>([=]() { ++calls; }));
        }
    };

    Kunlaboro::EntitySystem es;

    es.registerComponent<BasicComponent>("Basic");
    es.registerComponent<LambdaComponent>("Lambda");
    es.registerTemplate("Basic", { "Basic" });
    es.registerTemplate("Lambda", { "Lambda" });

    WHEN("Using direct function mapping")
    {
        std::vector<Kunlaboro::EntityId> eids = {
            es.createEntity("Basic"),
            es.createEntity("Basic"),
            es.createEntity("Basic"),
            es.createEntity("Basic")
        };

        AND_WHEN("Unsafe calls")
        {
            AND_WHEN("Using direct string calls")
            {
                for (uint32_t i = 0; i < 100000; ++i)
                    es.sendUnsafeGlobalMessage<void>(Kunlaboro::hash::hashString("Ping"));

                CHECK(calls == 2000000);
            }

            AND_WHEN("Using cached request IDs")
            {
                Kunlaboro::RequestId rid = Kunlaboro::hash::hashString("Ping");
                for (uint32_t i = 0; i < 100000; ++i)
                    es.sendUnsafeGlobalMessage<void>(rid);

                CHECK(calls == 2000000);
            }
        }

        AND_WHEN("Safe calls")
        {
            AND_WHEN("Using cached request IDs")
            {
                Kunlaboro::RequestId rid = Kunlaboro::hash::hashString("Ping");
                for (uint32_t i = 0; i < 100000; ++i)
                    es.sendSafeGlobalMessage<void>(rid);

                CHECK(calls == 2000000);
            }
        }
    }

    WHEN("Using lambda mappings")
    {
        std::vector<Kunlaboro::EntityId> eids = {
            es.createEntity("Lambda"),
            es.createEntity("Lambda"),
            es.createEntity("Lambda"),
            es.createEntity("Lambda")
        };

        AND_WHEN("Unsafe calls")
        {
            AND_WHEN("Using direct string calls")
            {
                for (uint32_t i = 0; i < 100000; ++i)
                    es.sendUnsafeGlobalMessage<void>(Kunlaboro::hash::hashString("Ping"));

                CHECK(calls == 2000000);
            }

            AND_WHEN("Using cached request IDs")
            {
                Kunlaboro::RequestId rid = Kunlaboro::hash::hashString("Ping");
                for (uint32_t i = 0; i < 100000; ++i)
                    es.sendUnsafeGlobalMessage<void>(rid);

                CHECK(calls == 2000000);
            }
        }

        AND_WHEN("Safe calls")
        {
            AND_WHEN("Using cached request IDs")
            {
                Kunlaboro::RequestId rid = Kunlaboro::hash::hashString("Ping");
                for (uint32_t i = 0; i < 100000; ++i)
                    es.sendSafeGlobalMessage<void>(rid);

                CHECK(calls == 2000000);
            }
        }
    }
}