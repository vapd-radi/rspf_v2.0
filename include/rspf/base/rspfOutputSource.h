//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts
//
// Description:
//
// Contains class declaration for rspfImageWriter
//*******************************************************************
//  $Id: rspfOutputSource.h 9968 2006-11-29 14:01:53Z gpotts $
//
#ifndef rspfOutputSource_HEADER
#define rspfOutputSource_HEADER
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfSource.h>
#include <rspf/base/rspfCommon.h>

class RSPFDLLEXPORT rspfOutputSource : public rspfSource
{
public:
   rspfOutputSource(rspfObject* owner=NULL)
      :rspfSource(owner)
      {}
   rspfOutputSource(rspfObject* owner,
                     rspf_uint32 inputListSize,
                     rspf_uint32 outputListSize,
                     bool inputListIsFixedFlag=true,
                     bool outputListIsFixedFlag=true)
      :rspfSource(owner,
                   inputListSize,
                   outputListSize,
                   inputListIsFixedFlag,
                  outputListIsFixedFlag)
      {
      }

   virtual ~rspfOutputSource(){}
   
   virtual bool isOpen()const = 0;
   
   /*!
    * Allows a generic interface to open a consumer for writing.  If
    * you are a display writer then you would probably show the window or
    * init the device context for writing.
    */
   virtual bool open()=0;

   /*!
    * Will set the output name and open the output device.
    */
   virtual bool open(const rspfString& outputName)
      {
         setOutputName(outputName);
         return open();
      }
   
   /*!
    * This is a generic interface to allow one to set the output name of
    * the writer.  If its a Visual display this could be used to
    * set the title.  If this is a file output then this could be a
    * directory or a file name.
    */
  virtual void setOutputName(const rspfString& outputName)
  {
    theOutputName = outputName;
  }

  virtual const rspfString& getOutputName()
  {
    return theOutputName;
  }
   /*!
    * Generic interface to allow one to close a consumer.  For displays
    * this might minimize the window and free up internal pointers for
    * file writers it will make sure the file stream is closed and flushed.
    */
   virtual void close()=0;
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);


protected:
  rspfString theOutputName;


TYPE_DATA
};

#endif
