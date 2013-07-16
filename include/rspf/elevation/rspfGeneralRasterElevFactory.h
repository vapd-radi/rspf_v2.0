//----------------------------------------------------------------------------
// License:  see top level LICENSE.txt file
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description:
//
// elevation handler for general raster elevation files.
//
//----------------------------------------------------------------------------
// $Id: rspfGeneralRasterElevFactory.h 13269 2008-07-25 14:27:36Z dburken $
#ifndef rspfGeneralRasterElevFactory_HEADER
#define rspfGeneralRasterElevFactory_HEADER

#include <rspf/elevation/rspfElevSourceFactory.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/elevation/rspfGeneralRasterElevHandler.h>

/**
 * @class rspfSrtmFactory Used by the elevation manager, this class returns
 * an rspfSrtmElevSource given a ground point and some directory with srtm
 * files in it.
 */
class RSPF_DLL rspfGeneralRasterElevFactory : public rspfElevSourceFactory
{
public:
   typedef std::vector<rspfGeneralRasterElevHandler::GeneralRasterInfo> BoundingRectListType;

   /** Constructor that takes a file name. */

   /** default constructor */
   rspfGeneralRasterElevFactory();

   /** Constructor that takes a directory name. */
   rspfGeneralRasterElevFactory(const rspfFilename& dir);

   /** destructor */
   virtual ~rspfGeneralRasterElevFactory();
   void setDirectory(const rspfFilename& directory);

   /**
    * Open the appropriate handler that covers given a
    * ground point.
    *
    * @param gpt Ground point that an elevation source is need for.
    *
    * @return Returns a pointer to an rspfElevSource if an elevation file is found
    * that can cover the ground point.  Returns NULL if no cell is found
    * for the point.
    */
   virtual rspfElevSource* getNewElevSource(const rspfGpt& gpt) const;

   /** To satisfy pure virtual. */
   virtual void createIndex();

protected:
/*    void initializeList(const rspfFilename& file); */
/*    bool initializeInfo(rspfGeneralRasterElevHandler::GeneralRasterInfo& info, */
/*                        const rspfFilename& file); */
   void addInfo(const rspfGeneralRasterElevHandler::GeneralRasterInfo& info);

   
   std::vector<rspfGeneralRasterElevHandler::GeneralRasterInfo>  theGeneralRasterInfoList;
   rspfDrect            theBoundingRect;
/*    mutable std::ifstream theFileStr;  */
   mutable rspf_int32   theCurrentIdx;
   mutable bool theHandlerReturnedFlag;
   
TYPE_DATA
};

#endif /* End of "#ifndef rspfSrtmFactory_HEADER" */
