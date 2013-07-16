//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
// Description: This class give the capability to access tiles from an
//              vpf file.
//
//********************************************************************
// $Id: vpf.h 13586 2008-09-17 01:35:25Z gpotts $
#ifndef VPF_HEADER
#define VPF_HEADER

#include <rspf/rspfConfig.h>

#if __rspf_CARBON__
#define MAXINT INT_MAX
#endif

extern "C"
{
#include <rspf/vpfutil/vpfapi.h>
#include <rspf/vpfutil/vpfview.h>
#include <rspf/vpfutil/vpfselec.h>
#include <rspf/vpfutil/vpftable.h>
#include <rspf/vpfutil/vpfprim.h>
#include <rspf/vpfutil/vpfmisc.h>
#include <rspf/vpfutil/vpfdisp.h>
}

#endif
