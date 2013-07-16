//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationMultiPolyLineObject.cpp 17206 2010-04-25 23:20:40Z dburken $

#include <sstream>

#include <rspf/imaging/rspfGeoAnnotationMultiPolyLineObject.h>
#include <rspf/imaging/rspfAnnotationMultiPolyLineObject.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfImageProjectionModel.h>
#include <rspf/base/rspfException.h>


RTTI_DEF1(rspfGeoAnnotationMultiPolyLineObject,
          "rspfGeoAnnotationMultiPolyLineObject",
          rspfGeoAnnotationObject);

rspfGeoAnnotationMultiPolyLineObject::rspfGeoAnnotationMultiPolyLineObject()
   :
   rspfGeoAnnotationObject(),
   theMultiPolyLine(),
   theBoundingRect(),
   theDatum(rspfDatumFactory::instance()->wgs84()),
   theProjectedPolyLineObject(0)
{
   allocateProjectedPolyLine();   
   theBoundingRect.makeNan();
}

rspfGeoAnnotationMultiPolyLineObject::rspfGeoAnnotationMultiPolyLineObject(
   const vector<rspfPolyLine>& multiPoly,
   rspf_uint8 r,
   rspf_uint8 g,
   rspf_uint8 b,
   rspf_uint8 thickness)
   :
   rspfGeoAnnotationObject(r, g, b, thickness),
   theMultiPolyLine(multiPoly),
   theBoundingRect(),
   theDatum(rspfDatumFactory::instance()->wgs84()),
   theProjectedPolyLineObject(0)
{
   allocateProjectedPolyLine();   
   theBoundingRect.makeNan();
}

rspfGeoAnnotationMultiPolyLineObject::rspfGeoAnnotationMultiPolyLineObject(
   const rspfGeoAnnotationMultiPolyLineObject& rhs)
   :rspfGeoAnnotationObject(rhs),
    theMultiPolyLine(rhs.theMultiPolyLine),
    theBoundingRect(rhs.theBoundingRect),
    theDatum(rhs.theDatum),
    theProjectedPolyLineObject(rhs.theProjectedPolyLineObject.valid()?(rspfAnnotationMultiPolyLineObject*)rhs.theProjectedPolyLineObject->dup():0)
{
}

rspfObject* rspfGeoAnnotationMultiPolyLineObject::dup()const
{
   return new rspfGeoAnnotationMultiPolyLineObject(*this);
}

rspfGeoAnnotationMultiPolyLineObject::~rspfGeoAnnotationMultiPolyLineObject()
{
   theProjectedPolyLineObject = 0;
}

void rspfGeoAnnotationMultiPolyLineObject::applyScale(double /* x */, double /* y */)
{
   rspfNotify(rspfNotifyLevel_NOTICE)
      << "rspfGeoAnnotationPolyLineObject::applyScale NOT IMPLEMENTED!!!!"
      << endl;
}

void rspfGeoAnnotationMultiPolyLineObject::transform(rspfImageGeometry* projection)
{
   if(!projection)
   {
      return;
   }
   
   allocateProjectedPolyLine();

   //---
   // NOTE:
   // allocateProjectedPolygon() will set theProjectedPolyLineObject to 0 if
   // theMultiPolyLine is empty (theMultiPolyLine.size() == 0).  So check
   // before accessing pointer to avoid a core dump.
   //---
   if(!theProjectedPolyLineObject)
   {
      return;
   }

   std::vector<rspfPolyLine>& multiPolyLine =
      theProjectedPolyLineObject->getMultiPolyLine();
   rspfGpt tempPoint(0,0, rspf::nan(), theDatum);
   
   for(std::vector<rspfPolyLine>::size_type polyI = 0;
       polyI < theMultiPolyLine.size();
       ++polyI)
   {
      rspfPolyLine polyLine;
      
      rspf_uint32 numberOfVertices =
         theMultiPolyLine[polyI].getNumberOfVertices();
      for(rspf_uint32 pointI = 0; pointI < numberOfVertices; ++pointI)
      {
         tempPoint.latd(theMultiPolyLine[polyI][pointI].lat);
         tempPoint.lond(theMultiPolyLine[polyI][pointI].lon);
         rspfDpt temp;
         projection->worldToLocal(tempPoint, temp);
         if(!temp.hasNans())
         {
	    multiPolyLine[polyI].addPoint(temp);
         }
      }
      multiPolyLine[polyI].roundToIntegerBounds(true);
   }
}

std::ostream& rspfGeoAnnotationMultiPolyLineObject::print(std::ostream& out)const
{
   rspfNotify(rspfNotifyLevel_INFO)
      << "rspfGeoAnnotationMultiPolyLineObject::print\n"
      << "Poly line size: " << theMultiPolyLine.size() << endl;
   return out;
}

bool rspfGeoAnnotationMultiPolyLineObject::intersects(const rspfDrect& /* rect */)const
{
   rspfNotify(rspfNotifyLevel_NOTICE)
      << "rspfGeoAnnotationMultiPolyObject::intersects NOT IMPLEMENTED"
      << endl;
  return false;
}

rspfAnnotationObject* rspfGeoAnnotationMultiPolyLineObject::getNewClippedObject(
   const rspfDrect& /* rect */)const
{
   rspfNotify(rspfNotifyLevel_NOTICE)
      << "rspfGeoAnnotationMultiPolyObject::getNewClippedObject "
      << "NOT IMPLEMENTED" << endl;

   return (rspfAnnotationObject*)(dup());
}

