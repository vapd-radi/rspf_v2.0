#ifndef rspfEastingNorthingCutter_HEADER
#define rspfEastingNorthingCutter_HEADER
#include <rspf/imaging/rspfRectangleCutFilter.h>
#include <rspf/base/rspfViewInterface.h>

class rspfProjection;
class rspfEastingNorthingCutter : public rspfRectangleCutFilter,
                                   public rspfViewInterface
{
public:
   rspfEastingNorthingCutter(rspfObject* owner,
                              rspfImageSource* inputSource=NULL);
   rspfEastingNorthingCutter(rspfImageSource* inputSource=NULL);
   
   void setEastingNorthingRectangle(const rspfDpt& ul,
                                    const rspfDpt& lr);
   rspfDpt getUlEastingNorthing()const;
   rspfDpt getLrEastingNorthing()const;
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
   virtual bool setView(rspfObject* baseObject);

   virtual rspfObject* getView();
   virtual const rspfObject* getView()const;

   virtual void initialize();
   
protected:
   virtual ~rspfEastingNorthingCutter();
   rspfDpt theUlEastingNorthing;
   rspfDpt theLrEastingNorthing;
   
   rspfRefPtr<rspfProjection> theViewProjection;


   void transformVertices();
TYPE_DATA
};

#endif
