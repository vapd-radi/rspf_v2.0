//----------------------------------------------------------------------------
//
// License:  see top level LICENSE.txt
// 
// Author:  Garrett Potts
//
// Description:
// 
//
//----------------------------------------------------------------------------
// $Id: rspfGeneralRasterElevHandler.h 16355 2010-01-14 21:15:25Z dburken $
#ifndef rspfGeneralRasterElevHandler_HEADER
#define rspfGeneralRasterElevHandler_HEADER
#include <list>
#include <rspf/base/rspfIoStream.h>
//#include <fstream>

#include <rspf/base/rspfString.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/elevation/rspfElevCellHandler.h>
#include <rspf/imaging/rspfGeneralRasterInfo.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfImageViewTransform.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfGpt.h>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>
class rspfProjection;
/**
 * @class rspfGeneralRasterElevHandler Elevation source for an srtm file.
 */
class  RSPF_DLL rspfGeneralRasterElevHandler : public rspfElevCellHandler
{
public:
   class GeneralRasterInfo
      {
      public:
         GeneralRasterInfo()
         :theWidth(0),
         theHeight(0),
         theNullHeightValue(rspf::nan()),
         theScalarType(RSPF_SCALAR_UNKNOWN),
         theBytesPerRawLine(0),
         theDatum(0),
         theGeometry(0)
         {
         }
         GeneralRasterInfo(const  rspfGeneralRasterElevHandler::GeneralRasterInfo& src)
         :theFilename(src.theFilename),
         theImageRect(src.theImageRect),
         theUl(src.theUl),
         theLr(src.theLr),
         theWidth(src.theWidth),
         theHeight(src.theHeight),
         theWgs84GroundRect(src.theWgs84GroundRect),
         theNullHeightValue(src.theNullHeightValue),
         theByteOrder(src.theByteOrder),
         theScalarType(src.theScalarType),
         theBytesPerRawLine(src.theBytesPerRawLine),
         theDatum(src.theDatum),
         theGeometry(src.theGeometry)
         {
         }
         rspfFilename     theFilename;
         rspfIrect        theImageRect;
         rspfIpt          theUl;
         rspfIpt          theLr;
         rspf_uint32      theWidth;
         rspf_uint32      theHeight;
         rspfDrect        theWgs84GroundRect;
         rspf_float64     theNullHeightValue;
         rspfByteOrder    theByteOrder;
         rspfScalarType   theScalarType;
         rspf_uint32      theBytesPerRawLine;
         const rspfDatum* theDatum;
         rspfRefPtr<rspfImageGeometry> theGeometry;  //add by simbla
      };
   rspfGeneralRasterElevHandler(const rspfFilename& file="");
   rspfGeneralRasterElevHandler(const rspfGeneralRasterElevHandler::GeneralRasterInfo& generalRasterInfo);
   rspfGeneralRasterElevHandler(const rspfGeneralRasterElevHandler& rhs);
   const rspfGeneralRasterElevHandler& operator=(const rspfGeneralRasterElevHandler& rhs);

   /** destructor */
   virtual ~rspfGeneralRasterElevHandler();
   virtual rspfObject* dup()const;

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
   bool open(const rspfFilename& file, bool memoryMapFlag=false);

   /**
    * Closes the stream to the file.
    */
   virtual void close();

   /**
    * This method does not really fit the handler since this handle a
    * directory not a cell that could have holes in it.  So users looking for
    * valid coverage should call "pointHasCoverage".
    */
   rspfDrect getBoundingRect()const;

   /**
    * Loops through rspfGeneralRasterElevHandler::BoundingRectListType and
    * checks for coverage.
    *
    * @param gpt Point to look for.
    *
    * @return true if coverage is found false if not.
    */
   virtual bool pointHasCoverage(const rspfGpt& gpt) const;

   const rspfGeneralRasterElevHandler::GeneralRasterInfo& generalRasterInfo()const;
   
private:
   template <class T>
   double getHeightAboveMSLFileTemplate(T dummy,
                                    const rspfGeneralRasterElevHandler::GeneralRasterInfo& info,
                                    const rspfGpt& gpt);
   template <class T>
   double getHeightAboveMSLMemoryTemplate(T dummy,
                                    const rspfGeneralRasterElevHandler::GeneralRasterInfo& info,
                                    const rspfGpt& gpt);
   
   virtual bool setFilename(const rspfFilename& file);
   
   rspfGeneralRasterElevHandler::GeneralRasterInfo theGeneralRasterInfo;
   mutable OpenThreads::Mutex m_inputStreamMutex;
   std::ifstream m_inputStream;

   /** @brief true if stream is open. */
   bool          m_streamOpen;
   
   std::vector<char> m_memoryMap;
TYPE_DATA
};

#endif /* End of "#ifndef rspfGeneralRasterElevHandler_HEADER" */
