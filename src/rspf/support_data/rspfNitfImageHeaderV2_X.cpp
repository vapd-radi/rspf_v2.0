#include <rspf/support_data/rspfNitfImageHeaderV2_X.h>

#include <cmath> /* for fmod */
#include <iomanip>
#include <sstream>

#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfDms.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/support_data/rspfNitfImageHeader.h>
#include <rspf/support_data/rspfNitfCommon.h>

static const rspfTrace traceDebug(
   rspfString("rspfNitfImageHeaderV2_X:debug"));

const rspfString rspfNitfImageHeaderV2_X::IM_KW       = "IM";
const rspfString rspfNitfImageHeaderV2_X::IID1_KW     = "IID1";
const rspfString rspfNitfImageHeaderV2_X::IDATIM_KW   = "IDATIM";
const rspfString rspfNitfImageHeaderV2_X::TGTID_KW    = "TGTID";
const rspfString rspfNitfImageHeaderV2_X::IID2_KW     = "IID2";
const rspfString rspfNitfImageHeaderV2_X::ITITLE_KW   = "ITITLE";
const rspfString rspfNitfImageHeaderV2_X::ISCLAS_KW   = "ISCLAS";
const rspfString rspfNitfImageHeaderV2_X::ENCRYP_KW   = "ENCRYP";
const rspfString rspfNitfImageHeaderV2_X::ISORCE_KW   = "ISORCE";
const rspfString rspfNitfImageHeaderV2_X::NROWS_KW    = "NROWS";
const rspfString rspfNitfImageHeaderV2_X::NCOLS_KW    = "NCOLS";
const rspfString rspfNitfImageHeaderV2_X::PVTYPE_KW   = "PVTYPE";
const rspfString rspfNitfImageHeaderV2_X::IREP_KW     = "IREP";
const rspfString rspfNitfImageHeaderV2_X::ICAT_KW     = "ICAT";
const rspfString rspfNitfImageHeaderV2_X::ABPP_KW     = "ABPP";
const rspfString rspfNitfImageHeaderV2_X::PJUST_KW    = "PJUST";
const rspfString rspfNitfImageHeaderV2_X::ICORDS_KW   = "ICORDS";
const rspfString rspfNitfImageHeaderV2_X::IGEOLO_KW   = "IGEOLO";
const rspfString rspfNitfImageHeaderV2_X::NICOM_KW    = "NICOM";
const rspfString rspfNitfImageHeaderV2_X::ICOM_KW     =  "ICOM";
const rspfString rspfNitfImageHeaderV2_X::IC_KW       = "IC";
const rspfString rspfNitfImageHeaderV2_X::COMRAT_KW   = "COMRAT";
const rspfString rspfNitfImageHeaderV2_X::NBANDS_KW   = "NBANDS";
const rspfString rspfNitfImageHeaderV2_X::ISYNC_KW    = "ISYNC";
const rspfString rspfNitfImageHeaderV2_X::IMODE_KW    = "IMODE";
const rspfString rspfNitfImageHeaderV2_X::NBPR_KW     = "NBPR";
const rspfString rspfNitfImageHeaderV2_X::NBPC_KW     = "NBPC";
const rspfString rspfNitfImageHeaderV2_X::NPPBH_KW    = "NPPBH";
const rspfString rspfNitfImageHeaderV2_X::NPPBV_KW    = "NPPBV";
const rspfString rspfNitfImageHeaderV2_X::NBPP_KW     = "NBPP";
const rspfString rspfNitfImageHeaderV2_X::IDLVL_KW    = "IDLVL";
const rspfString rspfNitfImageHeaderV2_X::IALVL_KW    = "IALVL";
const rspfString rspfNitfImageHeaderV2_X::ILOC_KW     = "ILOC";
const rspfString rspfNitfImageHeaderV2_X::IMAG_KW     = "IMAG";
const rspfString rspfNitfImageHeaderV2_X::UDIDL_KW    = "UDIDL";
const rspfString rspfNitfImageHeaderV2_X::UDOFL_KW    = "UDOFL";
const rspfString rspfNitfImageHeaderV2_X::IXSHDL_KW   = "IXSHDL";
const rspfString rspfNitfImageHeaderV2_X::IXSOFL_KW   = "IXSOFL";
const rspfString rspfNitfImageHeaderV2_X::IMDATOFF_KW = "IMDATOFF";
const rspfString rspfNitfImageHeaderV2_X::BMRLNTH_KW  = "BMRLNTH";



