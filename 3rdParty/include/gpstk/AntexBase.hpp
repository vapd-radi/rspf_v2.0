#pragma ident "$Id: AntexBase.hpp 2293 2010-02-12 18:14:16Z btolman $"

/**
 * @file AntexBase.hpp
 * Base class for Antex file data
 */

#ifndef GPSTK_ANTEX_BASE_HPP
#define GPSTK_ANTEX_BASE_HPP

//============================================================================
//
//  This file is part of GPSTk, the GPS Toolkit.
//
//  The GPSTk is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 2.1 of the License, or
//  any later version.
//
//  The GPSTk is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with GPSTk; if not, write to the Free Software Foundation,
//  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  
//  Copyright 2004, The University of Texas at Austin
//
//============================================================================

//============================================================================
//
//This software developed by Applied Research Laboratories at the University of
//Texas at Austin, under contract to an agency or agencies within the U.S. 
//Department of Defense. The U.S. Government retains all rights to use,
//duplicate, distribute, disclose, or release this software. 
//
//Pursuant to DoD Directive 523024 
//
// DISTRIBUTION STATEMENT A: This software has been approved for public 
//                           release, distribution is unlimited.
//
//=============================================================================

#include "FFData.hpp"

namespace gpstk
{
   /** @defgroup Antex files */
   //@{

  /** This level isn't serving any purpose other than to make
   * the class diagram look nice...
   */
   class AntexBase : public FFData
   {
   public:
         /// Destructor per the coding standards
      virtual ~AntexBase() {}
   };

   //@}

}  // namespace

#endif
