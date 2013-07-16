//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
// 
// Class for reading and writing an ENVI (The Environment for Visualizing
// Images) header file.  This parses envi header and places in a keyword
// list.
//
//----------------------------------------------------------------------------
// $Id: rspfEnviHeader.h 21519 2012-08-22 21:16:25Z dburken $

#ifndef rspfEnviHeader_HEADER
#define rspfEnviHeader_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfReferenced.h>
#include <iosfwd>

/**
 * Class for reading and writing an ENVI (The Environment for Visualizing
 * Images) header file.
 */
class RSPF_DLL rspfEnviHeader : public rspfReferenced
{
public:
   /** default construtor */
   rspfEnviHeader();

   /** virtual destructor */
   virtual ~rspfEnviHeader();

   virtual void reset();

   /** @return Const reference to map. */
   const rspfKeywordlist& getMap() const;
   
   /** @return Reference to map. */
   rspfKeywordlist& getMap();

   /**
    * @brief Gets value for key.
    * @param key
    * @param value
    * @return true if key is in map even if value is empty; false, if not.
    */
   bool getValue( const rspfString& key, rspfString& value ) const;

   /**
    * @brief Gets value for key.
    * @param key
    * @param value
    * @return true if key is in map even if value is empty; false, if not.
    */
   bool findCaseInsensitive( const rspfString& key,
                             rspfString& value ) const;

   /**
    * @brief Gets value for key.
    * @param key
    * @param value
    * @return true if key is in map even if value is empty; false, if not.
    */
   bool findSubStringCaseInsensitive( const rspfString& key,
                                      rspfString& value) const;

   /**
    * Opens an envi header.
    * 
    * @return true on success, false on error.
    */
   bool open(const rspfFilename& file);

   /**
    * Writes header to file in a standard envi format.
    *
    * @param file File to write to.
    *
    * @return true on success, false on error.
    */
   bool writeFile(const rspfFilename& file);
   
   /**
    * Prints header to out in a standard envi format.
    *
    * @param out Stream to write to.
    *
    * @return Reference to the stream passed.
    */
   std::ostream& print(std::ostream& out) const;

   /** @brief friend operator<< */
   friend RSPF_DLL std::ostream& operator<<(std::ostream& out,
                                             const rspfEnviHeader& obj);

   /**
    * @return The description of the file.
    */
   rspfString getDescription() const;

   /**
    * @param description The description of the file.
    */
   void setDescription(const rspfString& description);

   /**
    * @return The number of samples.
    */
   rspf_uint32 getSamples() const;

   /**
    * Sets the number of samples.
    * 
    * @param samples The number of samples.
    */
   void setSamples(rspf_uint32 samples);

   /**
    * @return The number of lines.
    */
   rspf_uint32 getLines() const;

   /**
    * Sets the number of lines.
    * 
    * @param lines The number of lines.
    */
   void setLines(rspf_uint32 lines);

   /**
    * @return The number of bands.
    */
   rspf_uint32 getBands() const;

   /**
    * Sets the number of bands.
    * 
    * @param bands The number of bands.
    */
   void setBands(rspf_uint32 bands);

   /**
    * @return The number of header offset in bytes.
    */
   rspf_uint32 getHeaderOffset() const;

   /**
    * Sets the header offset in bytes.
    * 
    * @param headerOffset The number of header offset in bytes.
    */
   void setHeaderOffset(rspf_uint32 headerOffset);

   /**
    * @return The file type
    */
   rspfString getFileType() const;

   /**
    * Sets the file type.
    * 
    * @param fileType The sensor type as a string.
    */
   void setFileType(const rspfString& fileType);

   /**
    * @return The envi data type..
    */
   rspf_uint32 getDataType() const;

   /**
    * @return The rspfScalarType from the envi data type..
    */
   rspfScalarType getOssimScalarType() const;

   /**
    * Sets the envi data type based on the rspfScalarType.
    * 
    * @param scalar The rspfScalarType of the image.
    */
   void setDataType(rspfScalarType scalar);

   /**
    * @return The envi interleave type.
    */
   rspfString getInterleaveType() const;

   /**
    * @return The rspfInterleaveType from the envi interleave..
    */
   rspfInterleaveType getOssimInterleaveType() const;

