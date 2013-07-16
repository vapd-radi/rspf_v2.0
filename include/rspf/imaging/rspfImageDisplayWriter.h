//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Frank Warmerdam (warmerdam@pobox.com)
//
// Description:
//
// Contains class declaration for rspfImageWriter
//*******************************************************************
//  $Id: rspfImageDisplayWriter.h 17195 2010-04-23 17:32:18Z dburken $
#ifndef rspfImageDisplayWriter_HEADER
#define rspfImageDisplayWriter_HEADER

#include <rspf/imaging/rspfImageWriter.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfConnectableObjectListener.h>
#include <rspf/base/rspfDisplayInterface.h>
#include <rspf/base/rspfObjectEvents.h>
#include <rspf/base/rspfProcessProgressEvent.h>

class rspfImageDisplayWriter :  public rspfImageWriter,
                                 public rspfDisplayInterface,
                                 public rspfConnectableObjectListener

{
public:
   
   bool canConnectMyInputTo(rspf_int32 /* inputIndex */,
                            const rspfConnectableObject* object)const
      {
         
         return (object&& PTR_CAST(rspfImageSource, object));
      }
   virtual void setOutputName(const rspfString& out)
      {
         setTitle(out);
      }

   /*!
    * Sets the title of a display.
    */
   virtual void setTitle( const rspfString& title )=0;

   /*!
    * will return true if its minimized
    */
   virtual bool isMinimized()const=0;

   virtual bool isMaximized()const=0;
   /*!
    * Will repaint the display
    */
   virtual void refreshDisplay()=0;

   
   virtual void minimize()=0;

   virtual void maximize()=0;
   /*!
    * Will show a display
    */
   virtual void show()=0;
   
protected:
   virtual ~rspfImageDisplayWriter(){}
   
   rspfImageDisplayWriter(rspfObject* owner=NULL);
   rspfImageDisplayWriter(rspfObject* owner,
                           rspf_uint32 numberOfInputs,
                           rspf_uint32 numberOfOutputs,
                           bool inputListIsFixed,
                           bool outputListIsFixed);
TYPE_DATA
};
#endif