RTTI_DEF1(rspfNitfImageHeaderV2_X,
          "rspfNitfImageHeaderV2_X",
          rspfNitfImageHeader);

rspfNitfImageHeaderV2_X::rspfNitfImageHeaderV2_X()
:theImageComments(0)
{
}

rspfIrect rspfNitfImageHeaderV2_X::getImageRect()const
{
   rspfIpt ul(0, 0);
   rspfIpt lr(getNumberOfCols()-1, getNumberOfRows()-1);
   return rspfIrect(ul, lr);
   
#if 0 
   //---
   // Changed to not include offset 20100619 (drb).
   // Use void getImageLocation(rspfIpt& loc)const to get offset.
   //---
   rspfDpt ul(rspfString((char*)(&theImageLocation[5])).toDouble(),
               rspfString((char*)theImageLocation,
                           (char*)(&theImageLocation[5])).toDouble());
   
    double rows = rspfString(theSignificantRows).toDouble();
    double cols = rspfString(theSignificantCols).toDouble();

    rspfDpt lr(ul.x + cols-1,
                ul.y + rows-1);
     return rspfDrect(ul, lr);
#endif
}

rspfIrect rspfNitfImageHeaderV2_X::getBlockImageRect()const
{
   rspfIpt ul(0, 0);
   rspfIpt lr( (getNumberOfPixelsPerBlockHoriz()*getNumberOfBlocksPerRow())-1,
                (getNumberOfPixelsPerBlockVert() *getNumberOfBlocksPerCol())-1);
               
   return rspfIrect(ul, lr);
   
#if 0
   //---
   // Changed to not include offset 20100619 (drb).
   // Use void getImageLocation(rspfIpt& loc)const to get offset.
   //---   
   rspfDpt ul(rspfString((char*)(&theImageLocation[5])).toDouble(),
               rspfString((char*)theImageLocation,
                           (char*)(&theImageLocation[5])).toDouble());
   
   double rows = getNumberOfPixelsPerBlockVert()*getNumberOfBlocksPerCol();
   double cols = getNumberOfPixelsPerBlockHoriz()*getNumberOfBlocksPerRow();;
   
   rspfDpt lr(ul.x + cols-1,
               ul.y + rows-1);
   return rspfDrect(ul, lr);
#endif
}

void rspfNitfImageHeaderV2_X::setImageId(const rspfString& value)
{
   rspfNitfCommon::setField(theImageId, value, 10);
}

void rspfNitfImageHeaderV2_X::setAquisitionDateTime(const rspfString& value)
{
   rspfNitfCommon::setField(theDateTime, value, 14);
}

void rspfNitfImageHeaderV2_X::setTargetId(const rspfString& value)
{
   rspfNitfCommon::setField(theTargetId, value, 17);
}

void rspfNitfImageHeaderV2_X::setTitle(const rspfString& value)
{
   rspfNitfCommon::setField(theTitle, value, 80);
}

void rspfNitfImageHeaderV2_X::setSecurityClassification(const rspfString& value)
{
   rspfNitfCommon::setField(theSecurityClassification, value, 1);
}

void rspfNitfImageHeaderV2_X::setEncryption(const rspfString& value)
{
   rspfNitfCommon::setField(theEncryption, value, 1);
}

void rspfNitfImageHeaderV2_X::setImageSource(const rspfString& value)
{
   rspfNitfCommon::setField(theImageSource, value, 42);
}

