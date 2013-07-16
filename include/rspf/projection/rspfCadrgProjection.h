#ifndef rspfCadrgProjection_HEADER
#define rspfCadrgProjection_HEADER
#include <rspf/projection/rspfMapProjection.h>
class RSPF_DLL rspfCadrgProjection : public rspfMapProjection
{
public:
   rspfCadrgProjection();
                       
   virtual rspfObject *dup()const;
   virtual rspfGpt inverse_do(const rspfDpt &eastingNorthing,const rspfGpt &latLon)const;
   virtual rspfGpt inverse(const rspfDpt &eastingNorthing)const;
   virtual rspfDpt forward(const rspfGpt &latLon)const;
   
   virtual rspfDpt worldToLineSample(const rspfGpt &worldPoint)    const;
   virtual rspfGpt lineSampleToWorld(const rspfDpt &projectedPoint)const;
   virtual void worldToLineSample(const rspfGpt &worldPoint,
                                  rspfDpt&       lineSample)const;
   virtual void lineSampleToWorld(const rspfDpt &projectedPoint,
                                  rspfGpt& gpt)const;
   double computeXPixConstant(double scale, long zone)const;
   double computeYPixConstant(double scale)const;
   /*!
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
   rspfString getProjectionName()const
      {
         return getClassName();
      }
      
   virtual bool operator==(const rspfProjection& projection) const;
protected:
   virtual ~rspfCadrgProjection();
   /*
    * Are from the ADRG Mil spec for 1:1,000,000 and
    * are scales for each zone.
    */
   static double theCadrgArcA[8];
   
   static double theOldZoneExtents[10];
   static double theNorthLimit;
   static double theSouthLimit;
   rspf_int32   theCadrgZone;
   rspfDpt thePixelConstant;
   double   theMapScale;
   double theWidth;
   double theHeight;
   rspfDpt theUlLineSample;
   rspfGpt theUlGpt;
   rspfGpt theLlGpt;
   rspfGpt theLrGpt;
   rspfGpt theUrGpt;
   
   void computeParameters();
TYPE_DATA
};
#endif
