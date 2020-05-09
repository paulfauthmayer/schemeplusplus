#include "scheme.hpp"

void scm_print(scmObject object)
{
    switch(object -> tag)
    {
    case SCM_INTEGER:
        std::cout << object->u.intValue;
        break;
    case SCM_STRING:
        std::cout << object->u.stringValue;
        break;
    default:
        std::cout << "not yet implemented: " << object->tag;
        break;
    }
}