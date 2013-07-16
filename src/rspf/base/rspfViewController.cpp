#include <rspf/base/rspfViewController.h>
#include <rspf/base/rspfViewInterface.h>
#include <rspf/base/rspfConnectableContainerInterface.h>
#include <rspf/base/rspfListenerManager.h>
#include <rspf/base/rspfPropertyEvent.h>
#include <rspf/base/rspfNotifyContext.h>
#include <vector>
using namespace std;

RTTI_DEF1(rspfViewController, "rspfViewController", rspfSource);

rspfViewController::rspfViewController()
   :
      rspfSource((rspfObject*)NULL, 0, 0, true, false)
{
   theView = NULL;
}

rspfViewController::rspfViewController(rspfObject* owner,
                                         rspf_uint32 inputListSize,
                                         rspf_uint32 outputListSize,
                                         bool   inputListFixedFlag,
                                         bool   outputListFixedFlag)
   :
      rspfSource(owner,
                  inputListSize,
                  outputListSize,
                  inputListFixedFlag,
                  outputListFixedFlag)
{
   theView = NULL;
}

rspfViewController::~rspfViewController()
{
   theView = 0;
}

rspfString rspfViewController::getShortName()const
{
   return rspfString("View");
}

 rspfString rspfViewController::getLongName()const
{
   return rspfString("View controller");
}

bool rspfViewController::canConnectMyInputTo(rspf_int32,
                                              const rspfConnectableObject*)const
{
   return false;
}

bool rspfViewController::propagateView()
{
   bool returnResult = true;
   if(!theOwner)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL: " << getClassName() << "::propagateView(), has no owner! We must find all views to set.\n";
      return false;
   }
   rspfConnectableContainerInterface* inter = PTR_CAST(rspfConnectableContainerInterface,
                                                        theOwner);
   if(inter)
   {
      RTTItypeid typeId = STATIC_TYPE_INFO(rspfViewInterface);
      rspfConnectableObject::ConnectableObjectList result = inter->findAllObjectsOfType(typeId,
                                                                           true);
      if(result.size() > 0)
      {
         rspf_uint32 index = 0;

         // first set all views then update all outputs
         //
         for(index = 0; index < result.size(); ++index)
         {
            rspfViewInterface* viewInterface = PTR_CAST(rspfViewInterface, result[index].get());

            if(viewInterface)
            {
               if(!viewInterface->setView(theView.get()))
               {
                  returnResult = false;
               }
            }
         }

         for(index = 0; index < result.size(); ++index)
         {
            rspfPropertyEvent event(result[index].get());
            result[index]->fireEvent(event);
            result[index]->propagateEventToOutputs(event);
         }
      }
   }

   return returnResult;
}

bool rspfViewController::setView(rspfObject* object)
{
   theView = object;

   return true;
}

rspfObject* rspfViewController::getView()
{
   return theView.get();
}

const rspfObject* rspfViewController::getView()const
{
   return theView.get();
}

const rspfObject* rspfViewController::findFirstViewOfType(RTTItypeid typeId)const
{
   rspfConnectableContainerInterface* inter = PTR_CAST(rspfConnectableContainerInterface,
                                                        theOwner);
   if(inter)
   {
      RTTItypeid viewInterfaceType = STATIC_TYPE_INFO(rspfViewInterface);
      rspfConnectableObject::ConnectableObjectList result = inter->findAllObjectsOfType(viewInterfaceType,
                                                                           true);
      if(result.size() > 0)
      {
         rspf_uint32 index = 0;

         for(index = 0; index < result.size(); ++index)
         {

            rspfViewInterface* viewInterface = PTR_CAST(rspfViewInterface, result[index].get());

            if(viewInterface)
            {
               if(viewInterface->getView())
               {
                  if(typeId.can_cast(viewInterface->getView()->getType()))
                  {
                     return viewInterface->getView();
                  }
               }
            }
         }
      }
   }

   return (rspfObject*)NULL;
}
