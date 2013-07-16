//*******************************************************************
// Copyright (C) 2003 Storage Area Networks, Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  Kenneth Melero (kmelero@sanz.com)
//
//*******************************************************************
//  $Id: rspfWorldFileWriter.h 15766 2009-10-20 12:37:09Z gpotts $

#ifndef rspfWorldFileWriter_H
#define rspfWorldFileWriter_H

#include <rspf/imaging/rspfMetadataFileWriter.h>

/** rspfWorldFileWriter */
class RSPFDLLEXPORT rspfWorldFileWriter : public rspfMetadataFileWriter
{
public:
   
   /** default constructor */
   rspfWorldFileWriter();

   /**
    * Load state method:
    *
    * This method call base class rspfMetadataFileWriter::loadState after
    * looking for its keywords.
    *
    * @param kwl Keyword list to initialize from.
    *
    * @param prefix Usually something like: "object1."
    *
    * @return This method will alway return true as it is intended to be
    * used in conjuction with the set methods.
    *
    * Keywords picked up by loadState:
    *
    * (linear units must be "feet", "us_survey_feet", or "meters")
    * (If projection is geographic units will always be decimal degrees.)
    * 
    * linear_units: us_survey_feet
    */
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

   /**
    * Satisfies pure virtual from rspfMetadataWriter base.
    *
    * Appends the writers image types to the "metadatatypeList".
    * 
    * @param metadatatypeList stl::vector<rspfString> list to append to.
    *
    * @note Appends to the list, does not clear it first.
    */
   virtual void getMetadatatypeList(
      std::vector<rspfString>& metadatatypeList) const;

   /**
    * Satisfies pure virtual from rspfMetadataWriter base.
    *
    * @param imageType string representing image type.
    *
    * @return true if "metadataType" is supported by writer.
    */
   virtual bool hasMetadataType(const rspfString& metadataType)const;

   /**
    * Sets the linear units of the output file to either feet,
    * us_survey_feet, or meters.
    *
    * If the projection is geographic, the call has no affect.
    *
    * If units is not feet, us_survey_feet or meters this call has no effect.
    *
    * @param units Desired output units of the world file.
    */
   void setLinearUnits(rspfUnitType units);
   
protected:
   /** default constructor */
   virtual ~rspfWorldFileWriter();
   virtual bool writeFile();

   rspfUnitType theUnits;

TYPE_DATA
};

#endif /* End of #ifndef rspfWorldFileWriter_H */
