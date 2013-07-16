//*************************************************************************************************
//                          RSPF -- Open Source Software Image Map
//
//  LICENSE: See top level LICENSE.txt file.
//   AUTHOR: Oscar Kramer
//
//*************************************************************************************************
//  $Id: rspfSrcRecord.h 2788 2011-06-29 13:20:37Z oscar.kramer $
#ifndef rspfSrcRecord_HEADER
#define rspfSrcRecord_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>
#include <vector>

//*************************************************************************************************
//  CLASS DESCRIPTION: 
//! Class used for parsing the command line *.src files. This is a scheme for providing input
//! file information to am rspf app such as rspf-orthoigen.
//!
//! The following keywords with image prefixes as shown are considered:
//! 
//!   image0.file: <image_filename>  
//!   image0.entry: <image_index_in_multi_image_file> (unsigned integer)
//!   image0.ovr: <full/path/to/overview_file.ovr>
//!   image0.hist: <full/path/to/histogram.his>
//!   image0.hist-op: auto-minmax | std-stretch N  (N=1|2|3)
//!   image0.support: <path_to_support_files>
//!   image0.rgb: R,G,B  (unsigned integers starting with 1)
//!   image0.mask: <filename>
//!   image0.opacity: <double>
//!   image0.replacement_mode: <REPLACE_BAND_IF_TARGET | 
//!                             REPLACE_BAND_IF_PARTIAL_TARGET | 
//!                             REPLACE_ALL_BANDS_IF_ANY_TARGET | 
//!                             REPLACE_ALL_BANDS_IF_PARTIAL_TARGET | 
//!                             REPLACE_ONLY_FULL_TARGETS>
//!   image0.clamp.min: <double>
//!   image0.clamp.max: <double>
//!   image0.clip.min: <double>
//!   image0.clip.max: <double>
//! 
//!   vector0.file: <image_filename>  
//!   vector0.entry: <image_index_in_multi_image_file> (unsigned integer)
//!   vector0.query: <select query>
//!   vector0.line.color: <R, G, B> (255,255,255)
//!   vector0.line.width: <line width> (unsigned integer)
//!   vector0.fill.color: <R, G, B>   (255,0,0)
//!   vector0.fill.opacity: <opacity> (unsigned integer)
//!
//! 
//! The "support" keyword can be specified in lieu of "ovr" and "hist" when the latter reside in
//! the same external directory. This directory can also contain external image geometry files.
//! If a support dir is provided without an ovr or hist specification, then default filenames are
//! derived from the image filename, but with the support dir as the path.
//! 
//! Multiple files can be specified by incrementing the prefix image index.
//! 
//*************************************************************************************************
class RSPF_DLL rspfSrcRecord
{
public:
   struct PixelFlipParams
   {
   public:
      PixelFlipParams() : 
         replacementMode(""), 
         clampMin(rspf::nan()), clampMax(rspf::nan()),
         clipMin(rspf::nan()), clipMax(rspf::nan()) {}

      rspfString replacementMode;
      double      clampMin;
      double      clampMax;
      double      clipMin;
      double      clipMax;
   };
   rspfSrcRecord();
      
   //! Constructs given an in-memory KWL and record index.
   rspfSrcRecord(const rspfKeywordlist& kwl, rspf_uint32 index=0, rspfString prefix_str="image");
   
   //! @brief Initializes record from an in-memory KWL and prefix.
   //!
   //! "prefix.file" e.g. "image0.file" is required.  All other data members will be cleared
   //! if their keword is not present.
   //
   //! @param kwl Keyword list containing one or more records.
   //! @param prefix Like "image0."
   //! @return true on success, false if required keyword is not set.
   bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

   //! Returns TRUE if record is valid.
   bool valid() const { return !m_filename.empty(); }

   const rspfFilename&             getFilename()    const { return m_filename; }
   rspf_int32                      getEntryIndex()  const { return m_entryIndex; }
   const rspfFilename&             getSupportDir()  const { return m_supportDir;}
   const rspfString&               getHistogramOp() const { return m_histogramOp;}
   const std::vector<rspf_uint32>& getBands()       const { return m_bandList; }
   const double&                    getWeight()      const { return m_weight; }  

   const PixelFlipParams&           getPixelFlipParams() const { return m_pixelFlipParams; }
   
   //! See note below on these data members.
   const rspfFilename& getOverviewPath()  const { return m_overviewPath; }
   const rspfFilename& getHistogramPath() const { return m_histogramPath; }
   const rspfFilename& getMaskPath() const { return m_maskPath; }
   
   void setFilename(const rspfFilename& f);          
   void setEntryIndex(rspf_int32 i);                 
   void setOverview(const rspfFilename& f);          
   void setMask(const rspfFilename& f)              { m_maskPath = f; }
   void setHistogram(const rspfFilename& f)         { m_histogramPath = f; }
   void setHistogramOp(const rspfString& s)         { m_histogramOp = s; }
   void setBands(const std::vector<rspf_uint32>& v) { m_bandList = v; }
   void setWeight(const double& weight)              {m_weight = weight; }
   void setRgbDataBool(bool isRgbData)               { m_isRgbData = isRgbData; }

   //! Sets supplementary data files dir. If the OVR and/or hist dirs are undefined, they are also
   //! set to this path.
   void setSupportDir(const rspfFilename& f);

   //! Returns TRUE if the record represents a vector data set:
   bool isVectorData() const { return m_isVectorData; }

   //! Returns TRUE if the record represents an rgb data set:
   bool isRgbData() const { return m_isRgbData; }

   rspfFilename  getRgbFilename     (int band) const { return m_rgbFilenames[band];      }
   rspfFilename  getRgbHistogramPath(int band) const { return m_rgbHistogramPaths[band]; }
   rspfString    getRgbHistogramOp  (int band) const { return m_rgbHistogramOps[band];   }
   rspfFilename  getRgbOverviewPath (int band) const { return m_rgbOverviewPaths[band];  }

   //! Returns the KWL containing the desired vector representation properties. In the future we
   //! should stuff many of the members in rspfSrcRecord in a KWL (similar to what is currently
   //! done with vector properties) so that the handler is initialized via loadState() instead of 
   //! individual calls to set methods. OLK 10/10
   const rspfKeywordlist& getAttributesKwl() const { return m_attributesKwl; }

private:
   rspfFilename m_filename;
   rspf_int32   m_entryIndex;
   rspfFilename m_supportDir;
   std::vector<rspf_uint32> m_bandList;
   rspfString   m_histogramOp;
   double        m_weight;

   //! The following data members are usually just a copy of m_supportDir, but are provided in
   //! order to support legacy systems where paths to OVR, thumbnails and histogram files
   rspfFilename m_overviewPath;
   rspfFilename m_histogramPath;
   rspfFilename m_maskPath;

   //! The following data members allow users to render vector data
   bool m_isVectorData;
   rspfKeywordlist m_attributesKwl;

   std::vector<rspfString> m_rgbFilenames;
   std::vector<rspfString> m_rgbHistogramPaths;
   std::vector<rspfString> m_rgbHistogramOps;
   std::vector<rspfString> m_rgbOverviewPaths;
   bool m_isRgbData;
   PixelFlipParams m_pixelFlipParams;
};

#endif
