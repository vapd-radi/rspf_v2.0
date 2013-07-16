//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfMaskSubsection.h 11691 2007-09-09 20:02:30Z dburken $
#ifndef rspfRpfMaskSubsection_HEADER
#define rspfRpfMaskSubsection_HEADER
#include <iostream>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorContext.h>

class rspfRpfMaskSubheader;
class rspfRpfImageDescriptionSubheader;

class RSPF_DLL rspfRpfMaskSubsection
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfRpfMaskSubsection& data);
   rspfRpfMaskSubsection();
   virtual ~rspfRpfMaskSubsection();

   rspfErrorCode parseStream(std::istream& in, rspfByteOrder byteOrder);
   virtual void print(std::ostream& out)const;
   
private:
   rspfRpfMaskSubheader *theMaskSubheader;   
};

#endif
