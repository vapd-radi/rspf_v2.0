//*****************************************************************************
// FILE: rspfElevCellHandler.h
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//   Contains declaration of class osimElevHandler. This class provides a
//   base-class interface for the various DEM file formats.
//
// SOFTWARE HISTORY:
//>
//   13Apr2001  Oscar Kramer
//              Initial coding.
//<
//*****************************************************************************
// $Id: rspfElevCellHandler.h 21210 2012-07-03 13:11:20Z gpotts $


#ifndef rspfElevCellHandler_HEADER
#define rspfElevCellHandler_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/elevation/rspfElevSource.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfIpt.h>

class rspfGpt;

/******************************************************************************
 *
 * CLASS:  rspfElevCellHandler
 *
 *****************************************************************************/
class RSPF_DLL rspfElevCellHandler : public rspfElevSource
{
public:
   /**
    * Constructors:
    */

   rspfElevCellHandler (const char* elev_filename);
   rspfElevCellHandler (const rspfElevCellHandler& src);


   const rspfElevCellHandler& operator=(const rspfElevCellHandler& rhs);
   virtual rspfFilename getFilename() const;
   
   
   /**
    *  METHOD:  getSizeOfElevCell 
    *  Returns the number of post in the cell.
    *  Note:  x = longitude, y = latitude
    */
   virtual rspfIpt getSizeOfElevCell() const = 0;

   /**
    *  METHOD:  getPostValue
    *  Returns the value at a given grid point as a double.
    */
   virtual double getPostValue(const rspfIpt& gridPt) const = 0;
      
   /**
    * METHOD: meanSpacingMeters()
    * Implements pure virtual for. This method is used for deterrming
    * preferred DEM among multiple overlapping elev sources:
    */
   virtual double getMeanSpacingMeters() const;

   /**
    * METHOD: pointIsInsideRect()
    * Method to check if the ground point elevation is defined:
    */
   virtual bool pointHasCoverage(const rspfGpt&) const;

   virtual bool getAccuracyInfo(rspfElevationAccuracyInfo& info, const rspfGpt& gpt) const;
   /**
    * METHODS: accuracyLE90(), accuracyCE90()
    * Returns the vertical and horizontal accuracy (90% confidence):
    */
 // virtual double getAccuracyLE90(const rspfGpt&) const;
 //  virtual double getAccuracyCE90(const rspfGpt&) const;

   bool canConnectMyInputTo(rspf_int32 inputIndex,
                            const rspfConnectableObject* object)const;
   virtual void close(){}
   virtual bool open(const rspfFilename&, bool=false){return false;}
      
protected:
   rspfElevCellHandler ();
   virtual ~rspfElevCellHandler();
   
   /**
    * Virtual method for reading 
    */
   rspfFilename        theFilename;
   double               theMeanSpacing;  // meters
   double               theAbsLE90;
   double               theAbsCE90;


   
TYPE_DATA
};

inline bool rspfElevCellHandler::pointHasCoverage(const rspfGpt& gpt) const
{
   return theGroundRect.pointWithin(gpt);
}

#endif
