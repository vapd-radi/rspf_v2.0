//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Oscar Kramer (okramer@imagelinks.com)
//
// DESCRIPTION: Contains declaration of class rspfAtbMatchPoint. This class
//    is used by the automated tonal balancing code for bookkeeping and
//    statistics associated with match-points. Match points are points shared in
//    common between two or more images being adjusted. 
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfAtbMatchPoint.h 15766 2009-10-20 12:37:09Z gpotts $

#ifndef rspfAtbMatchPoint_HEADER
#define rspfAtbMatchPoint_HEADER

#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfReferenced.h>
#include <vector>
using namespace std;

class rspfAtbPointSource;
class rspfGridRemapEngine;
class rspfGridRemapSource;

/*!****************************************************************************
 *
 * CLASS:  rspfAtbMatchPoint
 *
 *****************************************************************************/
class rspfAtbMatchPoint : public rspfReferenced
{
public:
   rspfAtbMatchPoint()
      : theGridRemapEngine(0) {}
   
   rspfAtbMatchPoint(const rspfDpt& view_point,
                      rspfGridRemapEngine* engine)
      : theViewPoint(view_point), theGridRemapEngine(engine) {}

   
   /*!
    * Returns the point in view coordinates associated with this object.
    */
   const rspfDpt& viewPoint() const { return theViewPoint; }

   /*!
    * Adds a new contribution to the sample set.
    */
   void addImage(rspfGridRemapSource* image_source);

   /*!
    * The target parameter value computed given all contributions. Returns true
    * if a valid quantity is returned.
    */
   bool assignRemapValues();

   /*!
    * Hook to set the size of the kernel used by all point sources in computing
    * their mean pixel value. The kernels will be resized to NxN.
    */
   void setKernelSize(int side_size);

   /*!
    * Sets the ATB remap engine reference owned by the controller. This engine
    * is the only object that understands how to compute remap parameters from
    * pixel data. This permits easily modifying the ATB algorithm without
    * the support classes such as this one.
    */
   void setGridRemapEngine(rspfGridRemapEngine* engine);

   /*!
    * Dumps the contents of the object to the stream in human readable format.
    */
//   void print(ostream& os) const;

//   friend ostream& operator << (ostream& os);

protected:
   virtual ~rspfAtbMatchPoint();
   
   rspfDpt                     theViewPoint;
   vector<rspfAtbPointSource*> thePointSourceList; 
   rspfGridRemapEngine*        theGridRemapEngine;
};

#endif
