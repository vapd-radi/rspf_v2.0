//*****************************************************************************
// FILE: rspfQuickbirdRpcModel.h
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// DESCRIPTION: Contains declaration of class rspfQuickbirdRpcModel. This 
//    derived class implements the capability of reading Quickbird RPC support
//    data.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfQuickbirdRpcModel.h 20606 2012-02-24 12:29:52Z gpotts $
#ifndef rspfQuickbirdRpcModel_HEADER
#define rspfQuickbirdRpcModel_HEADER

#include <rspf/projection/rspfRpcModel.h>
#include <rspf/support_data/rspfQuickbirdMetaData.h>

class rspfFilename;
class rspfQuickbirdMetaData;
class rspfQbTileFilesHandler;

/*!****************************************************************************
 *
 * CLASS:  rspfQuickbirdRpcModel
 *
 *****************************************************************************/
class RSPFDLLEXPORT rspfQuickbirdRpcModel : public rspfRpcModel
{
public:
   rspfQuickbirdRpcModel();
   rspfQuickbirdRpcModel(const rspfQuickbirdRpcModel& rhs);

   //! Initializes
   rspfQuickbirdRpcModel(const rspfQbTileFilesHandler* handler);
   ~rspfQuickbirdRpcModel();

   virtual rspfObject* dup() const;
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   virtual bool parseFile(const rspfFilename& file);
   
   void setSupportData(rspfQuickbirdMetaData* supportData)
   {
      theSupportData = supportData;
   }
   rspfQuickbirdMetaData* getSupportData()
   {
      return theSupportData.get();
   }
   const rspfQuickbirdMetaData* getSupportData()const
   {
      return theSupportData.get();
   }
protected:
   bool parseNitfFile(const rspfFilename& file);
   bool parseTiffFile(const rspfFilename& file);
   bool parseMetaData(const rspfFilename& file);
   bool parseRpcData (const rspfFilename& file);
   bool parseTileData(const rspfFilename& file);

   //! Given an initial filename with case-agnostic extension, this method searches first for an
   //! image-specific instance of that file (i.e., with _R*C* in the filename) before considering
   //! the mosaic-global support file (_R*C* removed). If a file is found, the argument is modified 
   //! to match the actual filename and TRUE is returned. Otherwise, argument filename is left 
   //! unchanged and FALSE is returned.
   bool findSupportFile(rspfFilename& file) const;

void finishConstruction();

   rspfRefPtr<rspfQuickbirdMetaData> theSupportData;

TYPE_DATA
};

#endif
