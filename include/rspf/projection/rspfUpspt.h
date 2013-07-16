#ifndef rspfUpspt_HEADER
#define rspfUpspt_HEADER
class rspfUtmpt;
class rspfGpt;
class rspfEcefPoint;
class rspfDatum;
class rspfUpspt
{
public:
   rspfUpspt(const rspfGpt &aPt);
   rspfUpspt(const rspfEcefPoint &aPt);
   char hemisphere()const{return theHemisphere;}
   double easting()const {return theEasting;}
   double northing()const{return theNorthing;}
   const rspfDatum* datum()const{return theDatum;}
   
private:
   char    theHemisphere;
   double  theEasting;
   double  theNorthing;
   rspfDatum  *theDatum;
   void convertFromGeodetic(const rspfGpt &aPt);
};
#endif
