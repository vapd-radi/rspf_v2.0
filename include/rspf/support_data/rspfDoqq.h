//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Ken Melero
//
// Description: This class parses a Digital Ortho Quarter Quad (DOQQ)
//              header.
//
//********************************************************************
// $Id: rspfDoqq.h 19900 2011-08-04 14:19:57Z dburken $

#ifndef rspfDoqq_HEADER
#define rspfDoqq_HEADER

//#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/support_data/rspfInfoBase.h>

class RSPF_DLL rspfDoqq : public rspfInfoBase
{
public:
   rspfDoqq();
   rspfDoqq(rspfFilename file);
   ~rspfDoqq();
   
   enum rspfErrorStatus
   {
      RSPF_OK    = 0,
      RSPF_ERROR = 1
   };

   enum
   {
      GSD_X_OFFSET            = 59,
      GSD_Y_OFFSET            = 59,
      DATA_ORDER_OFFSET       = 142,      
      LINE_OFFSET             = 145,
      SAMPLE_OFFSET           = 151,
      PROJECTION_OFFSET       = 196,
      RGB_OFFSET              = 157,
      UL_EASTING_OFFSET       = 288,
      UL_NORTHING_OFFSET      = 312,
      UTM_ZONE_OFFSET         = 199,
      DATUM_OFFSET            = 168,

      DATA_ORDER_SIZE         = 3,
      LINE_SIZE               = 8,
      SAMPLE_SIZE             = 8,  
      PROJECTION_SIZE         = 3,
      RGB_SIZE                = 3,
      MARKING_U_SIZE          = 5,
      MARKING_V_SIZE          = 5,
      UL_EASTING_SIZE         = 24,
      UL_NORTHING_SIZE        = 24,
      UTM_ZONE_SIZE           = 6,
      DATUM_SIZE              = 2,
      RADIOMETRY_SIZE         = 3,
      GSD_SIZE              = 12
   };

   //! Implementation of rspfInfoBase class pure virtual.
   virtual bool open(const rspfFilename& file);

   //! Implementation of rspfInfoBase class pure virtual.
   virtual std::ostream& print(std::ostream& out) const;

   rspfString theProjection;
   rspfString theDatum;
   rspfString theQuadName;
   rspfString theQuad;
   rspfString theNation;
   rspfString theState;
   rspfString theDataOrder;
   rspfString theMarkU;
   rspfString theMarkV;
   rspfString theImageSource;
   rspfString theSourceImageID;

   rspf_int32 theUtmZone;
   rspf_int32 theLine;
   rspf_int32 theSample;
   rspf_int32 theRgb;

   rspfString theAcqYear;
   rspfString theAcqMonth;
   rspfString theAcqDay;
   rspfString theAcqYearMonthDay;

   rspf_float64 theRMSE;

   rspf_float64   theEasting;
   rspf_float64   theNorthing;
   rspf_float64   theUN;
   rspf_float64   theUE;
   rspf_float64   theLN;
   rspf_float64   theLE;
   rspfDpt        theGsd;
   rspf_int32     theHeaderSize;
   rspf_int32     theRecordSize;

   rspf_int32 errorStatus() const { return theErrorStatus; }
   
private:

   std::ifstream           theDoqFile;
   rspfErrorStatus   theErrorStatus;
   
   rspf_float64 convertStr(const char* str) const;

   void ldstr_v1(std::istream& in);
   void ldstr_v2(std::istream& in);
};

#endif
