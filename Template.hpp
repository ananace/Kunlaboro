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