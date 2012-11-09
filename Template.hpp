/*  Kunlaboro - Template

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

#ifndef _KUNLABORO_TEMPLATE_HPP
#define _KUNLABORO_TEMPLATE_HPP

#include "Component.hpp"
#include <string>
#include <vector>

namespace Kunlaboro
{
    class EntitySystem;

    /** \brief A base class containing a template for creating an entire entity.
     * 
     *
     */
    class Template : public Component
    {
    public:
        virtual ~Template();

        void addedToEntity();

        /** \brief When overridden, this function should return a string represantation of the Template.
         *
         * The data this function returns should be enough to deserialize it again to an
         * acceptable copy.
         *
         * \returns A data string containing the template data.
         */
        virtual std::string serialize();
        /** \brief When overridden, this function should use the provided data to create the Template.
         *
         * 
         * 
         * \param data The data string containing the Template.
         */
        virtual bool deserialize(const std::string& data);

        /** \brief When overridden, this function will be called when an entitiy has been created and is
         * ready to accept data.
         * 
         * 
         */
        virtual void instanceCreated();

    protected:
        Template(const std::string& name);

        /** \brief Add a component to the Template.
         *
         * \param name The name of the component to add.
         */
        void addComponent(const std::string& name);

    private:
        std::vector<std::string> mComponents;
    };

}

#endif