#ifndef rspfMapViewController_HEADER
#define rspfMapViewController_HEADER
#include <rspf/base/rspfViewController.h>
#include <rspf/base/rspfFilename.h>
class rspfMapViewController : public rspfViewController
{
public:
   rspfMapViewController();
   rspfMapViewController(rspfObject* owner,
                          rspf_uint32 inputListSize,
                          rspf_uint32 outputListSize,
                          bool         inputListFixedFlag=true,
                          bool         outputListFixedFlag=false);
   virtual ~rspfMapViewController();
   virtual bool setView(rspfObject* object);
   virtual void changeOwner(rspfObject* owner);
   virtual void setFilename(const rspfFilename& file);
   virtual const rspfFilename& getFilename()const
      {
         return theGeometryFile;
      }
   
   virtual rspfString getShortName()const
      {
         return rspfString("Map View");
      }
   
   virtual rspfString getLongName()const
      {
         return rspfString("Map View controller");
      }
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
protected:
   /*!
    * If the file is not "" (empty) then it will
    * use this filename to load and save the geometry
    * projection.
    */
   rspfFilename theGeometryFile;
   
TYPE_DATA
};
#endif
