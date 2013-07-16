//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfFeatherMosaic.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfFeatherMosaic_HEADER
#define rspfFeatherMosaic_HEADER
#include <vector>
#include <iterator>

#include <rspf/imaging/rspfImageMosaic.h>
#include <rspf/base/rspfDpt.h>


/**
 * Performs a spatial blend accross overlapping regions
 */
class RSPFDLLEXPORT rspfFeatherMosaic : public rspfImageMosaic
{
public:
   rspfFeatherMosaic();
   rspfFeatherMosaic(rspfConnectableObject::ConnectableObjectList& inputSources);
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& origin,
                                   rspf_uint32 resLevel=0);
   virtual void initialize();
       
protected:
   virtual ~rspfFeatherMosaic();
   /**
    * This is only visible by this class and will remain here
    * unless we want to move it later.
    */
   class rspfFeatherInputInformation
   {
      
   public:
      rspfFeatherInputInformation(const vector<rspfIpt>& validVertices=vector<rspfIpt>());

      void setVertexList(const vector<rspfIpt>& validVertices);
      
      rspfDpt        theCenter;
      rspfDpt        theAxis1;
      rspfDpt        theAxis2;
      double          theAxis1Length;
      double          theAxis2Length;
      std::vector<rspfIpt> theValidVertices;
      
   };

   friend ostream& operator<<(ostream& out,
                              const rspfFeatherInputInformation& data);
   

   /**
    * will hold an array of input information
    *
    */
   rspfFeatherInputInformation* theInputFeatherInformation;
   rspfRefPtr<rspfImageData> theAlphaSum;
   rspfRefPtr<rspfImageData> theResult;
   
   /**
    * Will hold the count for the feather information list.
    */
   long                          theFeatherInfoSize;

   /**
    * The dummy variable is used for the template type.  See the getTile
    * method for the call.
    */
   template <class T>
   rspfRefPtr<rspfImageData> combine(T dummyVariableNotUsed,
                                       const rspfIrect& tileRect,
                                       rspf_uint32 resLevel);
   
   virtual double computeWeight(long index,
                                const rspfDpt& point)const;
TYPE_DATA
};

#endif /* #ifndef rspfFeatherMosaic_HEADER */
