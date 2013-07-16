//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Ken Melero
// Description: This class encapsulates the image parameters and
//              support data for a ADRG image file(s).
//
//********************************************************************
// $Id: rspfAdrgHeader.cpp 13766 2008-10-22 19:31:32Z gpotts $

#include <cstring>
#include <iostream>

#include <rspf/base/rspfNotify.h>
#include <rspf/imaging/rspfAdrgHeader.h>
#include <rspf/imaging/rspfIso8211.h>

//**************************************************************************
// Constructor
//**************************************************************************
rspfAdrgHeader::rspfAdrgHeader(const rspfFilename& img_file)
   :
      theErrorStatus(RSPF_OK),
      theHeaderFile(img_file),
      theImageFile(img_file),
      theValidImageRect(0,0,0,0),
      thePixelType(RSPF_UCHAR),
      theInterleaveType(RSPF_BSQ),
      theNumberOfBands(3),
      theLines(0),
      theSamples(0),
      theHeaderSize(0),
      theTim(0)
{
   // Get the extension.
   rspfString ext = img_file.ext();

   if (ext == "IMG")
   {
      theHeaderFile.setExtension(rspfString("GEN"));
   }
   else if (ext == "img")
   {
      theHeaderFile.setExtension(rspfString("gen"));
   }
   else
   {
      // Required "img" or "IMG" extension.
      theErrorStatus = RSPF_ERROR;
      return;
   }

   // Check for header file.
   if(theHeaderFile.exists() == false)
   {
      theErrorStatus = RSPF_ERROR;
      return;
   }

   // Parse header for all needed support data.
   parse();
}

//**************************************************************************
// Destructor
//**************************************************************************
rspfAdrgHeader::~rspfAdrgHeader()
{
   if(theTim)
   {
      delete [] theTim;
      theTim = 0;
   }
}

