// $Id: rspfVisitor.h 22158 2013-02-20 12:29:10Z gpotts $

#ifndef rspfVisitor_HEADER
#define rspfVisitor_HEADER 1
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfId.h>
#include <rspf/base/rspfEvent.h>
#include <rspf/base/rspfRefPtr.h>
#include <set>
#include <vector>

class rspfConnectableObject;


class RSPF_DLL rspfVisitor : public rspfReferenced
{
public:
   typedef std::set<rspfRefPtr<rspfObject> >    MarkCollectionRef;
   typedef std::set<rspfRefPtr<rspfObject> >    CollectionRef;
   typedef std::vector<rspfRefPtr<rspfObject> > ListRef;
   typedef std::set<rspfObject*>                 Collection;
   typedef std::vector<rspfObject* >             List;
   /**
    * Enumeration type can be a mask and will traverse a graph of connectables based on the values.
    */
   enum VisitorType
   {
      VISIT_NONE     = 0,
      VISIT_CHILDREN = 1,
      VISIT_INPUTS   = 2,
      VISIT_OUTPUTS  = 4,
      VISIT_ALL      = (VISIT_CHILDREN|VISIT_INPUTS|VISIT_OUTPUTS)
   };
   
   rspfVisitor(int visitorType = VISIT_NONE);
   rspfVisitor(const rspfVisitor& src);
   virtual void reset();
   virtual rspfRefPtr<rspfVisitor> dup()const=0;
   virtual void visit(rspfObject* obj);
   virtual void visit(rspfConnectableObject* obj);
   VisitorType getVisitorType()const;
   void setVisitorType(int vType, bool on=true);
   void turnOffVisitorType(int vType);
   bool hasVisited(rspfObject* obj)const;
   bool stopTraversal()const;
protected:
   VisitorType            m_visitorType;
   mutable Collection     m_markNode;
   mutable bool           m_stopTraversalFlag;
};

class RSPF_DLL rspfCollectionVisitor : public rspfVisitor
{
public:
   rspfCollectionVisitor(int visitorType =(VISIT_INPUTS|VISIT_CHILDREN));
   rspfCollectionVisitor(const rspfCollectionVisitor& src);
   ListRef& getObjects();
   const ListRef& getObjects()const;
   rspfObject* getObject(rspf_uint32 idx = 0);
   const rspfObject* getObject(rspf_uint32 idx = 0)const;

   // Inlined to force template instantiation.
   template <class T> T* getObjectAs(rspf_uint32 idx=0)
   {
      if(idx < m_collection.size())
      {
         return dynamic_cast<T*>(m_collection[idx].get());
      }
      return 0;
   }

   // Inlined to force template instantiation.
   template<class T> const T* getObjectAs(rspf_uint32 idx=0)const
   {
      if(idx < m_collection.size())
      {
         return dynamic_cast<const T*>(m_collection[idx].get());
      }
      return 0;
   }

   virtual void reset();
 
protected:
   ListRef m_collection; 
   
};

class RSPF_DLL rspfIdVisitor : public rspfVisitor
{
public:
   /** 
    * @brief Default constructor
    * Constructs with id of 0 and vistor type of VISIT_INPUTS|VISIT_CHILDREN.
    */
   rspfIdVisitor();

   rspfIdVisitor(int visitorType);
   rspfIdVisitor(const rspfId& id, int visitorType =(VISIT_INPUTS|VISIT_CHILDREN));
   rspfIdVisitor(const rspfIdVisitor& src);

   virtual rspfRefPtr<rspfVisitor> dup()const;
   virtual void visit(rspfConnectableObject* obj);

   /** @brief Resets m_object to 0, calls rspfVisitor::reset(). */
   virtual void reset();
   
   rspfConnectableObject* getObject();
   const rspfConnectableObject* getObject()const;
   void setId(const rspfId& id);
   const rspfId& getId()const;
protected:
   rspfConnectableObject* m_object;
   rspfId m_id;
};

class RSPF_DLL rspfTypeNameVisitor : public rspfCollectionVisitor
{
public:
   rspfTypeNameVisitor(int visitorType =(VISIT_INPUTS|VISIT_CHILDREN));
   rspfTypeNameVisitor(const rspfString& typeName, bool firstofTypeFlag=false, int visitorType =(VISIT_INPUTS|VISIT_CHILDREN));
   rspfTypeNameVisitor(const rspfTypeNameVisitor& src);
   virtual rspfRefPtr<rspfVisitor> dup()const;
   virtual void visit(rspfObject* obj);
   void setTypeName(const rspfString& typeName);
   const rspfString& getTypeName()const;

   /**
    * @brief Set the m_firstOfTypeFlag.
    *
    * @param flag If flag is true, rspfTypeNameVisitor::visit will stop traversing if it
    * finds an object of m_typeName.
    */
   void setFirstOfTypeFlag(bool flag);

   /** @return m_firstOfTypeFlag */
   bool getFirstOfTypeFlag() const;
   
protected:
   rspfString m_typeName;
   bool        m_firstOfTypeFlag;
};

class RSPF_DLL rspfTypeIdVisitor : public rspfCollectionVisitor
{
public:
   rspfTypeIdVisitor(int visitorType =(VISIT_INPUTS|VISIT_CHILDREN));
   rspfTypeIdVisitor(const RTTItypeid& typeId, bool firstofTypeFlag=false, int visitorType =(VISIT_INPUTS|VISIT_CHILDREN));
   rspfTypeIdVisitor(const rspfTypeIdVisitor& src);
   virtual rspfRefPtr<rspfVisitor> dup()const;
   virtual void visit(rspfObject* obj);
   void setTypeId(const RTTItypeid& typeId);
   const RTTItypeid& getTypeId()const;
protected:
   RTTItypeid m_typeId;
   bool        m_firstOfTypeFlag;
};

class RSPF_DLL rspfEventVisitor : public rspfVisitor
{
public:
   rspfEventVisitor(rspfEvent* evt,
                      int visitorType =(VISIT_OUTPUTS|VISIT_CHILDREN));
   rspfEventVisitor(const rspfEventVisitor& src);
   virtual rspfRefPtr<rspfVisitor> dup()const;
   virtual void visit(rspfObject* obj);
   
protected:
   rspfRefPtr<rspfEvent> m_event;
  
};

/**
 * @class rspfViewInterfaceVisitor
 *
 * Visitor to update view and then propagate property event to outputs.
 */
class RSPF_DLL rspfViewInterfaceVisitor : public rspfVisitor
{
public:
   rspfViewInterfaceVisitor(rspfObject* view,
                             int visitorType =(VISIT_OUTPUTS|VISIT_CHILDREN));
   rspfViewInterfaceVisitor(const rspfViewInterfaceVisitor& src);
   virtual rspfRefPtr<rspfVisitor> dup()const;

   /**
    * @brief Calls rspfViewInterface::setView if obj cast to
    * rspfViewInterface*.  On success a property event is sent to
    * output side so that things like rspfCombiner can reinitialize if
    * needed.
    *
    * @param obj Object to visit.
    */
   virtual void visit(rspfObject* obj);
   

   rspfObject* getView();
   const rspfObject* getView()const;
protected:
   rspfRefPtr<rspfObject> m_view;
};

#endif
