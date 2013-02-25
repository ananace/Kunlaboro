#ifndef _KUNLABORO_HPP
#define _KUNLABORO_HPP

#include "Component.hpp"
#include "EntitySystem.hpp"
#include "Template.hpp"

#endif // _KUNLABORO_HPP

/** \mainpage

\section welcome Welcome
So I see you've stumbled upon this little project of mine.
Kunlaboro, which is esperanto for 'cooperation', is a C++ Entity System designed around a heavily modified RDBMS.

It is designed to have a close to cost-free message passing system for communication between components, as well as a way for components to store pointers to each other for direct access.

\section getting Getting the code
The code for Kunlaboro can be found on github at this link: https://github.com/ace13/Kunlaboro.
With the code in your hand you can use the provided CMakeList to generate project files for your favourite platform.

Feel free to send me issues, and/or patches for any problems you find in my code. Or provide me with constructive critisism, as I doubt this to be the most optimal code possible. Github has an excellent issue tracker for this exact function, also feel free to fork it and make your own changes to the code.

Kunlaboro is designed to easily fit into your project and is therefore set up to be statically compiled as a small library, perfect to just stick into your already existing git project as a simple submodule.

\section requirements Requirements
Kunlaboro is built with many of the c++0x/c++11 features that were provided both in the tr1 release, and before.
Therefore Kunlaboro requires a reasonably modern compiler to work properly, it's been tested and made sure to compile with both Microsoft Visual Studio 2010 and GCC 4.6.3.

\section usage Code Example
Here's a simple example of a Kunlaboro application:

\code
#include <Kunlaboro/Kunlaboro.hpp>
#include <iostream>

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

int main()
{
    EntitySystem sys;
    sys.registerComponent<PrintComponent>("Print");

    EntityId eId = sys.createEntity();
    sys.addComponent(eId, sys.createComponent("Print"));
    sys.finalizeEntity(eId);

    sys.sendGlobalMessage("Print.PrintString", "Hello World!");
    sys.sendMessageToEntity(eId, "Print.PrintString", "Hello Local World!");

    return 0;
}
\endcode

More examples are available in the \a Component and \a EntitySystem documentation.

*/