//**************************************************************************
// Parse header
//**************************************************************************
void rspfAdrgHeader::parse()
{
   // NOTE:  By default, ISO8211 library reports errors to stderr,
   //        so we don't bother doing it ourselves.  We simply set
   //        the RSPF_ERROR flag.

   rspfDDFModule oModule;
   rspfDDFRecord* oRecord;

   // Open header file which is ISO8211 encoded.
   if(!oModule.Open(theHeaderFile.chars()) )
   {
      theErrorStatus = RSPF_ERROR;
      return;
   }

   oRecord = oModule.ReadRecord();

   rspfDDFField* oField = oRecord->FindField("DRF");
   int bytesOfData = oField->GetDataSize();
   int nBytesConsumed = 0;
   const char* rawFieldData;

   rspfDDFFieldDefn* fieldDefn = oField->GetFieldDefn();
   rspfDDFSubfieldDefn* nfile = fieldDefn->FindSubfieldDefn("NOZ");
   rawFieldData = oField->GetSubfieldData(nfile);
   rspfString num_img_files = nfile->ExtractStringData(rawFieldData,
                                                        bytesOfData,
                                                        &nBytesConsumed);

   // Number of ZDR images
   rspf_uint32 num_ifiles = (rspf_uint32)rspfString::toInt(num_img_files);

   // Skip overview record.
   oRecord = oModule.ReadRecord();

   // Validate this is the correct record and it is an ADRG image.
   rspfString img_file = "";
   rspfString img_base = theImageFile.fileNoExtension();
   img_base.downcase();

   // Find the correct geometry record corresponding to the image file.
   rspf_uint32 count = 0;
   while(count < num_ifiles)
   {
      oField = oRecord->FindField("001");
      bytesOfData = oField->GetDataSize();
      nBytesConsumed = 0;
      
      fieldDefn = oField->GetFieldDefn();
      rspfDDFSubfieldDefn* rty = fieldDefn->FindSubfieldDefn("RTY");  
      rawFieldData = oField->GetSubfieldData(rty);
      rspfString rtype = rty->ExtractStringData(rawFieldData,
                                                 bytesOfData,
                                                 &nBytesConsumed);

      // Found general information record
      if(rtype == "GIN")
      {
         // Validate that this is an ADRG header file.
         oField = oRecord->FindField("DSI");
         bytesOfData = oField->GetDataSize();
         nBytesConsumed = 0;
         
         fieldDefn = oField->GetFieldDefn();
         rspfDDFSubfieldDefn* ptype = fieldDefn->FindSubfieldDefn("PRT");  
         rawFieldData = oField->GetSubfieldData(ptype);
         rspfString product_type = ptype->ExtractStringData(rawFieldData,
                                                             bytesOfData,
                                                             &nBytesConsumed);
         
         if(!(strncmp(product_type.c_str(), "ADRG", 4) == 0))
         {
            rspfNotify(rspfNotifyLevel_NOTICE)
               << "File does not appear to be an ADRG header file: "
               << theHeaderFile.chars() << std::endl;
            
            theErrorStatus = RSPF_ERROR;
            return;      
         }
         
         fieldDefn = oField->GetFieldDefn();
         rspfDDFSubfieldDefn* ifile = fieldDefn->FindSubfieldDefn("NAM");
         rawFieldData = oField->GetSubfieldData(ifile);
         img_file = ifile->ExtractStringData(rawFieldData, bytesOfData,
                                             &nBytesConsumed);
         img_file.downcase();
         
         count++;

         if(img_base == img_file)
            break;
         else
            oRecord = oModule.ReadRecord();
      }
      else
         oRecord = oModule.ReadRecord();
   }

   // Find Distribution Rectangle name.
   oField = oRecord->FindField("GEN");
   bytesOfData = oField->GetDataSize();
   nBytesConsumed = 0;
   
   fieldDefn = oField->GetFieldDefn();

   // Find Subfield names.
   rspfDDFSubfieldDefn* min_lon = fieldDefn->FindSubfieldDefn("SWO");  
   rspfDDFSubfieldDefn* min_lat = fieldDefn->FindSubfieldDefn("SWA");  
   rspfDDFSubfieldDefn* max_lon = fieldDefn->FindSubfieldDefn("NEO");  
   rspfDDFSubfieldDefn* max_lat = fieldDefn->FindSubfieldDefn("NEA");  
      
   // Extract the min/max lon/lat coordinates.
   rawFieldData = oField->GetSubfieldData(min_lon);
   theMinLon = min_lon->ExtractStringData(rawFieldData,
                                          bytesOfData,
                                          &nBytesConsumed);
   rawFieldData = oField->GetSubfieldData(min_lat);
   theMinLat = min_lat->ExtractStringData(rawFieldData,
                                          bytesOfData,
                                          &nBytesConsumed);
   rawFieldData = oField->GetSubfieldData(max_lon);
   theMaxLon = max_lon->ExtractStringData(rawFieldData,
                                          bytesOfData,
                                          &nBytesConsumed);
   rawFieldData = oField->GetSubfieldData(max_lat);
   theMaxLat = max_lat->ExtractStringData(rawFieldData,
                                          bytesOfData,
                                          &nBytesConsumed);   
   
   // Find the DATA_SET_PARAMETERS_FIELD
   oField = oRecord->FindField("SPR");
   bytesOfData = oField->GetDataSize();
   nBytesConsumed = 0;

   fieldDefn = oField->GetFieldDefn();

   // Find subfields.
   rspfDDFSubfieldDefn* start_row = fieldDefn->FindSubfieldDefn("NUL");   
   rspfDDFSubfieldDefn* stop_col  = fieldDefn->FindSubfieldDefn("NUS");   
   rspfDDFSubfieldDefn* stop_row  = fieldDefn->FindSubfieldDefn("NLL");   
   rspfDDFSubfieldDefn* start_col = fieldDefn->FindSubfieldDefn("NLS");   
   rspfDDFSubfieldDefn* lines     = fieldDefn->FindSubfieldDefn("NFL");   
   rspfDDFSubfieldDefn* samples   = fieldDefn->FindSubfieldDefn("NFC");   
   rspfDDFSubfieldDefn* tif       = fieldDefn->FindSubfieldDefn("TIF");

   // Extract the number of lines & samples per tile.
   rawFieldData = oField->GetSubfieldData(start_row);
   theStartRow = atoi(start_row->ExtractStringData(rawFieldData,
                                                   bytesOfData,
                                                   &nBytesConsumed));
   rawFieldData = oField->GetSubfieldData(start_col);
   theStartCol = atoi(start_col->ExtractStringData(rawFieldData,
                                                   bytesOfData,
                                                   &nBytesConsumed));
   rawFieldData = oField->GetSubfieldData(stop_row);
   theStopRow = atoi(stop_row->ExtractStringData(rawFieldData,
                                                   bytesOfData,
                                                   &nBytesConsumed));
   rawFieldData = oField->GetSubfieldData(stop_col);
   theStopCol = atoi(stop_col->ExtractStringData(rawFieldData,
                                                   bytesOfData,
                                                   &nBytesConsumed));
   rawFieldData = oField->GetSubfieldData(lines);
   theLinesInTiles = atoi(lines->ExtractStringData(rawFieldData,
                                                   bytesOfData,
                                                   &nBytesConsumed));
   rawFieldData = oField->GetSubfieldData(samples);
   theSamplesInTiles = atoi(samples->ExtractStringData(rawFieldData,
                                                       bytesOfData,
                                                       &nBytesConsumed));
   rawFieldData = oField->GetSubfieldData(tif);
   theTif = tif->ExtractStringData(rawFieldData, bytesOfData,
                                   &nBytesConsumed);
   
   theLines = theLinesInTiles * ADRG_TILE_SIZE;
   theSamples = theSamplesInTiles * ADRG_TILE_SIZE;  

   theValidImageRect = rspfIrect(0, 0, theSamples - 1, theLines - 1);

   if(theTim)
   {
      delete [] theTim;
      theTim = 0;
   }
   // Initialize the TIM (tile index map).
   theTim = new rspf_uint32[theLinesInTiles * theSamplesInTiles];

   // Read in the tile index map (TIM) if it exists.
   if(strncmp(theTif.c_str(), "Y", 1) == 0)
   {
      // Find the TILE_INDEX_MAP_FIELD
      oField = oRecord->FindField("TIM");
      bytesOfData = oField->GetDataSize();
      fieldDefn = oField->GetFieldDefn();
      rawFieldData = oField->GetData();
      nBytesConsumed = 0;

      for(rspf_uint32 r = 0; r < theLinesInTiles * theSamplesInTiles; r++ )
      {
         rspfDDFSubfieldDefn* tim = fieldDefn->FindSubfieldDefn("TSI");
         theTim[r] = atoi(tim->ExtractStringData(rawFieldData, bytesOfData,
                                                 &nBytesConsumed));
         bytesOfData -= nBytesConsumed;
         rawFieldData += nBytesConsumed;
      }
   }
   else
   {
      for(rspf_uint32 m = 0; m < (theLinesInTiles * theSamplesInTiles); m++)
         theTim[m] = (m+1);
   }
   
   // Close module used for reading encoded header file.
   oModule.Close();
}