void rspfNitfImageHeaderV2_X::setJustification(const rspfString& value)
{
   rspfNitfCommon::setField(theJustification, value, 1);
}

void rspfNitfImageHeaderV2_X::setCoordinateSystem(const rspfString& value)
{
   rspfNitfCommon::setField(theCoordinateSystem, value, 1);
}

void rspfNitfImageHeaderV2_X::setGeographicLocation(const rspfString& value)
{
   rspfNitfCommon::setField(theGeographicLocation, value, 60);
}

void rspfNitfImageHeaderV2_X::setNumberOfComments(const rspfString& value)
{
   rspfNitfCommon::setField(theNumberOfComments, value, 1);
}

void rspfNitfImageHeaderV2_X::setCompression(const rspfString& value)
{
   rspfNitfCommon::setField(theCompression, value, 2);
}

void rspfNitfImageHeaderV2_X::setCompressionRateCode(const rspfString& value)
{
   rspfNitfCommon::setField(theCompressionRateCode, value, 4);
}

void rspfNitfImageHeaderV2_X::setDisplayLevel(const rspfString& value)
{
   rspfNitfCommon::setField(theDisplayLevel, value, 3, std::ios::right, '0');
}

void rspfNitfImageHeaderV2_X::setAttachmentLevel(const rspfString& value)
{
   rspfNitfCommon::setField(theAttachmentLevel, value, 3);
}

void rspfNitfImageHeaderV2_X::setPixelType(const rspfString& pixelType)
{
   rspfNitfCommon::setField(thePixelValueType, pixelType, 3);
}

void rspfNitfImageHeaderV2_X::setBitsPerPixel(rspf_uint32 bitsPerPixel)
{
   std::stringstream strm;
   strm << bitsPerPixel;
   rspfNitfCommon::setField(theNumberOfBitsPerPixelPerBand,
                             strm.str(),
                             2,
                             std::ios::right,
                             '0');
}

void rspfNitfImageHeaderV2_X::setActualBitsPerPixel(rspf_uint32 bitsPerPixel)
{
   std::stringstream strm;
   strm << bitsPerPixel;
   rspfNitfCommon::setField(theActualBitsPerPixelPerBand,
                             strm.str(),
                             2,
                             std::ios::right,
                             '0');
}

void rspfNitfImageHeaderV2_X::setImageMode(char mode)
{
   theImageMode[0] = mode;
}

void rspfNitfImageHeaderV2_X::setCategory(const rspfString& category)
{
   rspfNitfCommon::setField(theCategory, category, 8);
}

void rspfNitfImageHeaderV2_X::setRepresentation(const rspfString& rep)
{
   rspfNitfCommon::setField(theRepresentation, rep, 8);
}

void rspfNitfImageHeaderV2_X::setBlocksPerRow(rspf_uint32 blocks)
{
   std::stringstream strm;
   strm << blocks;
   rspfNitfCommon::setField(theNumberOfBlocksPerRow,
                             strm.str(),
                             4,
                             std::ios::right,
                             '0');
}

void rspfNitfImageHeaderV2_X::setBlocksPerCol(rspf_uint32 blocks)
{
   std::stringstream strm;
   strm << blocks;
   rspfNitfCommon::setField(theNumberOfBlocksPerCol,
                             strm.str(),
                             4,
                             std::ios::right,
                             '0');
}

void rspfNitfImageHeaderV2_X::setNumberOfPixelsPerBlockRow(rspf_uint32 pixels)
{
   std::stringstream strm;
   strm << pixels;
   rspfNitfCommon::setField(theNumberOfPixelsPerBlockHoriz,
                             strm.str(),
                             4,
                             std::ios::right,
                             '0');
}

void rspfNitfImageHeaderV2_X::setNumberOfPixelsPerBlockCol(rspf_uint32 pixels)
{
   std::stringstream strm;
   strm << pixels;
   rspfNitfCommon::setField(theNumberOfPixelsPerBlockVert,
                             strm.str(),
                             4,
                             std::ios::right,
                             '0');
}

