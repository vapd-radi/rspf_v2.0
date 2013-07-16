//----------------------------------------------------------------------------
//
// License:  See tope level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
// 
// Shuttle Radar Topography Mission (SRTM) elevation source.
//
//----------------------------------------------------------------------------
// $Id: rspfSrtmHandler.h 17195 2010-04-23 17:32:18Z dburken $
#ifndef rspfSrtmHandler_HEADER
#define rspfSrtmHandler_HEADER

#include <rspf/base/rspfIoStream.h>
//#include <fstream>

#include <rspf/base/rspfString.h>
#include <rspf/elevation/rspfElevCellHandler.h>
#include <rspf/support_data/rspfSrtmSupportData.h>

class rspfEndian;

/**
 * @class rspfSrtmHandler Elevation source for an srtm file.
 */
class RSPFDLLEXPORT rspfSrtmHandler : public rspfElevCellHandler
{
public:

   /** Constructor that takes a file name. */
   rspfSrtmHandler();
   const rspfSrtmHandler& operator=(const rspfSrtmHandler& rhs);
   rspfSrtmHandler(const rspfSrtmHandler&);


   enum
   {
      NULL_POST = -32768 // Fixed by SRTM specification.
   };

   /**
    * METHOD: getHeightAboveMSL
    * Height access methods.
    */
   virtual double getHeightAboveMSL(const rspfGpt&);

   /**
    *  METHOD:  getSizeOfElevCell
    *  Returns the number of post in the cell.  Satisfies pure virtual.
    *  Note:  x = longitude, y = latitude
    */
   virtual rspfIpt getSizeOfElevCell() const;
      
   /**
    *  METHOD:  getPostValue
    *  Returns the value at a given grid point as a double.
    *  Satisfies pure virtual.
    */
   virtual double getPostValue(const rspfIpt& gridPt) const;

   virtual bool isOpen()const;
   
   /**
    * Opens a stream to the srtm cell.
    *
    * @return Returns true on success, false on error.
    */
   virtual bool open(const rspfFilename& file, bool memoryMapFlag=false);

   /**
    * Closes the stream to the file.
    */
   virtual void close();
   
protected:
   /** destructor */
   virtual ~rspfSrtmHandler();
   rspfSrtmSupportData m_supportData;
   mutable OpenThreads::Mutex m_fileStrMutex;
   std::ifstream m_fileStr;

   /** @brief true if stream is open. */
   bool          m_streamOpen;
   
   rspf_int32      m_numberOfLines;
   rspf_int32      m_numberOfSamples;
   rspf_int32      m_srtmRecordSizeInBytes;
   double           m_latSpacing;   // degrees
   double           m_lonSpacing;   // degrees
   rspfDpt         m_nwCornerPost; // cell origin;
   rspfEndian*     m_swapper;
   rspfScalarType  m_scalarType;
   
   mutable std::vector<rspf_int8> m_memoryMap;
   
   template <class T>
   double getHeightAboveMSLFileTemplate(T dummy, const rspfGpt& gpt);
   template <class T>
   double getHeightAboveMSLMemoryTemplate(T dummy, const rspfGpt& gpt);
   TYPE_DATA
};

#endif /* End of "#ifndef rspfSrtmHandler_HEADER" */
