// $Id: rspfVisitor.cpp 22158 2013-02-20 12:29:10Z gpotts $

#include <rspf/base/rspfVisitor.h>
#include <rspf/base/rspfConnectableObject.h>
#include <rspf/base/rspfPropertyEvent.h>
#include <rspf/base/rspfViewInterface.h>


rspfVisitor::rspfVisitor(int visitorType)
:m_visitorType(static_cast<VisitorType>(visitorType)),
m_stopTraversalFlag(false)
{
   
}

rspfVisitor::rspfVisitor(const rspfVisitor& src)
:m_visitorType(src.m_visitorType),
m_markNode(src.m_markNode),
m_stopTraversalFlag(src.m_stopTraversalFlag)
{
}

rspfVisitor::VisitorType rspfVisitor::getVisitorType()const
{
   return m_visitorType;
}

void  rspfVisitor::setVisitorType(int vType, bool on)
{
   if(on)
   {
      m_visitorType = (VisitorType)((vType|m_visitorType)&VISIT_ALL);
   }
   else 
   {
      m_visitorType = (VisitorType)(((~vType) & m_visitorType)&VISIT_ALL);
   }
}

void rspfVisitor::turnOffVisitorType(int vType)
{
   setVisitorType(vType, false);
}

void rspfVisitor::reset()
{
   m_markNode.clear();
   m_stopTraversalFlag = false;
}

void rspfVisitor::visit(rspfObject* obj)
{
   m_markNode.insert(obj);
}

void rspfVisitor::visit(rspfConnectableObject* obj)
{
   visit(static_cast<rspfObject*>(obj));
}

bool rspfVisitor::hasVisited(rspfObject* obj)const
{
   return m_markNode.find(obj) != m_markNode.end(); 
}

bool rspfVisitor::stopTraversal()const
{
   return m_stopTraversalFlag;
}


rspfIdVisitor::rspfIdVisitor()
   :
   rspfVisitor(rspfVisitor::VISIT_INPUTS|rspfVisitor::VISIT_CHILDREN),
   m_object(0),
   m_id(0)
{
}

rspfIdVisitor::rspfIdVisitor(int visitorType)
   :
   rspfVisitor(visitorType),
   m_object(0),
   m_id(0)
{
}

rspfIdVisitor::rspfIdVisitor(const rspfId& id, int visitorType)
   :
   rspfVisitor(visitorType),
   m_object(0),
   m_id(id)
{
}

rspfIdVisitor::rspfIdVisitor(const rspfIdVisitor& src)
   :
   rspfVisitor(src),
   m_object(src.m_object),
   m_id(src.m_id)
{
}

rspfRefPtr<rspfVisitor> rspfIdVisitor::dup()const
{
   return new rspfIdVisitor(*this);
}

void rspfIdVisitor::visit(rspfConnectableObject* obj)
{
   if ( !hasVisited(obj) )
   {
      if(obj->getId() == m_id)
      {
         m_object = obj;
         m_stopTraversalFlag = true;
      }
      rspfVisitor::visit(obj);
   }
}

void rspfIdVisitor::reset()
{
   rspfVisitor::reset();
   m_object = 0;
}

rspfConnectableObject* rspfIdVisitor::getObject()
{
   return m_object;
}

const rspfConnectableObject* rspfIdVisitor::getObject()const
{
   return m_object;
}

void rspfIdVisitor::setId(const rspfId& id)
{
   m_id = id;
}

const rspfId& rspfIdVisitor::getId()const
{
   return m_id;
}

rspfCollectionVisitor::rspfCollectionVisitor(int visitorType)
:rspfVisitor(visitorType)
{
}

rspfCollectionVisitor::rspfCollectionVisitor(const rspfCollectionVisitor& src)
:m_collection(src.m_collection)
{
}

rspfCollectionVisitor::ListRef& rspfCollectionVisitor::getObjects()
{
   return m_collection;
}

const rspfCollectionVisitor::ListRef& rspfCollectionVisitor::getObjects()const
{
   return m_collection;
}

rspfObject* rspfCollectionVisitor::getObject(rspf_uint32 idx)
{
   if(idx < m_collection.size())
   {
      return m_collection[idx].get();
   }
   return 0;
}

const rspfObject* rspfCollectionVisitor::getObject(rspf_uint32 idx)const
{
   if(idx < m_collection.size())
   {
      return m_collection[idx].get();
   }
   return 0;
}

void rspfCollectionVisitor::reset()
{
   rspfVisitor::reset();
   m_collection.clear();
}

rspfTypeNameVisitor::rspfTypeNameVisitor(int visitorType)
   :rspfCollectionVisitor(visitorType),
    m_typeName(),
    m_firstOfTypeFlag(false)
{
}

rspfTypeNameVisitor::rspfTypeNameVisitor(const rspfString& typeName,
                                           bool firstOfTypeFlag,
                                           int visitorType)
   :
   rspfCollectionVisitor(visitorType),
   m_typeName(typeName),
   m_firstOfTypeFlag(firstOfTypeFlag)
{
}

