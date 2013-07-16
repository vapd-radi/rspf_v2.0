//**************************************************************************
// Copyright (C) 2003 Storage Area Networks, Inc.
//
// Written by:   Kenneth Melero  <kmelero@sanz.com>
//
//**************************************************************************

#ifndef rspfNadconGridFile_HEADER
#define rspfNadconGridFile_HEADER
#include <rspf/base/rspfNadconGridHeader.h>
#include <fstream>

class rspfNadconGridFile
{
public:
  rspfNadconGridFile():
    theFileOkFlag(false)
      {}
   ~rspfNadconGridFile();
   bool open(const rspfFilename& file);
   void close();

   /*!
    */
   double getShiftAtLatLon(double lat, double lon)const;
   bool pointWithin(double lat, double lon)const;
   const rspfDrect getBoundingRect()const
      {
         return theBoundingRect;
      }
   
   const rspfFilename& getFilename()const
      {
         return theFilename;
      }
   bool getFileOkFlag()const
      {
         return theFileOkFlag;
      }
   rspfDpt getSpacing()const
      {
         return theHeader.getSpacing();
      }
   
protected:
   mutable std::ifstream theInputFile;
  mutable bool theFileOkFlag;
  rspfFilename         theFilename;
   rspfNadconGridHeader theHeader;
   rspfDrect            theBoundingRect;
  rspfDpt               theLatLonOrigin;
};

#endif
