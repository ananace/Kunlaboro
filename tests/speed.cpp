#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>
#include "catch.hpp"

SCENARIO("Message passing benchmark",
    "[benchmark]")
{
    class BasicComponent : public Kunlaboro::Component
    {
    public:
        BasicComponent() : Kunlaboro::Component("Basic") { }

        void addedToEntity() {
            requestMessage("Ping", &BasicComponent::ping);
            requestMessage("Pong", &BasicComponent::pong);
        }

        void ping() const
        {
            sendGlobalMessage<void>("Pong");
        }

        void pong()
        {

        }
    };

    Kunlaboro::EntitySystem es;
    es.registerComponent<BasicComponent>("Basic");
    es.registerTemplate("Basic", { "Basic" });

    std::vector<Kunlaboro::EntityId> eids = {
        es.createEntity("Basic"),
        es.createEntity("Basic"),
        es.createEntity("Basic"),
        es.createEntity("Basic")
    };

    Kunlaboro::RequestId rid = Kunlaboro::hash::hash_func1::hash("Ping");
    for (uint32_t i = 0; i < 1000000; ++i)
        es.sendSafeGlobalMessage<void>(rid);
}