rspf_uint64 rspfNitfImageHeaderV2_X::getDataLocation() const
{
   return theDataLocation;
}

rspfString rspfNitfImageHeaderV2_X::getImageId()const
{
   return theImageId;
}

rspfString rspfNitfImageHeaderV2_X::getAquisitionDateTime()const
{
   return theDateTime;
}

rspfString rspfNitfImageHeaderV2_X::getTargetId()const
{
   return rspfString(theTargetId).trim();
}

rspfString rspfNitfImageHeaderV2_X::getTitle()const
{
   return rspfString(theTitle).trim();
}

rspfString rspfNitfImageHeaderV2_X::getSecurityClassification()const
{
   return rspfString(theSecurityClassification).trim();
}

rspfString rspfNitfImageHeaderV2_X::getEncryption()const
{
   return rspfString(theEncryption).trim();
}

rspfString rspfNitfImageHeaderV2_X::getImageSource()const
{
   return rspfString(theImageSource).trim();
}

rspfString rspfNitfImageHeaderV2_X::getJustification()const
{
   return rspfString(theJustification).trim();
}

rspfString rspfNitfImageHeaderV2_X::getCoordinateSystem()const
{
   return rspfString(theCoordinateSystem).trim();
}

rspfString rspfNitfImageHeaderV2_X::getGeographicLocation()const
{
   return rspfString(theGeographicLocation).trim();
}

rspfString rspfNitfImageHeaderV2_X::getNumberOfComments()const
{
   return rspfString(theNumberOfComments).trim();
}

rspfString rspfNitfImageHeaderV2_X::getCompressionCode()const
{
   return rspfString(theCompression).trim();
}

rspfString rspfNitfImageHeaderV2_X::getCompressionRateCode()const
{
   return theCompressionRateCode;
}

rspf_uint32 rspfNitfImageHeaderV2_X::getDisplayLevel()const
{
   return rspfString(theDisplayLevel).toUInt32();
}

rspf_uint32 rspfNitfImageHeaderV2_X::getAttachmentLevel()const
{
   return rspfString(theAttachmentLevel).toUInt32();
}

rspfString rspfNitfImageHeaderV2_X::getPixelType()const
{
   return thePixelValueType;
}

rspf_uint32 rspfNitfImageHeaderV2_X::getBitsPerPixel()const
{
   return rspfString(theNumberOfBitsPerPixelPerBand).toUInt32();
}

rspf_uint32 rspfNitfImageHeaderV2_X::getActualBitsPerPixel()const
{
   return rspfString(theActualBitsPerPixelPerBand).toUInt32();
}

char rspfNitfImageHeaderV2_X::getImageMode()const
{
   return theImageMode[0];
}

rspfString rspfNitfImageHeaderV2_X::getCategory()const
{
   return theCategory;
}

rspfString rspfNitfImageHeaderV2_X::getRepresentation()const
{
   return theRepresentation;
}

rspf_uint32 rspfNitfImageHeaderV2_X::getBlocksPerRow()const
{
   return rspfString(theNumberOfBlocksPerRow).toUInt32();
}

rspf_uint32 rspfNitfImageHeaderV2_X::getBlocksPerCol()const
{
   return rspfString(theNumberOfBlocksPerCol).toUInt32();
}

rspf_uint32 rspfNitfImageHeaderV2_X::getNumberOfPixelsPerBlockRow()const
{
   return rspfString(theNumberOfPixelsPerBlockVert).toUInt32();
}

rspf_uint32 rspfNitfImageHeaderV2_X::getNumberOfPixelsPerBlockCol()const
{
   return rspfString(theNumberOfPixelsPerBlockHoriz).toUInt32();
}

