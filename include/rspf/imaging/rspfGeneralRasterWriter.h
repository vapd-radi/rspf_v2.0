 //*******************************************************************
// Copyright (C) 2001 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Contains class declaration for rspfGeneralRasterWriter.
//
//*******************************************************************
// $Id: rspfGeneralRasterWriter.h 22197 2013-03-12 02:00:55Z dburken $

#ifndef rspfGeneralRasterWriter_HEADER
#define rspfGeneralRasterWriter_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/imaging/rspfImageFileWriter.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeyword.h>

class RSPF_DLL rspfGeneralRasterWriter : public rspfImageFileWriter
{
public:

   /** @brief default constructor */
   rspfGeneralRasterWriter();
   
   /**
    * void getImageTypeList(std::vector<rspfString>& imageTypeList)const
    *
    * Appends this writer image types to list "imageTypeList".
    *
    * This writer has the following types:
    * general_raster_bip
    * general_raster_bil
    * general_raster_bsq
    *
    * @param imageTypeList stl::vector<rspfString> list to append to.
    */ 
   virtual void getImageTypeList(std::vector<rspfString>& imageTypeList)const;

   /**
    * Returns a 3-letter extension from the image type descriptor 
    * (theOutputImageType) that can be used for image file extensions.
    *
    * @param imageType string representing image type.
    *
    * @return the 3-letter string extension.
    */
   virtual rspfString getExtension() const;

   virtual void set_rr_level(rspf_uint32 rr) { theRlevel = rr; }
   virtual bool isOpen()const;      
   virtual bool open();
   virtual void close();

   /**
    * saves the state of the object.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   /**
    * @brief Method to write the image to a stream.
    *
    * Callers must call setOutputStream(...) method prior to this.
    *
    * @return true on success, false on error.
    */
   virtual bool writeStream();

   /**
    * @brief Sets the output stream to write to.
    *
    * The stream will not be closed/deleted by this object.
    *
    * @param output The stream to write to.
    */
   virtual bool setOutputStream(std::ostream& stream);
   
protected:

   /**
    * Write out the file.
    * @return true on success, false on error.
    */
   virtual bool writeFile();

   /** @brief Protected ( this is a rspfRefPtr) destructor. */
   virtual ~rspfGeneralRasterWriter();

private:

   /**
    *  Writes image data to output file in BIP(Band Interleaved by Pixel)
    *  format.
    *  @return true on success, false on error.
    */
   bool writeToBip();

   /**
    *  Writes image data to output file in BIL(Band Interleaved by Line)
    *  format.
    *  @return true on success, false on error.
    */
   bool writeToBil();
   
   /**
    *  Writes image data to output file in BSQ(Band Sequential) format.
    *  @return true on success, false on error.
    */
   bool writeToBsq();
   
   /**
    *  Writes an rspf header file which can be used to load file in rspf.
    *  Returns true on success, false on error.
    */
   void writeHeader() const;

   /**
    *  Writes an envi header file which can be used to load file in envi.
    *  If the output image file is "foo.ras" then the rspf header file will
    *  be "foo.hdr".
    */
   void writeEnviHeader() const;

   /**
    * @return The interleave as a string of either: bil, bip, or bsq
    */
   rspfString getInterleaveString() const;
   
   std::ostream*       theOutputStream;
   bool                theOwnsStreamFlag;
   rspf_uint32        theRlevel;
   rspfByteOrder      theOutputByteOrder;
   
  /**
   * Populated while writting the data
   */
  std::vector<double>     theMinPerBand;
  std::vector<double>     theMaxPerBand;

TYPE_DATA
};

#endif
