#include <rspf/support_data/rspfRpfMaskSubsection.h>
#include <rspf/support_data/rspfRpfMaskSubheader.h>
#include <rspf/base/rspfErrorCodes.h>

std::ostream& operator <<(std::ostream& out,
                          const rspfRpfMaskSubsection& data)
{
   data.print(out);
   return out;
}

rspfRpfMaskSubsection::rspfRpfMaskSubsection()
   :theMaskSubheader(0)
{
   theMaskSubheader = new rspfRpfMaskSubheader;
}

rspfRpfMaskSubsection::~rspfRpfMaskSubsection()
{
   if(theMaskSubheader)
   {
      delete theMaskSubheader;
      theMaskSubheader = 0;
   }
}

rspfErrorCode rspfRpfMaskSubsection::parseStream(std::istream& in,
                                                   rspfByteOrder byteOrder)
{
   rspfErrorCode result = rspfErrorCodes::RSPF_OK;
   
   if(in&&theMaskSubheader)
   {
      result = theMaskSubheader->parseStream(in, byteOrder);
   }
   else
   {
      result = rspfErrorCodes::RSPF_ERROR;
   }
   
   return result;
}

void rspfRpfMaskSubsection::print(std::ostream& out)const
{
   if(theMaskSubheader)
   {
      out << *theMaskSubheader;
   }
}