bool rspfNitfImageHeaderV2_X::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   // Note: Currently not looking up all fieds only ones that make sense.
   
   const char* lookup;

   lookup = kwl.find( prefix, IID1_KW);
   if ( lookup )
   {
      setImageId( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, IDATIM_KW);
   if ( lookup )
   {
      setAquisitionDateTime( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, TGTID_KW);
   if ( lookup )
   {
      setTargetId( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, IID2_KW);
   if ( lookup )
   {
      setTitle( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, ITITLE_KW); // duplicate of above
   if ( lookup )
   {
      setTitle( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, ISCLAS_KW);
   if ( lookup )
   {
      setSecurityClassification( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, ENCRYP_KW);
   if ( lookup )
   {
      setEncryption( rspfString(lookup) );
   }  
   lookup = kwl.find( prefix, ISORCE_KW);
   if ( lookup )
   {
      setImageSource( rspfString(lookup) );
   }
   lookup = kwl.find( prefix, ICAT_KW);
   if ( lookup )
   {
      setCategory( rspfString(lookup) );
   }
   
   return true;
}

void rspfNitfImageHeaderV2_X::setProperty(rspfRefPtr<rspfProperty> property)
{
   rspfString name = property->getName();

   // Make case insensitive:
   name.upcase();
   
   std::ostringstream out;
   
   if(!property) return;
   
   if(name.contains(IID1_KW))
   {
      setImageId(property->valueToString());
   }
   else if(name.contains(IDATIM_KW))
   {
      setAquisitionDateTime(property->valueToString());
   }
   else if(name.contains(TGTID_KW))
   {
      setTargetId(property->valueToString());
   }
   else if(name.contains(IID2_KW)||
           name.contains(ITITLE_KW))
   {
      setTitle(property->valueToString());
   }
   else if(name.contains(ISCLAS_KW))
   {
      setSecurityClassification(property->valueToString());
   }
   else if(name.contains(ENCRYP_KW))
   {
      setEncryption(property->valueToString());
   }
   else if(name.contains(ISORCE_KW))
   {
      setImageSource(property->valueToString());
   }
   else if(name.contains(PVTYPE_KW))
   {
      setPixelType(property->valueToString());
   }
   else if(name.contains(IREP_KW))
   {
      setRepresentation(property->valueToString());
   }
   else if(name.contains(ICAT_KW))
   {
      setCategory(property->valueToString());
   }
   else if(name.contains(ABPP_KW))
   {
      setActualBitsPerPixel(property->valueToString().toUInt32());
   }
   else if(name.contains(PJUST_KW))
   {
      setJustification(property->valueToString());
   }
   else if(name.contains(ICORDS_KW))
   {
      setCoordinateSystem(property->valueToString());
   }
   else if(name.contains(IGEOLO_KW))
   {
      setGeographicLocation(property->valueToString());
   }
   else if(name.contains(NICOM_KW))
   {
      setNumberOfComments(property->valueToString());
   }
   else if(name.contains(IC_KW))
   {
      setCompression(property->valueToString());
   }
   else if(name.contains(COMRAT_KW))
   {
      setCompressionRateCode(property->valueToString());
   }
   else if(name.contains(ISYNC_KW))
   {
   }
   else if(name.contains(IMODE_KW))
   {
      setImageMode(*(property->valueToString().trim().begin()));
   }
   else if(name.contains(NBPR_KW))
   {
      setBlocksPerRow(property->valueToString().toUInt32());
   }
   else if(name.contains(NBPC_KW))
   {
      setBlocksPerCol(property->valueToString().toUInt32());
   }
   else if(name.contains(NPPBH_KW))
   {
      setNumberOfPixelsPerBlockRow(property->valueToString().toUInt32());
   }
   else if(name.contains(NPPBV_KW))
   {
      setNumberOfPixelsPerBlockCol(property->valueToString().toUInt32());
   }
   else if(name.contains(NBPP_KW))
   {
      setBitsPerPixel(property->valueToString().toUInt32());
   }
   else if(name.contains(IDLVL_KW))
   {
      setDisplayLevel(property->valueToString());
   }
   else if(name.contains(IALVL_KW))
   {
      setAttachmentLevel(property->valueToString());
   }
   else if(name.contains(ILOC_KW))
   {
   }
   else if(name.contains(IMAG_KW))
   {
   }
   else
   {
      rspfNitfImageHeader::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfNitfImageHeaderV2_X::getProperty(const rspfString& name)const
{
   rspfProperty* property = 0;

   if(name == IID1_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theImageId).trim());
   }
   else if(name == IDATIM_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theDateTime).trim());
   }
   else if(name == TGTID_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theTargetId).trim());
   }
   else if((name == IID2_KW)||
           (name == ITITLE_KW))
   {
      property = new rspfStringProperty(name,
                                         rspfString(theTitle).trim());
   }
   else if(name == ISCLAS_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theSecurityClassification).trim());
   }
   else if(name == ENCRYP_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theEncryption).trim());
   }
   else if(name == ISORCE_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theImageSource).trim());
   }
   else if(name == PVTYPE_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(thePixelValueType).trim());
   }
   else if(name == IREP_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theRepresentation).trim());
   }
   else if(name == ICAT_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theCategory).trim());
   }
   else if(name == ABPP_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theActualBitsPerPixelPerBand).trim());
   }
   else if(name == PJUST_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theJustification).trim());
   }
   else if(name == ICORDS_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theCoordinateSystem).trim());
   }
   else if(name == IGEOLO_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theGeographicLocation).trim());
   }
   else if(name == NICOM_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theNumberOfComments).trim());
   }
   else if(name == IC_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theCompression).trim());
   }
   else if(name == COMRAT_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theCompressionRateCode).trim());
   }
   else if(name == ISYNC_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theImageSyncCode).trim());
   }
   else if(name == IMODE_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theImageMode).trim());
   }
   else if(name == NBPR_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theNumberOfBlocksPerRow).trim());
   }
   else if(name == NBPC_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theNumberOfBlocksPerCol).trim());
   }
   else if(name == NPPBH_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theNumberOfPixelsPerBlockHoriz).trim());
   }
   else if(name == NPPBV_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theNumberOfPixelsPerBlockVert).trim());
   }
   else if(name == NBPP_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theNumberOfBitsPerPixelPerBand).trim());
   }
   else if(name == IDLVL_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theDisplayLevel).trim());
   }
   else if(name == IALVL_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theAttachmentLevel).trim());
   }
   else if(name == ILOC_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theImageLocation).trim());
   }
   else if(name == IMAG_KW)
   {
      property = new rspfStringProperty(name,
                                         rspfString(theImageMagnification).trim());
   }
   else if(name == "source")
   {
      property = new rspfStringProperty(name,
                                         getImageSource());
   }
   else if(name == "image_date")
   {
      property = new rspfStringProperty(name,
                                         getAcquisitionDateMonthDayYear());
   }
   else if(name == "image_title")
   {
      property = new rspfStringProperty(name,
                                         getTitle());
   }
   else
   {
      return rspfNitfImageHeader::getProperty(name);
                                         
   }
   return property;
}

