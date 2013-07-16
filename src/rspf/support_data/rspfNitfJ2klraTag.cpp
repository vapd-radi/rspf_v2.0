//----------------------------------------------------------------------------
//
// File: rspfNitfJ2klraTag.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: NITF J2KLRA tag.
//
// See:  ISO/IEC BIIF Profile BPJ2K01.00 Table 9-3.
// 
//----------------------------------------------------------------------------
// $Id$


#include <rspf/support_data/rspfNitfJ2klraTag.h>
#include <rspf/base/rspfString.h>

#include <cstring> /* for memcpy */
#include <iomanip>
#include <istream>
#include <ostream>

RTTI_DEF1(rspfNitfJ2klraTag, "rspfNitfJ2klraTag", rspfNitfRegisteredTag);

rspfNitfJ2klraTag::rspfNitfJ2klraTag()
   : rspfNitfRegisteredTag(std::string("J2KLRA"), 0),
     m_layer(1)
{
   clearFields();
}

void rspfNitfJ2klraTag::parseStream(std::istream& in)
{
   clearFields();
   
   in.read(m_orig, ORIG_SIZE);
   in.read(m_levels_o, NLEVELS_O_SIZE);
   in.read(m_bands_o, NBANDS_O_SIZE);
   in.read(m_layers_o, NLAYERS_O_SIZE);

   rspf_uint32 layers = getNumberOfLayersOriginal();
   if ( layers && (layers < 1000) ) // 999 max
   {
      m_layer.resize(layers);
      for ( rspf_uint32 i = 0; i < layers; ++i )
      {
         in.read(m_layer[i].m_layer_id, LAYER_ID_SIZE);
         in.read(m_layer[i].m_bitrate, BITRATE_SIZE);
         m_layer[i].m_layer_id[LAYER_ID_SIZE] = '\0';
         m_layer[i].m_bitrate[BITRATE_SIZE]   = '\0';
      }
   }
   else
   {
      m_layer.clear();
   }

   // Conditional:
   rspf_uint32 orig = getOriginNumber();
   if ( (orig == 1) || ( orig == 3 ) || ( orig == 9 ) )
   {
      in.read(m_nlevels_i, NLEVELS_I_SIZE);
      in.read(m_nbands_i, NBANDS_I_SIZE);
      in.read(m_nlayers_i, NLAYERS_I_SIZE);
   }

   // Set the base tag length.
   setTagLength( getSizeInBytes() );
}

void rspfNitfJ2klraTag::writeStream(std::ostream& out)
{
   out.write(m_orig, ORIG_SIZE);
   out.write(m_levels_o, NLEVELS_O_SIZE);
   out.write(m_bands_o, NBANDS_O_SIZE);
   out.write(m_layers_o, NLAYERS_O_SIZE);
   
   rspf_uint32 size = m_layer.size();
   for ( rspf_uint32 i = 0; i < size; ++i )
   {
      out.write(m_layer[i].m_layer_id, LAYER_ID_SIZE);
      out.write(m_layer[i].m_bitrate, BITRATE_SIZE);
   }

   // Conditional:
   rspf_uint32 orig = getOriginNumber();
   if ( (orig == 1) || ( orig == 3 ) || ( orig == 9 ) )
   {
      out.write(m_nlevels_i, NLEVELS_I_SIZE);
      out.write(m_nbands_i, NBANDS_I_SIZE);
      out.write(m_nlayers_i, NLAYERS_I_SIZE);
   }
}

rspf_uint32 rspfNitfJ2klraTag::getSizeInBytes()const
{
   rspf_uint32 result = 11 + (m_layer.size() * 12);

   // Conditional:
   rspf_uint32 orig = getOriginNumber();
   if ( (orig == 1) || ( orig == 3 ) || ( orig == 9 ) )
   {
      result += 10;
   }
   return result;
}

void rspfNitfJ2klraTag::clearFields()
{
   memset(m_orig, '0', ORIG_SIZE);

   memset(m_levels_o, '0', NLEVELS_O_SIZE);
   memset(m_bands_o, '0', NBANDS_O_SIZE);
   memset(m_layers_o, '0', NLAYERS_O_SIZE);

   m_layer.clear();

   memset(m_nlevels_i, '0', NLEVELS_I_SIZE);
   memset(m_nbands_i, '0', NBANDS_I_SIZE);
   memset(m_nlayers_i, '0', NLAYERS_I_SIZE);
   
   m_orig[ORIG_SIZE]= '\0';
   
   m_levels_o[NLEVELS_O_SIZE]= '\0';
   m_bands_o[NBANDS_O_SIZE]= '\0';
   m_layers_o[NLAYERS_O_SIZE]= '\0';

   m_nlevels_i[NLEVELS_I_SIZE]= '\0';
   m_nbands_i[NBANDS_I_SIZE] = '\0';
   m_nlayers_i[NLAYERS_I_SIZE] = '\0';

   // Set the base tag length.
   setTagLength( 0 );
}

std::ostream& rspfNitfJ2klraTag::print(std::ostream& out,
                                        const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";
   
   out << setiosflags(ios::left)
       << pfx << std::setw(24) << "CETAG:"     << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"       << getTagLength() << "\n"
       << pfx << std::setw(24) << "ORIG:"      << m_orig << "\n"
       << pfx << std::setw(24) << "NLEVELS_O:" << m_levels_o << "\n"
       << pfx << std::setw(24) << "NBANDS_O:"  << m_bands_o << "\n"
       << pfx << std::setw(24) << "NLAYERS_O:" << m_layers_o << "\n";

   rspf_uint32 size = m_layer.size();
   for ( rspf_uint32 i = 0; i < size; ++i )
   {
      out << pfx << "LAYER_ID[" << i << std::setw(14) << "]:" << m_layer[i].m_layer_id << "\n"
          << pfx << "BITRATE[" << i << std::setw(15) << "]:" << m_layer[i].m_bitrate << "\n";
   }

   // Conditional:
   rspf_uint32 orig = getOriginNumber();
   if ( (orig == 1) || ( orig == 3 ) || ( orig == 9 ) )
   {
      out << pfx << std::setw(24) << "NLEVELS_I:"     << m_nlevels_i << "\n"
          << pfx << std::setw(24) << "NBANDS_I_SIZE:" << m_nbands_i << "\n"
          << pfx << std::setw(24) << "NLAYERS_I:"     << m_nlayers_i << "\n";
   }
   return out;
}
   
rspf_uint32 rspfNitfJ2klraTag::getOriginNumber() const
{
   return rspfString(m_orig).toUInt32();
}
   
rspf_uint32 rspfNitfJ2klraTag::getNumberOfLayersOriginal() const
{
   return rspfString(m_layers_o).toUInt32();
}
