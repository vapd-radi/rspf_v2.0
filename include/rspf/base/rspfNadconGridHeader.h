//**************************************************************************
// Copyright (C) 2003 Storage Area Networks, Inc.
//
// Written by:   Kenneth Melero  <kmelero@sanz.com>
//
//**************************************************************************

#ifndef rspfNadconGridHeader_HEADER
#define rspfNadconGridHeader_HEADER
#include <iostream>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfDrect.h>

class rspfNadconGridHeader
{
public:
   friend std::ostream& operator <<(std::ostream& out, const rspfNadconGridHeader& rhs);
   rspfNadconGridHeader()
      :theCols(0),
       theRows(0),
       theZ(0),
       theMinX(0),
       theDx(0),
       theMinY(0),
       theDy(0)
      {
      }
   bool readHeader(const rspfFilename& file);
   bool readHeader(std::istream& in);

   int getStartOffset()const
      {
         return (theCols+2)*4;
      }

   int getBytesPerRow()const
      {
         return (theCols+1)*4;
      }

   int getNumberOfRows()const
      {
         return theRows;
      }
   int getNumberOfCols()const
      {
         return theCols;
      }
   double getDeltaX()const
      {
         return (double)theDx;
      }
   double getDeltaY()const
      {
         return (double)theDy;
      }
   rspfDrect getBoundingRect()const
      {
         return rspfDrect(theMinX,
                           theMinY + (theRows)*theDy,
                           theMinX + (theCols)*theDx,
                           theMinY,
                           RSPF_RIGHT_HANDED);
      }
   double getMinX()const
      {
         return theMinX;
      }
   double getMinY()const
      {
         return theMinY;
      }
   rspfDpt getSpacing()const
      {
         return rspfDpt(theDx, theDy);
      }
protected:
   int*   theCharBuf[64];
   int   theCols;
   int   theRows;
   int   theZ;
   double theMinX;
   double theDx;
   double theMinY;
   double theDy;
};

#endif
