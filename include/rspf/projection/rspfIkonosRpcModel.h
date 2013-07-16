//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains declaration of class rspfIkonosRpcModel. This 
//    derived class implements the capability of reading Ikonos RPC support
//    data.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfIkonosRpcModel.h 20606 2012-02-24 12:29:52Z gpotts $

#ifndef rspfIkonosRpcModel_HEADER
#define rspfIkonosRpcModel_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/projection/rspfRpcModel.h>
#include <rspf/support_data/rspfIkonosMetaData.h>

class rspfFilename;

/*!****************************************************************************
 *
 * CLASS:  rspfIkonosRpcModel
 *
 *****************************************************************************/
class RSPF_DLL rspfIkonosRpcModel : public rspfRpcModel
{
public:
  rspfIkonosRpcModel();
   rspfIkonosRpcModel(const rspfFilename& geom_file);

   rspfIkonosRpcModel(const rspfFilename& metadata,
                       const rspfFilename& rpcdata);

  virtual bool saveState(rspfKeywordlist& kwl,
			 const char* prefix=0)const;

  /**
    * @brief loadState
    * Fulfills rspfObject base-class pure virtuals. Loads and saves geometry
    * KWL files. Returns true if successful.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
  
   /*!
    * STATIC METHOD: writeGeomTemplate(ostream)
    * Writes a template of an rspfIkonosRpcModel geometry file.
    */
   static void writeGeomTemplate(ostream& os);

   virtual bool parseFile(const rspfFilename& file);
   
protected:
   virtual ~rspfIkonosRpcModel();
   void finishConstruction();
   void parseMetaData(const rspfFilename& metadata);
   void parseRpcData (const rspfFilename& rpcdata);

   bool isNitf(const rspfFilename& filename);
   bool parseTiffFile(const rspfFilename& filename);
/*    bool parseNitfFile(const rspfFilename& geom_file); */
   bool parseHdrData(const rspfFilename& data_file);

   rspfRefPtr<rspfIkonosMetaData> theSupportData;

   TYPE_DATA
};

#endif
