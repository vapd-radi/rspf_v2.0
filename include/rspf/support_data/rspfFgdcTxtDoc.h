//----------------------------------------------------------------------------
//
// File rspfFgdcTxtDoc.h
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: rspfFgdcTxtDoc class declaration.
//
// FGDC = "Federal Geographic Data Committee"
//
// See: http://www.fgdc.gov/
// 
//----------------------------------------------------------------------------
// $Id: rspfFgdcTxtDoc.h 2673 2011-06-06 14:57:24Z david.burken $
#ifndef rspfFgdcTxtDoc_HEADER
#define rspfFgdcTxtDoc_HEADER 1

#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/projection/rspfProjection.h>

/**
 * @class rspfFgdcTxtDoc
 * @brief Support data container for FGDC in text format.
 *
 * Has minimum parse support to get Spatial_Reference_Information section to
 * extract projection information.
 *
 * TODO: Make an rspfFgdcBase and consolidate code from
 * rspfFgdcXmlDoc and this class. (drb - 15 Aug. 2011)
 */
class RSPF_DLL rspfFgdcTxtDoc : public rspfReferenced
{
public:

   /** default constructor */
   rspfFgdcTxtDoc();

   /** virtual destructor */
   virtual ~rspfFgdcTxtDoc();

   /**
    * @brief open method.
    *
    * @param file File name to open.
    *
    * @return true on success false on error.
    */
   bool open(const rspfFilename& file);

   /**
    * @brief Close method.
    *
    * This doesn't really do anything.  Here only because I think every open()
    * should have a matching close().
    */
   void close();

   /**
    * @brief Gets projection from Spatial_Reference_Information block.
    * @param proj Intialized by this method.  Set to null on projection cannot
    * be created.
    */   
   void getProjection(rspfRefPtr<rspfProjection>& proj);

   /**
    * @brief Gets units from Altitude_Distance_Units.
    * @return Units if found, RSPF_UNIT_UNKNOWN if not.
    */
   void getAltitudeDistanceUnits(std::string& units) const;

private:

   /**
    * @brief Gets projection from Spatial_Reference_Information block for
    * version FGDC-STD-001-1998.
    * @param str Stream to read from.
    * @param proj Intialized by this method.  Set to null on projection cannot
    * be created.
    */ 
   void getProjectionV1(std::ifstream& str,
                        rspfRefPtr<rspfProjection>& proj);

   /**
    * @brief Finds key and returns true if present.
    *
    * Leaves stream at position of last getline.
    * 
    * @param str Stream to read from.
    * @param key Key to find.
    * @return true if present false if not.
    */
   bool findKey( std::ifstream& str, const std::string& key);

   /**
    * @brief Finds key and returns and intializes value if present.
    *
    * Stream position at end of call is dependent on seekBack flag.
    * 
    * @param str Stream to read from.
    * @param seekBack If true the stream will be repositioned to original
    * position at beginning of the call.
    * @param key Key to find.
    * @param value Intialized with value if key is found.
    * @return true if present false if not.
    */
   bool findKey( std::ifstream& str,
                 bool seekBack,
                 const std::string& key,
                 std::string& value);

   /**
    * Gets rspf datum string from fgdc datum string.
    */
   void getOssimDatum( const std::string& fgdcDatumString,
                       std::string& rspfDatumCode ) const;

   // Container for relevant data from FGDC file.
   rspfRefPtr<rspfKeywordlist> m_kwl;
   
}; // End: class rspfFgdcTxtDoc

#endif /* #ifndef rspfFgdcTxtDoc_HEADER */