void rspfNitfImageHeaderV2_X::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfNitfImageHeader::getPropertyNames(propertyNames);
   propertyNames.push_back(IID1_KW);
   propertyNames.push_back(IDATIM_KW);
   propertyNames.push_back(TGTID_KW);
   propertyNames.push_back(IID2_KW);
   propertyNames.push_back(ISCLAS_KW);
   propertyNames.push_back(ENCRYP_KW);
   propertyNames.push_back(ISORCE_KW);
   propertyNames.push_back(PVTYPE_KW);
   propertyNames.push_back(IREP_KW);
   propertyNames.push_back(ICAT_KW);
   propertyNames.push_back(ABPP_KW);
   propertyNames.push_back(PJUST_KW);
   propertyNames.push_back(ICORDS_KW);
   propertyNames.push_back(IGEOLO_KW);
   propertyNames.push_back(NICOM_KW);
   propertyNames.push_back(IC_KW);
   propertyNames.push_back(COMRAT_KW);
   propertyNames.push_back(ISYNC_KW);
   propertyNames.push_back(IMODE_KW);
   propertyNames.push_back(NBPR_KW);
   propertyNames.push_back(NBPC_KW);
   propertyNames.push_back(NPPBH_KW);
   propertyNames.push_back(NPPBV_KW);
   propertyNames.push_back(NBPP_KW);
   propertyNames.push_back(IDLVL_KW);
   propertyNames.push_back(IALVL_KW);
   propertyNames.push_back(ILOC_KW);
   propertyNames.push_back(IMAG_KW);

   // Temp addition (drb)
   propertyNames.push_back(rspfString("source"));
   propertyNames.push_back(rspfString("image_date"));
   propertyNames.push_back(rspfString("image_title"));
}

