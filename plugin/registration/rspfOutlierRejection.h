//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Frederic Claudel (fclaudel@laposte.net)
//         ICT4EO,Meraka Institute,CSIR
//
// Description: generic model optimization with RANSAC, high-level interface
//
//*************************************************************************
#ifndef rspfOutlierRejection_HEADER
#define rspfOutlierRejection_HEADER

#include "rspfModelOptimizer.h"

/*!
 * class rspfOutlierRejection
 * class for optimizing a model, while dealing with outliers
 *
 * exports a model and a set of tie points (opt)
 * you can use rspfImageCorrelator to get a tie point input (object rspfTieGptSet)
 */
class RSPF_REGISTRATION_DLL rspfOutlierRejection : public rspfModelOptimizer
{
public:
   rspfOutlierRejection();
   virtual inline ~rspfOutlierRejection() {}

  /**
   *accessors to parms
   */   
   inline void  setInlierRatio(const rspf_float64& v) { theInlierRatio = v; }
   inline const rspf_float64& getInlierRatio()const { return theInlierRatio; }

   inline void  setInlierImageAccuracy(const rspf_float64& v) { theInlierImageAccuracy = v; }
   inline const rspf_float64& getInlierImageAccuracy()const { return theInlierImageAccuracy; }

   /*!
    * export (modified) tie point set
    */
   bool saveGMLTieSet(const rspfString& filepath);

   /*!
    * remove outliers (using RANSAC)
    * the projection will be modified, and fitted to inliers if success
    *
    * RETURNS: true for success
    *          variance , unit pixel^2 (optional, if pointer not NULL)
    * INPUT:  target variance in pixel^2 (optional, and not used so far - feb 2006)
    */
   virtual bool removeOutliers(rspf_float64* result_var_pix2 = NULL,
                               rspf_float64* target_var_pix2 = NULL);

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   virtual bool execute();
   
protected:
   rspf_float64 theInlierRatio;         //!between 0 and 1
   rspf_float64 theInlierImageAccuracy; //!unit pixels
   rspfFilename theInlierOutputFilename;
   //disable copy constructor
   inline rspfOutlierRejection(const rspfOutlierRejection& copyme) {}
   
TYPE_DATA   
};

#endif //rspfOutlierRejection_HEADER
