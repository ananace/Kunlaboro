Kunlaboro [![Build Status](https://travis-ci.org/ace13/Kunlaboro.svg)](https://travis-ci.org/ace13/Kunlaboro)
=========

[Source](https://github.com/ace13/Kunlaboro) | [Issues](https://github.com/ace13/Kunlaboro/issues) | [Documentation](https://ace13.github.io/Kunlaboro)

So I see you've stumbled upon this little project of mine.
Kunlaboro - which is esperanto and means *cooperation* - is a C++ Entity System designed around a heavily modified RDBMS Beta paradigm. It's licensed under the MIT license and you're welcome to use it basically wherever you want.

It is designed to have a reasonably cost-free message passing system to handle communication between components, while still allowing for direct member access when speed is neccessary.

Feel free to provide constructive critisism through issues, pull requests, or just general comments.

Requirements
------------

Kunlaboro is built using many C++11 features, therefore it requires a reasonably modern compiler to work properly.

It's been mainly developed on Microsoft Visual Studio 2013 and uses Travis testing with both GCC and Clang, which are - as of writing this - version GCC-4.6.3 and Clang-3.4 (using libstdc++4.8 to solve a threading issue).

Code examples
-------------

A few quick code examples of Kunlaboro can be seen below, for more detailed information please consult the documentation.

Simple printing component:
```cpp
#include <Kunlaboro/Kunlaboro.hpp>
#include <iostream>

using namespace std;

class PrintComponent : public Kunlaboro::Component
{
public:
    PrintComponent() : Kunlaboro::Component("Print") { }

    void printString(const Kunlaboro::Message& msg)
    {
        cout << boost::any_cast<std::string>(msg.payload) << endl;
    }

    void addedToEntity()
    {
        registerMessage("Print.PrintString", &PrintComponent::printString, true); // Local message
        registerMessage("Print.PrintString", &PrintComponent::printString, false); // Global message
    }
};

int main()
{
    Kunlaboro::EntitySystem es;
    es.registerComponent<PrintComponent>("Print");
    es.registerTemplate("Printer", { "Print" });
    // Note that both template and component names have to be unique

    Kunlaboro::EntityId eId = es.createEntity("Printer");
    /* When using this entity template the Entity System will basically be running this code:
        Kunlaboro::EntityId eId = es.createEntity();
        es.addComponent(eId, es.createComponent("Print"));
        es.finalizeEntity(eId);
    */

    es.sendGlobalMessage("Print.PrintString", "Hello World!");
    es.sendLocalMessage(eId, "Print.PrintString", "Hello Local World!");

    return 0;
}
```

More advanced printing component:
```cpp
#include <Kunlaboro/Kunlaboro.hpp>

class TestComponent : public Kunlaboro::Component
{
public:
    TestComponent() : Kunlaboro::Component("Test") { }
};

int main()
{
    Kunlaboro::EntitySystem es;

    es.registerComponent<TestComponent>("Test");

    auto c1 = es.createComponent("Test");
    c1.requestMessage("Information", [](const Kunlaboro::Message& msg) {
    	std::cout << "Components can have their requests altered during runtime..." << std::endl;
    });
    
    auto c2 = es.createComponent("Test");
    c2.requestMessage("Information", [](const Kunlaboro::Message& msg) {
		std::cout << "Allowing you to do some rather interesting things with them." << std::endl;
    })

    // This is technically not needed, since they end up in the order of their creation.
    // But it helps illustrate the fact that this is possible to do.
    c1.changeRequestPriority("Information", 0);
    c2.changeRequestPriority("Information", 1); 

    std::cout << "Did you know?" << std::endl;

    es.sendGlobalMessage("Information");

    return 0;
}
```

Example of a more advanced application:
```cpp
#include <Kunlaboro/Kunlaboro.hpp>
#include <tuple> // Using a 2D point class might be more effective

class Physical : public Kunlaboro::Component
{
public:
	Physical() : Kunlaboro::Component("Physical"), x(0), y(0), dX(0), dY(0) { }

	void addedToEntity()
	{
		requestMessage("GetPosition", [this](Kunlaboro::Message& msg) {
			msg.handle(std::make_tuple(x, y));
		}, true);
		requestMessage("SetPosition", [this](const Kunlaboro::Message& msg) {
			auto pos = boost::any_cast<std::tuple<float, float> >(msg.payload);
			setPosition(std::get<0>(pos), std::get<1>(pos));
		}, true);
		requestMessage("SetDelta", [this](const Kunlaboro::Message& msg) {
			auto delta = boost::any_cast<std::tuple<float, float> >(msg.payload);
			setDelta(std::get<0>(delta), std::get<1>(delta));
		}, true);

		requestMessage("Update", &Physical::update);
	}

	float getX() const { return x; }
	float getY() const { return y; }

	void setPosition(float newX, float newY) { x = newX; y = newY; }
	void setDelta(float x, float y) { dX = x; dY = y; }

    // Right now, Kunlaboro can only directly register functions that take Message references (const or not).
    // I'm planning to create a variadic system that does type casting automatically in the future though.
	void update(const Kunlaboro::Message&) { x += dX; y += dY; }

private:
	float x, y, dX, dY;
};

class Drawable : public Kunlaboro::Component
{
public:
    Drawable() : Kunlaboro::Component("Graphics::Drawable"), phys(nullptr) { }

    void addedToEntity() 
    {
    	// This could be delegated to functions instead of lambdas if wanted.
    	// NOTE: Component requests default to being local.
    	requestComponent("Physical", [this](const Kunlaboro::Message& msg) {
    		phys = dynamic_cast<Physical>(msg.sender);

    		// NOTE: And message requests default to being global.
    		requestMessage("Update", [this](const Kunlaboro::Message&) {
    			// Draw order sorting based off of Y position,
    			// using direct pointer access.
    			changeRequestPriority("Draw", phys->getY());
    		});
    	});

    	requestMessage("Draw", &Drawable::Draw);
    }

    void draw(const Kunlaboro::Message&)
    {
    	// Ask for the position using message passing, maybe it's not physical.
    	auto msg = sendQuestion("GetPosition");
    	if (!msg.handled)
    		return;

    	auto pos = boost::any_cast<std::tuple<float, float> >(msg.payload);

    	// Draw it here using the position
    }

private:
	Physical* phys;
};

int main()
{
    Kunlaboro::EntitySystem es;

    // Registration
    es.registerComponent<Physical>("Physical");
    es.registerComponent<Drawable>("Drawable");
    es.registerTemplate("Entity", { "Physical", "Drawable" });

    // Creation
    auto eid = es.createEntity("Entity");
    es.sendLocalMessage(eid, "SetPosition", std::make_tuple(12.f, 15.f));
    es.sendLocalMessage(eid, "SetDelta", std::make_tuple(1.f, 0.5f));

    // Main loop
    while (true)
    {
    	es.sendGlobalMessage("Update");

    	es.sendGlobalMessage("Draw");
    }

    return 0;
}
```