rspfString rspfNitfImageHeaderV2_X::getImageMagnification()const
{
   return rspfString(theImageMagnification).trim();
}

void rspfNitfImageHeaderV2_X::getImageLocation(rspfIpt& loc)const
{
   loc.x = rspfString((char*)(&theImageLocation[5])).toInt32();
   loc.y = rspfString((char*)theImageLocation,
                       (char*)(&theImageLocation[5])).toInt32();
}

void rspfNitfImageHeaderV2_X::setGeographicLocationDms(const rspfDpt& ul,
                                                        const rspfDpt& ur,
                                                        const rspfDpt& lr,
                                                        const rspfDpt& ll)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
      << rspfDms(ul.y, true).toString("ddmmss.ssssC").c_str()
      << rspfDms(ul.x, false).toString("dddmmss.ssssC").c_str()
      << rspfDms(ur.y, true).toString("ddmmss.ssssC").c_str()
      << rspfDms(ur.x, false).toString("dddmmss.ssssC").c_str()
      << rspfDms(lr.y, true).toString("ddmmss.ssssC").c_str()
      << rspfDms(lr.x, false).toString("dddmmss.ssssC").c_str()
      << rspfDms(ll.y, true).toString("ddmmss.ssssC").c_str()
      << rspfDms(ll.x, false).toString("dddmmss.ssssC").c_str()
      << std::endl;
      
      checkForGeographicTiePointTruncation(ul);
      checkForGeographicTiePointTruncation(ur);
      checkForGeographicTiePointTruncation(lr);
      checkForGeographicTiePointTruncation(ll);
   }
   
   theCoordinateSystem[0] = 'G';
   
   memcpy(theGeographicLocation,
          rspfNitfCommon::encodeGeographicDms(ul,ur,lr,ll).c_str(), 60);
}

void rspfNitfImageHeaderV2_X::setGeographicLocationDecimalDegrees(
   const rspfDpt& ul,
   const rspfDpt& ur,
   const rspfDpt& lr,
   const rspfDpt& ll)
{
   theCoordinateSystem[0] = 'D';
   memcpy(theGeographicLocation,
          rspfNitfCommon::encodeGeographicDecimalDegrees(
             ul, ur, lr, ll).c_str(), 60);
}

void rspfNitfImageHeaderV2_X::setUtmNorth(rspf_uint32 zone,
                                           const rspfDpt& ul,
                                           const rspfDpt& ur,
                                           const rspfDpt& lr,
                                           const rspfDpt& ll)
{
   theCoordinateSystem[0] = 'N';
   
   memcpy(theGeographicLocation,
          rspfNitfCommon::encodeUtm(zone, ul, ur, lr, ll).c_str(), 60);
}

void rspfNitfImageHeaderV2_X::setUtmSouth(rspf_uint32 zone,
                                           const rspfDpt& ul,
                                           const rspfDpt& ur,
                                           const rspfDpt& lr,
                                           const rspfDpt& ll)
{
   theCoordinateSystem[0] = 'S';
   
   memcpy(theGeographicLocation,
          rspfNitfCommon::encodeUtm(zone, ul, ur, lr, ll).c_str(), 60);
}


