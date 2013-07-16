//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfRgbLutDataObject.cpp 13710 2008-10-14 16:27:57Z gpotts $
#include <rspf/base/rspfRgbLutDataObject.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfColumnVector3d.h>
#include <rspf/base/rspfNotifyContext.h>

#include <sstream>
using namespace std;

RTTI_DEF1(rspfRgbLutDataObject, "rspfRgbLutDataObject", rspfObject);

ostream& operator <<(ostream& out,
                     const rspfRgbLutDataObject& lut)
{
   for(rspf_uint32 index = 0; index < lut.theNumberOfEntries; ++index)
   {
      out << "entry" << index << " " << lut[index] << endl;
   }

   return out;
}

rspfRgbLutDataObject::rspfRgbLutDataObject(unsigned long numberOfEntries)
   :
      theLut(NULL),
      theNumberOfEntries(numberOfEntries)
{
   if(theNumberOfEntries > 0)
   {
      // allocate 256 entries for the data object;
      theLut = new rspfRgbVector[theNumberOfEntries];
   }
      
}

rspfRgbLutDataObject::rspfRgbLutDataObject(const rspfRgbLutDataObject& lut)
   :
      theLut(NULL),
      theNumberOfEntries(0)
{
   theNumberOfEntries = lut.theNumberOfEntries;
   if(theNumberOfEntries > 0)
   {
      theLut = new rspfRgbVector[theNumberOfEntries];
      for(rspf_uint32 index = 0; index < theNumberOfEntries; ++index)
      {
         theLut[index] = lut.theLut[index];
      }
   }
}

rspfRgbLutDataObject::~rspfRgbLutDataObject()
{
   if(theLut)
   {
      delete [] theLut;
      theLut = NULL;
   }
   theNumberOfEntries = 0;
}

int rspfRgbLutDataObject::findIndex(rspf_uint8 r, rspf_uint8 g, rspf_uint8 b)
{
   rspf_uint32 distance = 0x7FFFFFFF; // max 4 byte signed
   rspf_int32 result   = 0;

   if(theNumberOfEntries > 0)
   {
      for(rspf_uint32 i = 0; i < theNumberOfEntries; ++i)
      {
         rspf_uint32 rDelta = r - theLut[i].getR();
         rspf_uint32 gDelta = g - theLut[i].getG();
         rspf_uint32 bDelta = b - theLut[i].getB();

         rspf_uint32 deltaSumSquare = (rDelta*rDelta +
                                        gDelta*gDelta +
                                        bDelta*bDelta);
         if(deltaSumSquare == 0)
         {
            return static_cast<int>(i);
         }
         else if( deltaSumSquare < distance)
         {
            result = static_cast<int>(i);
            distance = deltaSumSquare;
         }
      }
   }

   return result;
}

rspfRgbLutDataObject rspfRgbLutDataObject::rotate(long numberOfElements)const
{
   if(numberOfElements < 0)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL: Negative rotation is not supported yet in rspfRgbLutDataObject::rotate" << endl;
      return *this;
   }
   rspfRgbLutDataObject lut;
   for(rspf_uint32 index = 0; index < theNumberOfEntries; ++index)
   {
      int adjustedDestinationIndex = (index+numberOfElements)%theNumberOfEntries;
      lut[adjustedDestinationIndex] = theLut[index] ;
   }

   return lut;
}

rspfRgbLutDataObject& rspfRgbLutDataObject::rotate(long numberOfElements)
{
   if(numberOfElements < 0)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL: Negative rotation is not supported yet in rspfRgbLutDataObject::rotate" << endl;
      return *this;
   }
   const rspfRgbLutDataObject* temp = this;

   *this = temp->rotate(numberOfElements);

   return *this;
}

const rspfRgbLutDataObject& rspfRgbLutDataObject::operator =(const rspfRgbLutDataObject& lut)
{
   if(theNumberOfEntries != lut.theNumberOfEntries)
   {
      delete [] theLut;
      theLut = NULL;
   }

   theNumberOfEntries = lut.theNumberOfEntries;
   if(!theLut&&(theNumberOfEntries > 0))
   {
      theLut = new rspfRgbVector[theNumberOfEntries];
   }
   for(unsigned long index = 0; index < theNumberOfEntries; ++index)
   {
      theLut[index] = lut.theLut[index];
   }

   return *this;
}

