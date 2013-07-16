//*******************************************************************
//
// LICENSE: LGPL see top level directory for more details
//
// Author:  David Burken <dburken@imagelinks.com>
//
//********************************************************************
// $Id: rspfNitfImageBandV2_1.h 15416 2009-09-11 20:58:51Z dburken $

#ifndef rspfNitfImageBandV2_1_HEADER
#define rspfNitfImageBandV2_1_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/support_data/rspfNitfImageBandV2_0.h>

class RSPF_DLL rspfNitfImageBandV2_1 : public rspfNitfImageBandV2_0
{
public:

   /** default constructor */
   rspfNitfImageBandV2_1();

   /** virtual destructory */
   virtual ~rspfNitfImageBandV2_1();
};

#endif
