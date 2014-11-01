#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>
#include "catch.hpp"

#include <iostream>

SCENARIO("Testing")
{
    class TestComponent : public Kunlaboro::Component
    {
    public:
        TestComponent() : Kunlaboro::Component("Test") { }

        void addedToEntity() {
            requestMessage("Print", &TestComponent::test);
        }

    private:
        int test(const std::string& msg)
        {
            std::cout << msg << std::endl;

            return 5;
        }
    };

    Kunlaboro::EntitySystem es;
    es.registerComponent<TestComponent>("Test");
        
    auto eid = es.createEntity();
    es.addComponent(eid, "Test");
    es.finalizeEntity(eid);

    int ret = es.sendSafeGlobalMessage<int>(Kunlaboro::hash::hashString("Test"), "I am a string");

    std::cout << "Returned: " << ret << std::endl;
}