void rspfGeoAnnotationMultiPolyLineObject::draw(rspfRgbImage& anImage)const
{
   if(theProjectedPolyLineObject.valid())
   {
      theProjectedPolyLineObject->draw(anImage);
   }
}

void rspfGeoAnnotationMultiPolyLineObject::getBoundingRect(rspfDrect& rect)const
{
   rect = theBoundingRect;
}

void rspfGeoAnnotationMultiPolyLineObject::addPoint(rspf_uint32 polygonIndex,
                                                     const rspfGpt& pt)
{
   if(polygonIndex < theMultiPolyLine.size())
   {
      theMultiPolyLine[polygonIndex].addPoint(pt);
      
      // we will have to reset the projected polygon
      theProjectedPolyLineObject = 0;
   }
}

void rspfGeoAnnotationMultiPolyLineObject::setMultiPolyLine(
   const vector<rspfPolyLine>& multiPoly)
{
   theMultiPolyLine = multiPoly;
   theProjectedPolyLineObject = 0;
}

void rspfGeoAnnotationMultiPolyLineObject::computeBoundingRect()
{
   theBoundingRect.makeNan();
   if(theProjectedPolyLineObject.valid())
   {
      theProjectedPolyLineObject->computeBoundingRect();
      theProjectedPolyLineObject->getBoundingRect(theBoundingRect);
   }
   
}

bool rspfGeoAnnotationMultiPolyLineObject::isPointWithin(const rspfDpt& imagePoint)const
{
   if(theProjectedPolyLineObject.valid())
   {
      return theProjectedPolyLineObject->isPointWithin(imagePoint);
   }
   
   return false;
}

void rspfGeoAnnotationMultiPolyLineObject::setColor(rspf_uint8 r,
                                                     rspf_uint8 g,
                                                     rspf_uint8 b)
{
   rspfAnnotationObject::setColor(r, g, b);
   if(theProjectedPolyLineObject.valid())
   {
      theProjectedPolyLineObject->setColor(r, g, b);
   }
}

void rspfGeoAnnotationMultiPolyLineObject::setThickness(rspf_uint8 thickness)
{
   rspfAnnotationObject::setThickness(thickness);
   if(theProjectedPolyLineObject.valid())
   {
      theProjectedPolyLineObject->setThickness(thickness);
   }
}
   
void rspfGeoAnnotationMultiPolyLineObject::setDatum(const rspfDatum* datum)
{
   theDatum = datum;
}

const rspfDatum* rspfGeoAnnotationMultiPolyLineObject::getDatum()const
{
   return theDatum;
}

const std::vector<rspfPolyLine>&
rspfGeoAnnotationMultiPolyLineObject::getMultiPolyLine() const
{
   return theMultiPolyLine;
}

std::vector<rspfPolyLine>&
rspfGeoAnnotationMultiPolyLineObject::getMultiPolyLine()
{
   return theMultiPolyLine;
}

bool rspfGeoAnnotationMultiPolyLineObject::saveState(rspfKeywordlist& kwl,
                                                      const char* prefix) const
{
   rspf_uint32 vIdx = 0;
   
   for(rspf_uint32 i = 0; i < theMultiPolyLine.size(); ++i)
   {
      rspfPolyLine pl = theMultiPolyLine[i];
      for(rspf_uint32 j = 0; j < pl.getNumberOfVertices(); ++j)
      {
         rspfDpt dpt = pl[j];
         std::ostringstream os;
         os << setprecision(15) << dpt.x << " " << dpt.y << endl;
         
         rspfString key = "v";
         key += rspfString::toString(vIdx);
         kwl.add(prefix, key.c_str(), os.str().c_str());
         ++vIdx;
      }
   }      
      
   return rspfGeoAnnotationObject::saveState(kwl, prefix);
}

bool rspfGeoAnnotationMultiPolyLineObject::loadState(
   const rspfKeywordlist& kwl, const char* prefix)
{
   //---
   // Base class state must be called first to pick up colors...
   //---
   bool status =  rspfGeoAnnotationObject::loadState(kwl, prefix);

   rspf_uint32 index = 0;
   rspfString copyPrefix = prefix;
   rspf_uint32 count = kwl.getNumberOfSubstringKeys(copyPrefix+
                                                      "v[0-9]");
   
   rspf_uint32 numberOfMatches = 0;
   const rspf_uint32 MAX_INDEX = count + 100;
   rspfPolyLine pl;
   
   while(numberOfMatches < count)
   {
      rspfString key = "v";
      key += rspfString::toString(index);

      const char* lookup = kwl.find(prefix, key.c_str());
      if (lookup)
      {
         ++numberOfMatches;
         rspfDpt dpt;
         rspfString x,y;
         std::istringstream is(lookup);
         is >> x >> y;
         pl.addPoint(rspfDpt(x.toDouble(), y.toDouble()));
      }

      if (pl.size() == 2)
      {
         theMultiPolyLine.push_back(pl);
         pl.clear();
      }

      ++index;
      
      if (index > MAX_INDEX) // Avoid infinite loop...
      {
         break;
      }
   }

   return status;
}

void rspfGeoAnnotationMultiPolyLineObject::allocateProjectedPolyLine()
{
   theProjectedPolyLineObject = 0;
   
   if(theMultiPolyLine.size())
   {
      vector<rspfPolyLine> polyList(theMultiPolyLine.size());
      theProjectedPolyLineObject =
         new rspfAnnotationMultiPolyLineObject(polyList,
                                                theRed,
                                                theGreen,
                                                theBlue,
                                                theThickness);
   }
}