//*********************************************************************
// tim(int32 row, int32 col)
//*********************************************************************
rspf_uint32 rspfAdrgHeader::tim(rspf_uint32 row, rspf_uint32 col) const
{
   return ( theTim[(row * theSamplesInTiles) + col] );
}

//*********************************************************************
// operator <<
//*********************************************************************
std::ostream& operator<<(std::ostream& os, const rspfAdrgHeader& adrg)
{
   os << "\nContents of ADRG Header file (.gen):"
      << "\nImage File = " << adrg.theImageFile
      << "\nHeader File = " << adrg.theHeaderFile
      << "\nPixelType = " << adrg.thePixelType
      << "\nInterleaveType = " << adrg.theInterleaveType
      << "\nNumberOfBands = " << adrg.theNumberOfBands
      << "\nLines = " << adrg.theLines
      << "\nLines (tiles) = " << adrg.theLinesInTiles
      << "\nSamples = " << adrg.theSamples      
      << "\nSamples (tiles) = " << adrg.theSamplesInTiles
      << "\nHeaderSize = " << adrg.theHeaderSize
      << "\nValidImageRect = " << adrg.theValidImageRect
      << "\nTIF = " << adrg.theTif
      << "\nMin Lon = " << adrg.theMinLon
      << "\nMin Lat = " << adrg.theMinLat
      << "\nMax Lon = " << adrg.theMaxLon
      << "\nMax Lat = " << adrg.theMaxLat
      << "\nStart Row = " << adrg.theStartRow
      << "\nStart Col = " << adrg.theStartCol
      << "\nStop Row = " << adrg.theStopRow
      << "\nStop Col = " << adrg.theStopCol
      << std::endl;

   for(rspf_uint32 m = 0; m < (adrg.theLinesInTiles * adrg.theSamplesInTiles); m++)
      os << "\nTIM[" << m << "] = " << adrg.theTim[m];

   os << std::endl;

   return(os);
}