rspfTypeNameVisitor::rspfTypeNameVisitor(const rspfTypeNameVisitor& src)
:m_typeName(src.m_typeName),
 m_firstOfTypeFlag(src.m_firstOfTypeFlag)
{
}

rspfRefPtr<rspfVisitor> rspfTypeNameVisitor::dup()const
{
   return new rspfTypeNameVisitor(*this);
}

void rspfTypeNameVisitor::visit(rspfObject* obj)
{
   if(!hasVisited(obj))
   {
      rspfCollectionVisitor::visit(obj);
      if(obj&&obj->canCastTo(m_typeName))
      {
         m_collection.push_back(obj);
         if(m_firstOfTypeFlag) m_stopTraversalFlag = true;
      }
  }
}

void rspfTypeNameVisitor::setTypeName(const rspfString& typeName)
{
   m_typeName = typeName;
}

const rspfString& rspfTypeNameVisitor::getTypeName()const
{
   return m_typeName;
}

void rspfTypeNameVisitor::setFirstOfTypeFlag(bool flag)
{
   m_firstOfTypeFlag = flag;
}

bool rspfTypeNameVisitor::getFirstOfTypeFlag() const
{
   return m_firstOfTypeFlag;
}

rspfTypeIdVisitor::rspfTypeIdVisitor(int visitorType)
:rspfCollectionVisitor(visitorType)
{
}

rspfTypeIdVisitor::rspfTypeIdVisitor(const RTTItypeid& typeId, bool firstOfTypeFlag, int visitorType)
:rspfCollectionVisitor(visitorType),
m_typeId(typeId),
m_firstOfTypeFlag(firstOfTypeFlag)
{
}

rspfTypeIdVisitor::rspfTypeIdVisitor(const rspfTypeIdVisitor& src)
:m_typeId(src.m_typeId),
m_firstOfTypeFlag(src.m_firstOfTypeFlag)
{
}

rspfRefPtr<rspfVisitor> rspfTypeIdVisitor::dup()const
{
   return new rspfTypeIdVisitor(*this);
}

void rspfTypeIdVisitor::setTypeId(const RTTItypeid& typeId)
{
   m_typeId = typeId;
}

const RTTItypeid& rspfTypeIdVisitor::getTypeId()const
{
   return m_typeId;
}

void rspfTypeIdVisitor::visit(rspfObject* obj)
{
   if(!hasVisited(obj))
   {
      rspfCollectionVisitor::visit(obj);
      if(obj&&obj->canCastTo(m_typeId))
      {
         m_collection.push_back(obj);
         if(m_firstOfTypeFlag) m_stopTraversalFlag = true;
      }
   }
}

rspfEventVisitor::rspfEventVisitor(rspfEvent* evt, int visitorType)
:rspfVisitor(visitorType),
m_event(evt)
{
}
rspfEventVisitor::rspfEventVisitor(const rspfEventVisitor& src)
:rspfVisitor(src),
m_event(src.m_event)
{
}

rspfRefPtr<rspfVisitor> rspfEventVisitor::dup()const
{
   return new rspfEventVisitor(*this);
}

void rspfEventVisitor::visit(rspfObject* obj)
{
   if(!hasVisited(obj))
   {
      rspfListenerManager* manager = dynamic_cast<rspfListenerManager*> (obj);
      if(manager)
      {
         manager->fireEvent(*m_event);
         rspfVisitor::visit(obj);
      }
   }
}

rspfViewInterfaceVisitor::rspfViewInterfaceVisitor(rspfObject* view, int visitorType)
   :
   rspfVisitor(visitorType),
   m_view(view)
{
}

rspfViewInterfaceVisitor::rspfViewInterfaceVisitor(const rspfViewInterfaceVisitor& src)
   :
   rspfVisitor(src),
   m_view(src.m_view)
{
}

rspfRefPtr<rspfVisitor> rspfViewInterfaceVisitor::dup()const
{
   return new rspfViewInterfaceVisitor(*this);
}

rspfObject* rspfViewInterfaceVisitor::getView()
{
   return m_view.get();
}

const rspfObject* rspfViewInterfaceVisitor::getView()const
{
   return m_view.get();
}

void rspfViewInterfaceVisitor::visit(rspfObject* obj)
{
   if( !hasVisited(obj) )
   {
      if ( m_view.get() )
      {
         rspfViewInterface* vi = dynamic_cast<rspfViewInterface*>(obj);
         if ( vi )
         {
            // Set the view:
            vi->setView( m_view.get() );

            // Send property event for outputs connected to us.
            rspfRefPtr<rspfEvent> propEvent = new rspfPropertyEvent(obj);
            rspfEventVisitor ev( propEvent.get(), VISIT_OUTPUTS );
            obj->accept(ev);
         }
      }
      
      // This will put the obj in the m_markNode for hasVisited method.
      rspfVisitor::visit(obj);
   }
}
