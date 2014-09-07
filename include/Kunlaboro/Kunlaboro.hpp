#ifndef _KUNLABORO_HPP
#define _KUNLABORO_HPP

#include <Kunlaboro/Component.hpp>
#include <Kunlaboro/EntitySystem.hpp>

#endif // _KUNLABORO_HPP

/** \mainpage

\section kunlaboro Kunlaboro
Kunlaboro is an Entity System loosely based on the RDBMS-beta paradigm, and written in C++11.

It started as a simple test of entity systems, but has by now grown into something I find myself using in many of my personal projects.

\section getting Getting the code
The code for Kunlaboro can be found on my github at this link: https://github.com/ace13/Kunlaboro. There you'll also find the issue tracker among other things.

It's built using CMake to allow for easier cross-platform development, so you can generate solutions for your favourite build systems.

\section requirements Requirements
Kunlaboro requires a reasonably modern compiler that supports C++11 to a great degree, tested examples of such compilers are: Visual Studio 2013, GCC 4.6.3, and Clang 4.3.
It will probably compile and run on earlier compilers too, but they are untested and may give unexpected results.

Right now Kunlaboro also uses Boost::any to store data in message passing, but that will likely change in the near future.
If you don't feel like using Boost, then messages are set to fall back to passing void pointers.

\section usage Code Example
Here's a simple example of a Kunlaboro application using Boost for payloads:

\code
#include <Kunlaboro/Kunlaboro.hpp>
#include <iostream>
#include <tuple>

using namespace Kunlaboro;
using namespace std;

class PrintComponent : public Component
{
public:
    PrintComponent() : Component("Print") { }

    void printString(const Message& msg)
    {
        cout << boost::any_cast<std::string>(msg.payload) << endl;
    }

    void addedToEntity()
    {
        registerMessage("Print.PrintString", &PrintComponent::printString);
    }
};

// You can also use C++11 features without problems
class AddComponent : public Component
{
public:
    AddComponent : Component("Add") { }

    void addedToEntity()
    {
        registerMessage("Add.AddNumbers", [](const Kunlaboro::Message& msg)
        {
            auto tuple = boost::any_cast<std::tuple<int, float>>(msg.payload);

            float return = std::get<0>(tuple) + std::get<1>(tuple);

            msg.handle(return);
        });
    }
}

int main()
{
    EntitySystem sys;
    sys.registerComponent<PrintComponent>("Print");
    sys.registerComponent<AddComponent>("Add");

    sys.registerTemplate("Example", { "Print", "Add" });

    EntityId eId = sys.createEntity("Example");

    sys.sendGlobalMessage("Print.PrintString", "Hello World!");
    sys.sendMessageToEntity(eId, "Print.PrintString", "Hello Local World!");

    Message msg(Type_Message, nullptr, std::make_tuple(1, 2.5f));
    sys.sendGlobalQuestion(sys.getMessageRequestId("Add.AddNumbers"), msg);
    if (msg.handled)
        std::cout << "1 + 2.5 = " << boost::any_cast<float>(msg.payload) << std::endl;

    return 0;
}
\endcode

More examples are available in the \a Component and \a EntitySystem documentation.

*/