bool rspfNitfImageHeaderV2_X::saveState(rspfKeywordlist& kwl, const rspfString& prefix)const
{
   bool result = rspfNitfImageHeader::saveState(kwl, prefix);
   
   kwl.add(prefix, IM_KW.c_str(),       theType);
   kwl.add(prefix, IID1_KW.c_str(),     theImageId);
   kwl.add(prefix, IDATIM_KW.c_str(),   theDateTime);
   kwl.add(prefix, TGTID_KW.c_str(),    theTargetId);
   kwl.add(prefix, IID2_KW.c_str(),     theTitle);
   kwl.add(prefix, ISCLAS_KW.c_str(),   theSecurityClassification);
   kwl.add(prefix, ENCRYP_KW.c_str(),   theEncryption);
   kwl.add(prefix, ISORCE_KW.c_str(),   theImageSource);
   kwl.add(prefix, NROWS_KW.c_str(),    theSignificantRows);
   kwl.add(prefix, NCOLS_KW.c_str(),    theSignificantCols);
   kwl.add(prefix, PVTYPE_KW.c_str(),   thePixelValueType);
   kwl.add(prefix, IREP_KW.c_str(),     theRepresentation);
   kwl.add(prefix, ABPP_KW.c_str(),     theActualBitsPerPixelPerBand);
   kwl.add(prefix, ICAT_KW.c_str(),     theCategory);
   kwl.add(prefix, PJUST_KW.c_str(),    theJustification);
   kwl.add(prefix, ICORDS_KW.c_str(),   theCoordinateSystem);
   kwl.add(prefix, NICOM_KW.c_str(),    theNumberOfComments);
   kwl.add(prefix, ICOM_KW.c_str(),     rspfString(theImageComments));
   kwl.add(prefix, IGEOLO_KW.c_str(),   theGeographicLocation);
   kwl.add(prefix, IC_KW.c_str(),       theCompression);
   kwl.add(prefix, COMRAT_KW.c_str(),   theCompressionRateCode);
   kwl.add(prefix, NBANDS_KW.c_str(),   theNumberOfBands);
   kwl.add(prefix, ISYNC_KW.c_str(),    theImageSyncCode);
   kwl.add(prefix, IMODE_KW.c_str(),    theImageMode);
   kwl.add(prefix, NBPR_KW.c_str(),     theNumberOfBlocksPerRow);
   kwl.add(prefix, NBPC_KW.c_str(),     theNumberOfBlocksPerCol);
   kwl.add(prefix, NPPBH_KW.c_str(),    theNumberOfPixelsPerBlockHoriz);
   kwl.add(prefix, NPPBV_KW.c_str(),    theNumberOfPixelsPerBlockVert);
   kwl.add(prefix, NBPP_KW.c_str(),     theNumberOfBitsPerPixelPerBand);
   kwl.add(prefix, IDLVL_KW.c_str(),    theDisplayLevel);
   kwl.add(prefix, IALVL_KW.c_str(),    theAttachmentLevel);
   kwl.add(prefix, ILOC_KW.c_str(),     theImageLocation);
   kwl.add(prefix, IMAG_KW.c_str(),     theImageMagnification);
   kwl.add(prefix, UDIDL_KW.c_str(),    theUserDefinedImageDataLength);
   kwl.add(prefix, UDOFL_KW.c_str(),    theUserDefinedOverflow);
   kwl.add(prefix, IXSHDL_KW.c_str(),   theExtendedSubheaderDataLen);
   kwl.add(prefix, IXSOFL_KW.c_str(),   theExtendedSubheaderOverflow);
   kwl.add(prefix, IMDATOFF_KW.c_str(), theBlockedImageDataOffset);
   kwl.add(prefix, BMRLNTH_KW.c_str(),  theBlockMaskRecordLength);
   
   return result;
}






