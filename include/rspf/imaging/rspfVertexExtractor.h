//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: David Burken (dburken@imagelinks.com)
//
//*************************************************************************
// $Id: rspfVertexExtractor.h 17207 2010-04-25 23:21:14Z dburken $

#ifndef rspfVertexExtractor_HEADER
#define rspfVertexExtractor_HEADER

#include <fstream>

#include <rspf/base/rspfOutputSource.h>
#include <rspf/base/rspfProcessInterface.h>
#include <rspf/base/rspfProcessProgressEvent.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/imaging/rspfImageSource.h>

class rspfImageSource;


//! Class rspfVertexExtractor

/*!
 *  Class designed to scan the area of interest and detect the valid vertices
 *  of non null image data.
 */
class RSPFDLLEXPORT rspfVertexExtractor : public rspfOutputSource,
    public rspfProcessInterface
{
public:
   rspfVertexExtractor(rspfImageSource* inputSource=NULL);


  virtual rspfObject* getObject()
  {
    return this;
  }
  virtual const rspfObject* getObject()const
  {
    return this;
  }
   /*!
    *  Sets the area of interest in the source to extract the vertices from.
    *  @param rect rspfIrec representing source area of interest.
    */
   void setAreaOfInterest(const rspfIrect& rect);

   /*!
    *  Sets the name of the output keyword list file which the vertices will
    *  be dumped to.
    *  @param filename rspfFilename representing the output file.
    *
    *  Note:
    *  - If "theFileStream" is open it will be closed.
    */
   virtual void setOutputName(const rspfString& filename);

   /*!
    *  Returns true if "theFileStream" is open, false if not.
    */
   virtual bool isOpen() const;

   /*!
    * Opens "theFilename" for output.
    * Returns true on success, false on error.
    * Notes:
    * - If the file was previously open then it will close it and re-opens the
    *   file.
    * - Returns false if "theFilename" has not been set.
    */
   virtual bool open();

   /*!
    *  Closes "theFileStream".
    */
   virtual void close();

   /*!
    *  Calls protected methods scanForEdges, extractVertices, and
    *  writeVertices.  Will error out if "theFilename" or "theAreaOfInterest
    *  has not been set or "theFilename" could not be opened.
    */
   virtual bool execute();

   virtual rspfObject* getObjectInterface() { return this; }

   virtual rspfListenerManager* getListenerManagerInterface()
      {
         return this;
      }
   
   
   virtual void setPercentComplete(double percentComplete)
      {
         rspfProcessInterface::setPercentComplete(percentComplete);
         rspfProcessProgressEvent event(this,
                                         percentComplete);
         fireEvent(event);      
      }

   bool canConnectMyInputTo(rspf_int32 /* inputIndex */,
                            const rspfConnectableObject* object)const
      {
         
         return (object&& PTR_CAST(rspfImageSource, object));
      }

   vector<rspfIpt> getVertices() { return theVertice; }
   
protected:
   virtual ~rspfVertexExtractor();
   /*!
    *  Walks each line from left and right side detecting first non null pixel.
    *  Returns true on success, false on error.
    */
   bool scanForEdges();

   /*!
    *  Extracts the vertices of the source.  Uses "theLeftEdge" and
    *  "theRightEdge" data members.
    *  Returns true on success, false on error.
    */
   bool extractVertices();

   template<class T1>
   void getMinAndIndex(T1* start, T1* end,
                       T1& minValue, rspf_int32& offsetFromStart);
   template<class T2>
   void getMaxAndIndex(T2* start, T2* end,
                       T2& maxValue, rspf_int32& offsetFromStart);
   
   /*!
    *  Writes the result to the output file (theFilename).
    *  Returns true on success, false on error.
    */

   
   bool writeVertices();

   rspfIrect       theAreaOfInterest;
   rspfFilename    theFilename;
   std::ofstream    theFileStream;
   vector<rspfIpt> theVertice;
   rspf_int32*     theLeftEdge;
   rspf_int32*     theRightEdge;

   //! Disallow copy constructor and operator=
   rspfVertexExtractor(const rspfVertexExtractor&) {}
   const rspfVertexExtractor& operator=(const rspfVertexExtractor& rhs)
      {return rhs;}

TYPE_DATA
};

#endif
