//----------------------------------------------------------------------------
// Copyright (c) 2005, David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:  Class to write out meta data in a Federal Geographic Data
// Committe (FGDC) format.
//
//----------------------------------------------------------------------------
// $Id: rspfFgdcFileWriter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfFgdcFileWriter_HEADER
#define rspfFgdcFileWriter_HEADER

#include <rspf/imaging/rspfMetadataFileWriter.h>
#include <rspf/base/rspfXmlNode.h>
#include <rspf/base/rspfXmlDocument.h>

/**
 * Class to write out meta data in a Federal Geographic Data
 * Committe (FGDC) format.
 */
class RSPFDLLEXPORT rspfFgdcFileWriter : public rspfMetadataFileWriter
{
public:
   
   /** constructor */
   rspfFgdcFileWriter();

   
   /**
    * Method to load the state from a keyword list.
    *
    * @param kwl Keyword list to load state from.
    *
    * @param prefix Prefix to attatch to keyword (like "object1.").
    * default=NULL
    *
    * @return true if ok or false on error.
    *
    * Keywords picked up by this loadState:
    * 
    * source_image_filename: foo.ntf
    * tab_string:            "\t"
    * fgdc_template_doc:     fgdc_template.xml
    * 
    */
   virtual bool loadState(const rspfKeywordlist &kwl, const char *prefix=0);

   /**
    * Satisfies pure virtual from rspfMetadataWriter base.
    *
    * Appends the writers image types to the "metadatatypeList".
    * 
    * @param metadatatypeList stl::vector<rspfString> list to append to.
    *
    * @note Appends to the list, does not clear it first.
    */
   virtual void getMetadatatypeList(
      std::vector<rspfString>& metadatatypeList) const;

   /**
    * Satisfies pure virtual from rspfMetadataWriter base.
    *
    * @param imageType string representing image type.
    *
    * @return true if "metadataType" is supported by writer.
    */
   virtual bool hasMetadataType(const rspfString& metadataType)const;

   
   /** @return Returns the current indentation level. */
   rspf_uint32 getIndentionLevel() const;

   /**
    * Sets the current indentation level.
    *
    * @param level Indentation level.  "0" equals no indentation.
    */
   void setIndentionLevel(rspf_uint32 level);

   /**
    * @return The current tab string e.g.: "\t".
    */
   rspfString getTabString() const;

   /**
    * Sets the tab string.  Usually "\t" or spaces "  ".
    *
    * @param tabString  String to use for indentation.
    */
   void setTabString(const rspfString& tabString);

   /**
    * @param title Tile of data usually the image file name.
    */
   void setTitle(const rspfString& pubDate);

   void setTemplate(const rspfFilename& xmlTemplateFile);

   rspfString find(const rspfString& xpath) const;

   bool writeTemplate(const rspfFilename& file) const;

protected:
   /** destructor */
   virtual ~rspfFgdcFileWriter();

   /**
    * Writes file to disk.
    *
    * @return true on success, false on failure.
    */
   virtual bool writeFile();

private:

   /**
    * Writes openning tag adding any indentation and brackets.  Increments
    * indentation level.
    *
    * @param os otream to write to.
    *
    * @param tag String representing tag with no brackets, i.e. "metadata".
    *
    * @param newLine If true a new line "\n" is added.
    */
   void openTag(std::ostream& os,
                const rspfString& tag,
                bool newLine) const;

   /**
    * Closes tag decrementing indentation.
    * e.g.: <metadata>
    *
    * @param os otream to write to.
    *
    * @param tag String representing tag with no brackets.
    *
    * @param decrementLevel If true indentation level is decremented.
    */
   void closeTag(std::ostream& os,
                 const rspfString& tag,
                 bool decrementLevel) const;
   
   mutable rspf_uint32 theIndentionLevel;
   rspfString          theTabString;
   rspfXmlDocument     theTemplate;
   rspfFilename        theSourceImageFilename;

TYPE_DATA
};

#endif /* End of "#ifndef rspfFgdcFileWriter_HEADER" */
