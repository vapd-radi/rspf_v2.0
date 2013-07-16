//*****************************************************************************
// FILE: rspfDtedHandler.h
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//   Contains declaration of class rspfDtedHandler. This class derives from
//   rspfElevHandler. It is responsible for loading an individual DTED cell
//   from disk. This elevation files are memory mapped.
//
// SOFTWARE HISTORY:
//>
//   05Feb2001  Ken Melero
//              Initial coding of rspfDted.h
//   19Apr2001  Oscar Kramer
//              Derived from rspfElevCellHandler.
//<
//*****************************************************************************
// $Id: rspfDtedHandler.h 21210 2012-07-03 13:11:20Z gpotts $

#ifndef rspfDtedHandler_HEADER
#define rspfDtedHandler_HEADER

#include <fstream>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>
#include <rspf/elevation/rspfElevCellHandler.h>
#include <OpenThreads/Mutex>
#include <rspf/support_data/rspfDtedVol.h>
#include <rspf/support_data/rspfDtedHdr.h>
#include <rspf/support_data/rspfDtedUhl.h>
#include <rspf/support_data/rspfDtedDsi.h>
#include <rspf/support_data/rspfDtedAcc.h>
#include <rspf/support_data/rspfDtedRecord.h>

class RSPF_DLL rspfDtedHandler : public rspfElevCellHandler
{
public:

   /// number of Dted posts per point.
   static const int TOTAL_POSTS = 4;
   /// number of Dted posts per block
   static const int NUM_POSTS_PER_BLOCK= 2;

   /// rspfDtedHandler
   rspfDtedHandler()
   {
      
   }
   rspfDtedHandler(const rspfFilename& dted_file, bool memoryMapFlag=false);
   

   enum
   {
      DATA_RECORD_OFFSET_TO_POST = 8,     // bytes
      DATA_RECORD_CHECKSUM_SIZE  = 4,     // bytes
      POST_SIZE                  = 2,     // bytes
      NULL_POST                  = -32767 // Fixed by DTED specification.
   };

   virtual bool open(const rspfFilename& file, bool memoryMapFlag=false);
   virtual void close();
   
   /*!
    * METHOD: getHeightAboveMSL
    * Height access methods.
    */
   virtual double getHeightAboveMSL(const rspfGpt&);

   /*!
    *  METHOD:  getSizeOfElevCell
    *  Returns the number of post in the cell.  Satisfies pure virtual.
    *  Note:  x = longitude, y = latitude
    */
   virtual rspfIpt getSizeOfElevCell() const;
      
   /*!
    *  METHOD:  getPostValue
    *  Returns the value at a given grid point as a double.
    *  Satisfies pure virtual.
    */
   virtual double getPostValue(const rspfIpt& gridPt) const;

   rspfString  edition()         const;
   rspfString  productLevel()    const;
   rspfString  compilationDate() const;

   virtual bool isOpen()const;
   
   virtual bool getAccuracyInfo(rspfElevationAccuracyInfo& info, const rspfGpt& gpt) const;
   
   const rspfDtedVol& vol()const
   {
      return m_vol;
   }
   const rspfDtedHdr& hdr()const
   {
      return m_hdr;
   }
   const rspfDtedUhl& uhl()const
   {
      return m_uhl;
   }
   const rspfDtedDsi& dsi()const
   {
      return m_dsi;
   }
   const rspfDtedAcc& acc()const
   {
      return m_acc;
   }
protected:

   /// DtedPost, this class contains the height, weighting factor and status
   class DtedPost
   {
   public:
     // constructor - initialise variables
     DtedPost():
       m_height(0),
       m_weight(0),
       m_status(false)
     {
     }
     // destructor
     virtual ~DtedPost();
     // member variables
     double m_height;
     double m_weight;
     bool m_status;
   };

   /// DtedHeight is a class for storing DTED information
   /// - 4 posts are used to generate an interpolated height value.
   class DtedHeight
   {
   public:
     // constructor
     DtedHeight();
     // destructor
     virtual ~DtedHeight();
     // calculate the interpolated Height for the posts
     double calcHeight();
     // debug
     void debug();
     // post data
     DtedPost m_posts[TOTAL_POSTS];
   };


   virtual ~rspfDtedHandler();
  // Disallow operator= and copy construction...
   const rspfDtedHandler& operator=(const rspfDtedHandler& rhs);
   rspfDtedHandler(const rspfDtedHandler&);

   /*!
    *  If statistics file exist, stats will be initialized from that; else,
    *  this scans the dted cell and gets stats, then, writes new stats file.
    *  The statistics file will be named accordingly:
    *  If dted cell = n27.dt1 then the stats file = n27.statistics.
    *  Currently method only grabs the min and max posts value.
    */
   void gatherStatistics();

   rspf_sint16 convertSignedMagnitude(rspf_uint16& s) const;
   virtual double getHeightAboveMSL(const rspfGpt&, bool readFromFile);

  /**
   * read the height posts from the File
   * @param postData - post heights, status & weight
   * @param offset - file contents offset to start reading from
   */
   void readPostsFromFile(DtedHeight &postData, int offset);

   mutable OpenThreads::Mutex m_fileStrMutex;
   mutable std::ifstream m_fileStr;
   
   rspf_int32      m_numLonLines;  // east-west dir
   rspf_int32      m_numLatPoints; // north-south
   rspf_int32      m_dtedRecordSizeInBytes;
   rspfString      m_edition;
   rspfString      m_productLevel;
   rspfString      m_compilationDate;
   rspf_int32      m_offsetToFirstDataRecord;
   double           m_latSpacing;   // degrees
   double           m_lonSpacing;   // degrees
   rspfDpt         m_swCornerPost; // cell origin;

   // Indicates whether byte swapping is needed.
   bool m_swapBytesFlag;

   mutable OpenThreads::Mutex m_memoryMapMutex;
   mutable std::vector<rspf_uint8> m_memoryMap;
   
   rspfDtedVol m_vol;
   rspfDtedHdr m_hdr;
   rspfDtedUhl m_uhl;
   rspfDtedDsi m_dsi;
   rspfDtedAcc m_acc;
   TYPE_DATA
};

inline rspf_sint16 rspfDtedHandler::convertSignedMagnitude(rspf_uint16& s) const
{
   // DATA_VALUE_MASK 0x7fff = 0111 1111 1111 1111
   // DATA_SIGN_MASK  0x8000 = 1000 0000 0000 0000
   
   // First check to see if the bytes need swapped.
   s = (m_swapBytesFlag ? ( ((s & 0x00ff) << 8) | ((s & 0xff00) >> 8) ) : s);
   
   // If the sign bit is set, mask it out then multiply by negative one.
   if (s & 0x8000)
   {
      return (static_cast<rspf_sint16>(s & 0x7fff) * -1);
   }
   
   return static_cast<rspf_sint16>(s);
}

inline bool rspfDtedHandler::isOpen()const
{
   if(!m_memoryMap.empty()) return true;
   
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_fileStrMutex);
   return (m_fileStr.is_open());
}

inline void rspfDtedHandler::close()
{
   m_fileStr.close();
   m_memoryMap.clear();
}

#endif
