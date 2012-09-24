/*  Kunlaboro

    The MIT License (MIT)
    Copyright (c) 2012 Alexander Olofsson (ace@haxalot.com)
 
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _KUNLABORO_HPP
#define _KUNLABORO_HPP

#include "Component.hpp"
#include "EntitySystem.hpp"

#endif // _KUNLABORO_HPP

/** \mainpage

\section welcome Welcome
So I see you've stumbled upon this little project of mine.
Kunlaboro, which is esperanto for 'cooperation', is a C++ Entity System designed around a heavily modified RDBMS.

It is designed to have a close to cost-free message passing system for communication between components, as well as a way for components to store pointers to each other for direct access.

\section getting Getting the code
The code for Kunlaboro can be found on github at this link: https://github.com/ace13/Kunlaboro.
With the code in your hand you can use the provided CMakeList to generate project files for your favourite platform.

Feel free to send me issues, and/or patches for any problems you find in my code. Or provide me with constructive critisism, as I doubt this to be the most optimal code possible.

Kunlaboro is designed to easily fit into your project and is therefore set up to be statically compiled as a small library.

\section requirements Requirements
Kunlaboro is built with many of the c++0x/c++11 features that were provided both in the tr1 release, and before.
Therefor Kunlaboro requires a reasonably modern compiler to work properly, it's been tested and made sure to compile with both Microsoft Visual Studio 2010 and GCC 4.6.3.

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
    PrintComponent() : Kunlaboro::Component("Print") { }

    void printString(const Message& msg)
    {
        cout << boost::any_cast<std::string>(msg.payload) << endl;
    }

    void addedToEntity()
    {
        registerMessage("Print.PrintString", &PrintComponent::printString);
    }
};

Component* createPrint() { return new PrintComponent(); }

int main()
{
    EntitySystem sys;
    sys.registerComponent("Print", &createPrint);

    EntityId eId = sys.createEntity();
    sys.addComponent(eId, sys.createComponent("Print"));
    sys.finalizeEntity(eId);

    sys.sendGlobalMessage("Print.PrintString", "Hello World!");

    return 0;
}
\endcode


*/