   /**
    * Sets the envi interleave type string based on the rspfInterleaveType.
    * 
    * @param interleave The rspfInterleaveType of the image.
    */
   void setInterleaveType(rspfInterleaveType interleave);

   /**
    * @return The sensor type..
    */
   rspfString getSensorType() const;

   /**
    * Sets the envi sensor type string.
    * 
    * @param sensorType The sensor type as a string.
    */
   void setSensorType(const rspfString& sensorType);

   /**
    * @return If key "byte order" found returns the envi byte order; else,
    * system byte order.  
    *
    * @note (Same as the rspfByteOrder enumeration):
    * 0 = LITTLE_ENDIAN,
    * 1 = BIG_ENDIAN
    */
   rspfByteOrder getByteOrder() const;
   
   /**
    * Sets the envi byte order from the rspfByteOrder.
    * 
    * @param byteorder The rspfByteOrder of the image.
    */
   void setByteorder(rspfByteOrder byteOrder);
   
   /**
    * @return The x start.
    */
   rspf_int32 getXStart() const;
   
   /**
    * Sets the x start.
    * 
    * @param xStart
    */
   void setXStart(rspf_int32 xstart);
   
   /**
    * @return The x start.
    */
   rspf_int32 getYStart() const;
   
   /**
    * Sets the y start.
    * 
    * @param ystart
    */
   void setYStart(rspf_int32 ystart);

   /**
    * @return The envi map info string.
    */
   rspfString getMapInfo() const;

   /**
    * Sets the envi map info string.
    * 
    * @param mapInfo envi map info string.
    */
   void setMapInfo(const rspfString& mapInfo);

   /**
    * Sets the envi map info string from a keyword list containing geometry
    * information.
    * 
    * @param kwl Keyword list containing geometry information.
    */
   void setMapInfo(const rspfKeywordlist& kwl,  const char* prefix=0);

   /**
    * @return The env wavelength units..
    */
   rspfString getWavelengthUnits() const;

   /**
    * Sets the envi wavelength units string.
    * 
    * @param wavelengthUnits envi wavelength units string.
    */
   void setWavelengthUnits(const rspfString& wavelenghtUnits);

   /**
    * @param bandNames Vector of strings to initialize with band names.
    */
   void getBandNames(std::vector<rspfString>& bandNames) const;

   /**
    * Sets the band name string vector.
    * 
    * @param bandNames Vector of band name strings.
    */
   void setBandNames(const std::vector<rspfString>& bandNames);

   /**
    * @param wavelengths Vector of strings to initialize with wave lengths.
    */
   void getWavelengths(std::vector<rspfString>& wavelengths) const;

   /**
    * Sets the envi band name string.
    * 
    * @param wavelengths Vector of band name strings.
    */
   void setWavelengths(const std::vector<rspfString>& wavelengths);

   /**
    * Method to the load (recreate) the state of the object from a keyword
    * list.
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
    * description: My_file
    *
    * number_samples: 1024
    *
    * number_lines: 512
    *
    * number_bands: 3
    *
    * header_offset: 0
    *
    * file_type: ENVI Standard
    *
    * data_type: 1
    *
    * interleave_type: bil
    *
    * sensor_type:  Unknown
    *
    * (little_endian or big_endian)
    * byte_order: little_endian
    * 
    * map_info: Unkown
    *
    * wavelength_units: 
    *
    * band_name0: Red band
    * band_name1: Green band
    * band_name2: Blue band
    *
    * wavelength0:  620.25
    * wavelength1:  514.5
    * wavelength2:  470.0
    */
   bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

   /**
    * @brief Global method to test first line of file for "ENVI".
    * @return true on success, false on error.
    */
   static bool isEnviHeader( const rspfFilename& file );

   /**
    * @brief Global method to test first line of stream for "ENVI".
    * @return true on success, false on error.
    */
   static bool isEnviHeader( std::istream& in );

   /** @return Path to envi header file. */
   const rspfFilename& getFile() const;
   
private:

   /**
    * @brief Parses stream.
    * @return true on success, false on error.
    */
   bool readStream(std::istream& in);
   
   rspfFilename       m_file; // Name of header file.
   rspfKeywordlist    m_keywords;
};
#endif /* #ifndef rspfEnviHeader_HEADER */

