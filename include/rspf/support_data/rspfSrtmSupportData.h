//----------------------------------------------------------------------------
// Copyright (c) 2005, David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
// 
// Support data class for a Shuttle Radar Topography Mission (SRTM) file.
//
//----------------------------------------------------------------------------
// $Id: rspfSrtmSupportData.h 17602 2010-06-20 19:12:24Z dburken $
#ifndef rspfSrtmSupportData_HEADER
#define rspfSrtmSupportData_HEADER

#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfIoStream.h>
#include <rspf/projection/rspfProjection.h>

/**
 * @class rspfStrmSupportData
 * Support data class for a Shuttle Radar Topography Mission (SRTM) file.
 * Provides lines, samples, post spacing and geometry info given a valid
 * srtm file.  This class will set an error if the file is not deemed to
 * be valid.
 */
class RSPFDLLEXPORT rspfSrtmSupportData : public rspfObject
{
public:
   /** default constructor */
   rspfSrtmSupportData();

   /** destructor */
   virtual ~rspfSrtmSupportData();

   /**
    * Initializes object from file.  File is expected to be in a format like
    * N27W081.hgt or n27w081.hgt where "N27" is the South West latitude of
    * the cell and "W081" is the South West longitude of the cell.
    *
    * @param srtmFile File to set.
    *
    * @param scanForMinMax If true and there is no rspf meta data file with
    * the min and max keywords in it, this will call computeMinMax which will
    * scan the file for min and max.
    *
    * @return Returns true if file is deemed to be an srtm, false if not.
    *
    * @note This will write an rspf meta data (.omd) if one does not exists.
    */
   bool setFilename(const rspfFilename& srtmFile,
                    bool scanForMinMax = false);

   /** @return Returns the srtm file name. */
   rspfFilename getFilename() const;

   /**
    * @return Returns the number of lines or rows.  Returns  RSPF_UINT_NAN
    * if not initialized or file was invalid.
    */
   rspf_uint32 getNumberOfLines() const;
   
   /**
    * @return Returns the number of samples or columns. Returns
    * RSPF_UINT_NAN if not initialized or file was invalid.
    */
   rspf_uint32 getNumberOfSamples() const;

   /**
    *  Populates the keyword list with image geometry information.  This
    *  method is used to relay projection information to users.
    *
    *  @param kwl Keyword list that will be initialized with geometry info.
    *  Returns true if geometry info is present, false if not.
    *
    *  @param prefix The prefix is added to the resulting keyword list
    *  keywords.  So is you if you pass in a prefix of "image01.", the
    *  keyword ul_lat will be like:  "image01.ul_lat:  -40.00000"
    */
   virtual bool getImageGeometry(rspfKeywordlist& kwl, const char* prefix=0);

   /**
    * @brief Gets the projection for srtm cell.
    * @return  The projection for srtm cell.
    */
   rspfRefPtr<rspfProjection> getProjection() const;

   /**
    * Method to save the state of the object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;
   
   /**
    * Method to the load (recreate) the state of the object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
   /**
    * @return Returns the Southwest latitude in decimal degrees. Returns
    * RSPF_DBL_NAN if not initialized or file was invalid.
    */
   rspf_float64 getSouthwestLatitude() const;

   /**
    * @return Returns the Southwest longitude in decimal degrees.  Returns
    * RSPF_DBL_NAN if not initialized or file was invalid.
    */
   rspf_float64 getSouthwestLongitude() const;
   
   /**
    * @return Returns the latitude spacing in decimal degrees. Returns
    * RSPF_DBL_NAN if not initialized or file was invalid.
    */
   rspf_float64 getLatitudeSpacing() const;

   /**
    * @return Returns the longitude spacing in decimal degrees.  Returns
    * RSPF_DBL_NAN if not initialized or file was invalid.
    */
   rspf_float64 getLongitudeSpacing() const;

   /**
    * @return Returns the minimum pixel value.  This is also
    * the minimum height above sea level.  Returns
    * RSPF_SSHORT_NAN if not initialized or file was invalid.
    */
   rspf_float64 getMinPixelValue() const;

   /**
    * @return Returns the maximum pixel value.  This is also
    * the minimum height above sea level.  Returns
    * RSPF_SSHORT_NAN if not initialized or file was invalid.
    */
   rspf_float64 getMaxPixelValue() const;

   rspfScalarType getScalarType()const;
   /**
    * Prints data members for debug.
    * 
    * @return std::ostream&
    */
   virtual std::ostream& print(std::ostream& out) const;

private:

   /**
    * This is a loadState called by setFilename that does not reset "theFile".
    */
   bool loadOmd(const rspfKeywordlist& kwl, const char* prefix=0);

   /** Sets all data members to an unitialized state. */
   void clear();

   /**
    * Sets corner points from "theFile".
    *
    * @return true on success, false on error.
    */
   bool setCornerPoints();
   
   /**
    * Sets lines and samples from "theFile" size.
    *
    * @return true on success, false on error.
    */
   bool setSize();

   /**
    * Scans file for min and max pixels.
    *
    * @retun true on success, false on error.
    */
   bool computeMinMax();
  
   rspfFilename theFile;
   rspf_uint32  theNumberOfLines;
   rspf_uint32  theNumberOfSamples;
   rspf_float64 theSouthwestLatitude;  //< decimal degrees
   rspf_float64 theSouthwestLongitude; //< decimal degrees
   rspf_float64 theLatSpacing;         //< decimal degrees
   rspf_float64 theLonSpacing;         //< decimal degrees
   rspf_float64 theMinPixelValue;      //< meters
   rspf_float64 theMaxPixelValue;      //< meters

   rspfScalarType theScalarType;

   mutable rspfRefPtr<rspfIFStream> theFileStream;
   template <class T>
   bool computeMinMaxTemplate(T dummy,
                              double defaultNull);
};

#endif /* #ifndef rspfSrtmSupportData */