//*********************************************************************
// Public method:
//*********************************************************************
double rspfAdrgHeader::minLongitude() const
{
   return parseLongitudeString(theMinLon);
}

//*********************************************************************
// Public method:
//*********************************************************************
double rspfAdrgHeader::minLatitude() const
{
   return parseLatitudeString(theMinLat);
}

//*********************************************************************
// Public method:
//*********************************************************************
double rspfAdrgHeader::maxLongitude() const
{
   return parseLongitudeString(theMaxLon);
}

//*********************************************************************
// Public method:
//*********************************************************************
double rspfAdrgHeader::maxLatitude() const
{
   return parseLatitudeString(theMaxLat);
}

//*********************************************************************
// Private method:
//*********************************************************************
double rspfAdrgHeader::parseLongitudeString(const rspfString& lon) const
{
   static const char MODULE[] = "rspfAdrgHeader::parseLongitudeString";

   // Format = +|-dddmmss.ss
   
   if (lon.size() != 11)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " Longitude string error!"
         << "Bad longitude string:  " << lon.c_str() << std::endl;
   }
         
   const char* cstr = lon.c_str();

   double degrees = 0.0;
   double sign = 1.0;
   char tmp_str[6];
   
   // Get the sign.
   if (cstr[0] == '-') sign = -1.0;

   // Get the degrees.
   tmp_str[3] = '\0';
   cstr++; // Increment past the sign. 
   strncpy(tmp_str, cstr, 3);
   degrees = atof(tmp_str);
   
   // Get the minutes.
   tmp_str[2] = '\0';
   cstr += 3; // Increment to the minutes.
   strncpy(tmp_str, cstr, 2);
   degrees += atof(tmp_str) / 60.0;
   
   // Get the seconds.
   tmp_str[5] = '\0';
   cstr += 2; // Increment to seconds.
   strncpy(tmp_str, cstr, 5);
   degrees += atof(tmp_str) / 3600.0;
   
   // Set the sign.
   degrees *= sign;

   return degrees;
}

//*********************************************************************
// Private method:
//*********************************************************************
double rspfAdrgHeader::parseLatitudeString(const rspfString& lat) const
{
   static const char MODULE[] = "rspfAdrgHeader::parseLatitudeString";
   
   // Format = +|-ddmmss.ss
   
   if (lat.size() != 10)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " Latitude string error!"
         << "Bad latitude string:  " << lat.c_str() << std::endl;
   }
   
   const char* cstr = lat.c_str();
   
   double degrees = 0.0;
   double sign = 1.0;
   char tmp_str[6];
   
   // Get the sign.
   if (cstr[0] == '-') sign = -1.0;
   
   // Get the degrees.
   tmp_str[2] = '\0';
   cstr++; // Increment past the sign. 
   strncpy(tmp_str, cstr, 2);
   degrees = atof(tmp_str);
   
   // Get the minutes.
   tmp_str[2] = '\0';
   cstr += 2; // Increment to the minutes.
   strncpy(tmp_str, cstr, 2);
   degrees += atof(tmp_str) / 60.0;
   
   // Get the seconds.
   tmp_str[5] = '\0';
   cstr += 2; // Increment to seconds.
   strncpy(tmp_str, cstr, 5);
   degrees += atof(tmp_str) / 3600.0;
   
   // Set the sign.
   degrees *= sign;
   
   return degrees;
}
