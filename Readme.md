Kunlaboro
=========

So I see you've stumbled upon this little project of mine.
Kunlaboro, which is esperanto for 'cooperation', is a C++ Entity System designed around a heavily modified RDBMS.

It is designed to have a close to cost-free message passing system for communication between components, as well as a way for components to store pointers to each other for direct access.

Getting the code
----------------

The code for Kunlaboro can be found on github at this link: https://github.com/ace13/Kunlaboro.

With the code in your hand you can use the provided CMakeList to generate project files for your favourite platform.
Feel free to send me issues, and/or patches for any problems you find in my code. Or provide me with constructive critisism, as I doubt this to be the most optimal code possible.

Kunlaboro is designed to easily fit into your project and is therefore set up to be statically compiled as a small library.

Requirements
------------

Kunlaboro is built with many of the c++0x/c++11 features that were provided both in the tr1 release, and before.
Therefore Kunlaboro requires a reasonably modern compiler to work properly, it's been tested and made sure to compile with both Microsoft Visual Studio 2010 and GCC 4.6.3.

Documentation
-------------

The documentation for Kunlaboro can be found in the source files, a Doxyfile has been provided if you want some better formatted documentation.