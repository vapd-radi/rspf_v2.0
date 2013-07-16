/*
 * Confidential Intellectual Property of Pixia Corp.
 * Copyright (C) 2004 Pixia Corp. All rights reserved.
 *
 * Tags beginning with a backslash are for automatic documentation generation.
 */
/**
 * \file nui_j2k2.h
 * \section NUI_J2K2
 * \brief The NUI_J2K2 library provides JPEG2000 codec support.
 *
 * The NUI_J2K2 library provides JPEG2000 compression, decompression and
 * informational functionality. In order to perform encoding and decoding
 * operations, the user creates encoder and decoder structures
 * (NUI_J2K2_ENCODE and NUI_J2K2_DECODE). These structures are then passed to a
 * variety of functions that read or write properties within the structure or
 * perform codec operations upon RAM-resident images and JPEG2000 code
 * streams.
 *
 * \note The NUI_J2K2 library encodes and decodes JPEG2000 code streams
 * (a.k.a. .j2c files). The NUI_J2K library does not use the more complex
 * .jpx file structure.
 *
 * \authors Alan Black
 *
 * $Header: /opt/cvs/cvsroot/pixia_dev/src/lib.src/nui_core/nui_j2k2/nui_j2k2.h,v 2.0 2006/11/16 21:13:05 jensenk Exp $
 * $Log: nui_j2k2.h,v $
 * Revision 2.0  2006/11/16 21:13:05  jensenk
 * Updating all revs to 2.0
 *
 * Revision 1.1  2006/11/15 17:15:13  jensenk
 * Creating pixia_dev code structure in new CVS server.  Note the initial
 * file rev of 1.1 will be bumped to 2.0 when restructuring is complete.
 *
 * Revision 1.5  2005/08/03 21:10:00  thakkarr
 * Added section for nuiSDK docs
 *
 * Revision 1.4  2005/08/03 19:50:45  thakkarr
 * Exported all functons
 *
 * Revision 1.3  2005/03/16 16:18:34  soods
 * some coding standard improvements
 *
 * Revision 1.2  2005/03/15 21:35:47  soods
 * Misc Small Improvements (J2C codestream explicit check, coding standards,
 * ADV202 Disabled, ADV202 SDK enable/disabled streamlined)
 *
 * Revision 1.1  2005/01/31 12:02:24  blacka
 * Added to CVS.
 *
 */

#include <nui_constants.h>
#include "nui_j2k2_core.h"

/*
 * nui_j2k2.h ends
 */