bool rspfRgbLutDataObject::operator ==(const rspfRgbLutDataObject& lut)const
{
   if(theNumberOfEntries != lut.theNumberOfEntries)
   {
      return false;
   }
   for(unsigned long index = 0; index < theNumberOfEntries; ++index)
   {
      if(theLut[index] != lut.theLut[index])
      {
         return false;
      }
   }
   return true;
}

bool rspfRgbLutDataObject::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   kwl.add(prefix,
           "type",
           getClassName(),
           true);
   kwl.add(prefix,
           "number_of_entries",
           rspfString::toString(theNumberOfEntries).c_str(),
           true);
   for(rspf_uint32 index = 0; index < theNumberOfEntries; ++index)
   {
      rspfString newPrefix = "entry";
      newPrefix += rspfString::toString(index);
      ostringstream ostr;
      ostr << (int)(theLut[index].getR())
           << " " << (int)(theLut[index].getG())
           << " " << (int)(theLut[index].getB());
      kwl.add(prefix,
              newPrefix,
              ostr.str().c_str(),
              true);
   }

   return true;
}

bool rspfRgbLutDataObject::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   const char* lutFile = kwl.find(prefix, "lut_file");
   rspfKeywordlist fileLut;
   rspfKeywordlist* tempKwl = (const_cast<rspfKeywordlist*>(&kwl));
   rspfString tempPrefix = prefix;

   // this should have been used instead of lut_file.  We will still look
   // for lut_file for backward compatibility.
   //
   if(!lutFile)
   {
      lutFile = kwl.find(prefix, "filename");
   }
   // check to see if we should open an external file
   // if so point the fileLut to the one that we use
   if(lutFile)
   {
      rspfFilename filename(lutFile);
      if(filename.exists())
      {
         fileLut.addFile(filename.c_str());
         tempKwl = &fileLut;
         tempPrefix = "";
      }
   }
 
   const char* numberOfEntries = tempKwl->find(tempPrefix, "number_of_entries");
   if(!numberOfEntries)
   {
      numberOfEntries = tempKwl->find(tempPrefix, "number_entries");
   }
   if(!numberOfEntries) return false;
   theNumberOfEntries = rspfString(numberOfEntries).toULong();

   delete [] theLut;
   theLut = new rspfRgbVector[theNumberOfEntries];

   if(tempKwl->find(tempPrefix, "entry0"))
   {
      for(rspf_uint32 index = 0; index < theNumberOfEntries; ++index)
      {
         rspfString newPrefix = "entry";
         newPrefix += rspfString::toString(index);
         rspfString v = tempKwl->find(tempPrefix, newPrefix.c_str());
         istringstream istr(v);

         rspfString r, g, b;
         istr >> r >> g >> b;
         theLut[index].setR((unsigned char)r.toInt32());
         theLut[index].setG((unsigned char)g.toInt32());
         theLut[index].setB((unsigned char)b.toInt32());
      }
   }
   else
   {
      for(rspf_uint32 index = 0; index < theNumberOfEntries; ++index)
      {
         rspfString newPrefix = "entry";
         newPrefix += rspfString::toString(index);
         
         const char* r = tempKwl->find(tempPrefix, (newPrefix+".r").c_str());
         const char* g = tempKwl->find(tempPrefix, (newPrefix+".g").c_str());
         const char* b = tempKwl->find(tempPrefix, (newPrefix+".b").c_str());
         
         if(r)
         {
            theLut[index].setR((unsigned char)rspfString(r).toLong());
         }
         if(g)
         {
            theLut[index].setG((unsigned char)rspfString(g).toLong());
         }
         if(b)
         {
            theLut[index].setB((unsigned char)rspfString(b).toLong());
         }
      }
   }
   return true;
}
