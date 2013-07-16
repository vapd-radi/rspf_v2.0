//*******************************************************************
// Copyright (C) 2000 Intelligence Data Systems. 
//
// LICENSE: LGPL
//
// see top level LICENSE.txt
// 
// Author: Garrett Potts
//
// Description: Nitf support class for RPC00A -
// Rational Polynomial Coefficient extension.
//
//********************************************************************
// $Id: rspfNitfRpcATag.h 22013 2012-12-19 17:37:20Z dburken $
#ifndef rspfNitfRpcATag_HEADER
#define rspfNitfRpcATag_HEADER 1

#include <rspf/support_data/rspfNitfRpcBase.h>

/**
 * The layout of RPC00B is the same as RPC00A.  The difference is how the
 * coefficients are used; hence, all the code is in the rspfNitfRpcBase class.
 */
class RSPF_DLL rspfNitfRpcATag : public rspfNitfRpcBase
{
public:
   
   rspfNitfRpcATag();
   
protected:
   
TYPE_DATA   
};

#endif
