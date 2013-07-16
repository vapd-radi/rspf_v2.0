//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// 
//********************************************************************
// $Id: rspfHexString.cpp 9963 2006-11-28 21:11:01Z gpotts $
#include <rspf/base/rspfHexString.h>

rspf_int32 rspfHexString::toInt32()const
{
   return static_cast<rspf_int32>(toUint32());
}

rspf_int16 rspfHexString::toInt16()const
{
   return static_cast<rspf_int16>(toUint32());
}

rspf_int8 rspfHexString::toChar()const
{
   return static_cast<rspf_int8>(toUint32());
}

rspf_uint8 rspfHexString::toUchar()const
{   
   return static_cast<rspf_uint8>(toUint32());
}

rspf_uint16 rspfHexString::toUint16()const
{
   return static_cast<rspf_uint16>(toUint32());
}

rspf_uint32 rspfHexString::toUint32()const
{
   rspf_uint32 temp=0;

   if(length())
   {
      rspf_uint32 i = 0;
      
      if(((*this)[0] == '0')&&
         (((*this)[1] == 'x')||
          ((*this)[1] == 'X')))
      {
         i = 2;
      }
         
      for(; i <length(); ++i)
      {
         temp <<=4;
         if( ((*this)[(int)i] >= '0')&&((*this)[(int)i] <= '9'))
         {
            temp |= (rspf_uint32)((*this)[(int)i] - '0');
         }
         else if( ((*this)[(int)i] >= 'A') && ((*this)[(int)i] <= 'F'))
         {
            temp |= (rspf_uint32)(10+((*this)[(int)i] - 'A'));
         }
         else if( ((*this)[(int)i] <= 'f') && ((*this)[(int)i] >= 'a'))
         {
            temp |= (rspf_uint32)(10 + ((*this)[(int)i] - 'a'));
         }
      }
   }
   
   return temp;
}

void rspfHexString::assign(rspf_uint8 value)
{
   *this = "";
   
   rspf_int8 v1 = (rspf_int8)(value&0x0F);
   rspf_int8 v2 = (rspf_int8)((value>>4)&0x0F);
   if(v2 <=9)
   {
      (*this) += (rspf_int8)(v2+'0');
   }
   else
   {
      (*this) += (rspf_int8)(v2-10 +'A');
   }

   if(v1 <=9)
   {
      (*this) += (rspf_int8)(v1+'0');
   }
   else
   {
      (*this) += (rspf_int8)(v1-10 +'A');
   }   
}

void rspfHexString::assign(rspf_uint16 value)
{
   *this = "";
   
   rspf_int8 v1 = (rspf_int8)(value&0x000F);
   value >>=4;
   rspf_int8 v2 = (rspf_int8)((value)&0x000F);
   value >>=4;
   rspf_int8 v3 = (rspf_int8)((value)&0x000F);
   value >>=4;
   rspf_int8 v4 = (rspf_int8)((value)&0x000F);

   if(v4 <=9)
   {
      (*this) += (rspf_int8)(v4+'0');
   }
   else
   {
      (*this) += (rspf_int8)(v4-10 +'A');
   }

   if(v3 <=9)
   {
      (*this) += (rspf_int8)(v3+'0');
   }
   else
   {
      (*this) += (rspf_int8)(v3-10 +'A');
   }

   if(v2 <=9)
   {
      (*this) += (rspf_int8)(v2+'0');
   }
   else
   {
      (*this) += (rspf_int8)(v2-10 +'A');
   }

   if(v1 <=9)
   {
      (*this) += (rspf_int8)(v1+'0');
   }
   else
   {
      (*this) += (rspf_int8)(v1-10 +'A');
   }
}

void rspfHexString::assign(rspf_uint32 value)
{
   *this = "";

   rspf_int8 v1 = (rspf_int8)(value&0x0000000F);
   value >>=4;
   rspf_int8 v2 = (rspf_int8)((value)&0x0000000F);
   value >>=4;
   rspf_int8 v3 = (rspf_int8)((value)&0x0000000F);
   value >>=4;
   rspf_int8 v4 = (rspf_int8)((value)&0x0000000F);
   value >>=4;
   rspf_int8 v5 = (rspf_int8)((value)&0x0000000F);
   value >>=4;
   rspf_int8 v6 = (rspf_int8)((value)&0x0000000F);
   value >>=4;
   rspf_int8 v7 = (rspf_int8)((value)&0x0000000F);
   value >>=4;
   rspf_int8 v8 = (rspf_int8)((value)&0x0000000F);

   if(v8 <=9)
   {
      (*this) += (rspf_int8)(v8+'0');
   }
   else
   {
      (*this) += (rspf_int8)(v8-10 +'A');
   }
   if(v7 <=9)
   {
      (*this) += (rspf_int8)(v7+'0');
   }
   else
   {
      (*this) += (rspf_int8)(v7-10 +'A');
   }
   if(v6 <=9)
   {
      (*this) += (rspf_int8)(v6+'0');
   }
   else
   {
      (*this) += (rspf_int8)(v6-10 +'A');
   }
   if(v5 <=9)
   {
      (*this) += (rspf_int8)(v5+'0');
   }
   else
   {
      (*this) += (rspf_int8)(v5-10 +'A');
   }

   if(v4 <=9)
   {
      (*this) += (rspf_int8)(v4+'0');
   }
   else
   {
      (*this) += (rspf_int8)(v4-10 +'A');
   }

   if(v3 <=9)
   {
      (*this) += (rspf_int8)(v3+'0');
   }
   else
   {
      (*this) += (rspf_int8)(v3-10 +'A');
   }

   if(v2 <=9)
   {
      (*this) += (rspf_int8)(v2+'0');
   }
   else
   {
      (*this) += (rspf_int8)(v2-10 +'A');
   }

   if(v1 <=9)
   {
      (*this) += (rspf_int8)(v1+'0');
   }
   else
   {
      (*this) += (rspf_int8)(v1-10 +'A');
   }
}
