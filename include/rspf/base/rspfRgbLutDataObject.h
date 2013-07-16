//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfRgbLutDataObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfRgbLutDataObject_HEADER
#define rspfRgbLutDataObject_HEADER
#include <rspf/base/rspfRgbVector.h>
#include <rspf/base/rspfObject.h>
#include <iosfwd>

class rspfKeywordlist;

class RSPFDLLEXPORT rspfRgbLutDataObject : public rspfObject
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfRgbLutDataObject& lut);
   /*!
    * Will allocate 256 values for the color table
    */
   rspfRgbLutDataObject(unsigned long numberOfEntries=0);
   rspfRgbLutDataObject(const rspfRgbLutDataObject& lut);
   virtual rspfObject* dup()const
   {
      return new rspfRgbLutDataObject(*this);
   }
   const rspfRgbVector& operator[](unsigned int index)const
      {
         return index<theNumberOfEntries?theLut[index]:theLut[theNumberOfEntries-1];
      }
   rspfRgbVector& operator[](unsigned int index)
      {
         return index<theNumberOfEntries?theLut[index]:theLut[theNumberOfEntries-1];
      }
   const rspfRgbVector& operator[](int index)const
      {
         return index<(long)theNumberOfEntries?theLut[index]:theLut[theNumberOfEntries-1];
      }
   rspfRgbVector& operator[](int index)
      {
         return index<(long)theNumberOfEntries?theLut[index]:theLut[theNumberOfEntries-1];
      }
   const rspfRgbVector& operator[](double normalizedIndex)const
      {
         int index = int(normalizedIndex*theNumberOfEntries+.5);
         if (index < 0)
         {
            index = 0;
         }
         return (*this)[index];
      }
   rspfRgbVector& operator[](double normalizedIndex)
      {
         int index = int(normalizedIndex*theNumberOfEntries+.5);
         if (index < 0)
         {
            index = 0;
         }
         return (*this)[index];
      }
   long getNumberOfEntries()const{return theNumberOfEntries;}
   int findIndex(const rspfRgbVector& rgb)
      {
         return findIndex(rgb.getR(),
                          rgb.getG(),
                          rgb.getB());
      }
   int findIndex(rspf_uint8 r, rspf_uint8 g, rspf_uint8 b);
   rspfRgbLutDataObject& rotate(long numberOfElements=1);
   rspfRgbLutDataObject  rotate(long numberOfElements=1)const;
   
   const rspfRgbLutDataObject& operator =(const rspfRgbLutDataObject& lut);
   bool operator ==(const rspfRgbLutDataObject& lut)const;

   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
protected:
   virtual ~rspfRgbLutDataObject();

   rspfRgbVector *theLut;
   rspf_uint32 theNumberOfEntries;

TYPE_DATA
};

#endif
