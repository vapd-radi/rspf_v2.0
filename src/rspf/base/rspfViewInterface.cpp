#include <rspf/base/rspfViewInterface.h>

RTTI_DEF(rspfViewInterface, "rspfViewInterface");

rspfViewInterface::rspfViewInterface()
   : theObject(NULL)
{}

rspfViewInterface::rspfViewInterface(rspfObject* base)
   : theObject(base)
{}

rspfViewInterface::~rspfViewInterface()
{}

void rspfViewInterface::refreshView()
{}

rspfObject* rspfViewInterface::getBaseObject()
{
   return theObject;
}
