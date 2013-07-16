#include <iostream>
#include <iomanip>

#include <rspf/base/rspfTieGpt.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfNotifyContext.h>

//*******************************************************************

std::ostream& rspfTieGpt::print(std::ostream& os) const
{
   os << "( ";
   os << dynamic_cast<const rspfGpt&>(*this);
   os << ", ";
   os << tie;
   os << ", ";
   if (rspf::isnan(score) == false)
   {
      os << std::setiosflags(std::ios::fixed) << std::setprecision(15);
      os << score;
   } else {
      os << "nan";
   }
   os << " )";

   return os;
}

std::ostream& rspfTieGpt::printTab(std::ostream& os) const
{
   os << std::setiosflags(std::ios::fixed) << std::setprecision(15);
   os<< lond() ;
   os<<"\t"<< latd();
   os<<"\t"<< height();
   os<<"\t"<< tie.x;
   os<<"\t"<< tie.y;
   os<<"\t"<< score;

   return os;
}

std::ostream& operator<<(std::ostream& os, const rspfTieGpt& pt)
{
   return pt.print(os);
}

std::istream& operator>>(std::istream& is, rspfTieGpt& pt)
{
   //---
   // Expected input format:
   // ( (rspfDPt), (rspfDpt), score )
   // score is real or nan
   //---

   // Start with a nan point.
   pt.makeNan();
   // Check the stream.
   if (!is) return is;

   rspfString tempString;

   // Gobble the "(".
   is >> tempString;

   //get the first point
   is>>dynamic_cast<rspfGpt&>(pt);

   // Eat the ",".
   char c;
   is.get(c);

   //get the second point
   is>>pt.tie;

   // Eat the second ",".
   is.get(c);

   // Get the score
   const int SZ = 64; // real number buffer size
   char tempChars[SZ];
   is.get(tempChars, SZ, ',');
   if (!is) return is;
   tempChars[SZ-1] = '\0';
   tempString = tempChars;
   tempString.trim();
   if (tempString == "nan")
   {
      pt.score = rspf::nan();
   }
   else
   {
      pt.score = tempString.toDouble();
   }

   // Gobble the trailing ")".
   is >> tempString;
   
   // Finished
   return is;
}

//constants for GML 2.1.2
const char* GROUND_GML2      = "ground/gml:Point";
const char* IMAGE_GML2       = "image/gml:Point";
const char* SCORE_GML2       = "score";
const char* COORD_GML2       = "gml:coord";
const char* COORDINATES_GML2 = "gml:coordinates";

rspfRefPtr<rspfXmlNode>
rspfTieGpt::exportAsGmlNode(rspfString aGmlVersion)const
{   
   rspfRefPtr<rspfXmlNode> node(new rspfXmlNode);
   // check datum to be WGS84
   if ( !(datum()->operator==(*(rspfDatumFactory::instance()->wgs84()))) )
   {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING: rspfTieGpt::exportAsGmlNode datum must be WGS84\n";
      return node;
   }
   // check nans in lon/lat and in tie
   if (isLatNan() || isLonNan() || tie.hasNans())
   {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING: rspfTieGpt::exportAsGmlNode positions have nan\n";
      return node;
   }
   if (aGmlVersion[static_cast<std::string::size_type>(0)] == '2')
   {
      node->setTag("SimpleTiePoint");
      // note: no "fid" attribute (feature id")
      //store Ground Point WGS84 + height above ellipsoid in meters
      rspfRefPtr<rspfXmlNode> gcoord =  node->addNode(rspfString(GROUND_GML2) + "/" + COORD_GML2 );
      gcoord->addChildNode("X",rspfString::toString(lond()));
      gcoord->addChildNode("Y",rspfString::toString(latd()));
      if (!isHgtNan())
      {
        gcoord->addChildNode("Z",rspfString::toString(height())); //above ellipsoid
      }

      // store image tie point
      rspfRefPtr<rspfXmlNode> tcoord =  node->addNode(rspfString(IMAGE_GML2) + "/" + COORD_GML2);
      tcoord->addChildNode("X",rspfString::toString(tie.x));
      tcoord->addChildNode("Y",rspfString::toString(tie.y));

      //store score (change name to confidence?)
      node->addNode(SCORE_GML2,rspfString::toString(score));
   } else {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING: rspfTieGpt::exportAsGmlNode Unsupported GML version : " << aGmlVersion <<"\n";
   }
   return node;
}

bool
rspfTieGpt::importFromGmlNode(rspfRefPtr<rspfXmlNode> aGmlNode, rspfString aGmlVersion)
{
   //assuming datum is EPSG:4326 (aka WGS84)
   //feature has to be a SimpleTiePoint feature
   //TBD : add support for coord instead of coordinates
   //TBD : more robust type checks (for X,Y,Z and score) - create extra protected function

   //clear data
   makeNan();
   tie.makeNan();
   score = 0;

   if (aGmlVersion[static_cast<std::string::size_type>(0)] == '2')
   {
      //read ground point
      rspfRefPtr<rspfXmlNode> gn = aGmlNode->findFirstNode(GROUND_GML2);
      rspfRefPtr<rspfXmlNode> gcoord = gn->findFirstNode(COORD_GML2);
      if (gcoord.valid())
      {
         //read coord
         rspfRefPtr<rspfXmlNode> gx = gcoord->findFirstNode("X");
         if (gx.valid())
         {
            lond(rspfString(gx->getText()).toDouble());
         } else {
            rspfNotify(rspfNotifyLevel_WARN) << "WARNING: rspfTieGpt::importFromGmlNode no ground X found in coord\n";
            return false;
         }
         rspfRefPtr<rspfXmlNode> gy = gcoord->findFirstNode("Y");
         if (gy.valid())
         {
            latd(rspfString(gy->getText()).toDouble());
         } else {
            rspfNotify(rspfNotifyLevel_WARN) << "WARNING: rspfTieGpt::importFromGmlNode no ground Y found in coord\n";
            return false;
         }
         rspfRefPtr<rspfXmlNode> gz = gcoord->findFirstNode("Z");
         if (gz.valid())
         {
            height(rspfString(gz->getText()).toDouble());
         } // no Z value is possible
      }
      else {
         //try to read coordinates
         //TBD
         rspfNotify(rspfNotifyLevel_WARN) << "WARNING: rspfTieGpt::importFromGmlNode gml:coordinates not developped yet for ground\n";
         return false;
      }

      //read image point
      rspfRefPtr<rspfXmlNode> in = aGmlNode->findFirstNode(IMAGE_GML2);
      rspfRefPtr<rspfXmlNode> icoord = in->findFirstNode(COORD_GML2);
      if (icoord.valid())
      {
         //read coord
         rspfRefPtr<rspfXmlNode> ix = icoord->findFirstNode("X");
         if (ix.valid())
         {
            tie.x = rspfString(ix->getText()).toDouble();
         } else {
            rspfNotify(rspfNotifyLevel_WARN) << "WARNING: rspfTieGpt::importFromGmlNode no image X found in coord\n";
            return false;
         }
         rspfRefPtr<rspfXmlNode> iy = icoord->findFirstNode("Y");
         if (iy.valid())
         {
            tie.y = rspfString(iy->getText()).toDouble();
         } else {
            rspfNotify(rspfNotifyLevel_WARN) << "WARNING: rspfTieGpt::importFromGmlNode no image Y found in coord\n";
            return false;
         }
         //don't read Z value (shouldn't be any)
      }
      else {
         //try to read coordinates
         //TBD
         rspfNotify(rspfNotifyLevel_WARN) << "WARNING: rspfTieGpt::importFromGmlNode gml:coordinates not developped yet for image\n";
         return false;
      }

      //read score
      rspfRefPtr<rspfXmlNode> scoren = aGmlNode->findFirstNode(SCORE_GML2);
      if (scoren.valid())
      {
         score = rspfString(scoren->getText()).toDouble();
      } else {
         score = 0.0;
      }
      return true;
   } else {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING: rspfTieGpt::importFromGmlNode Unsupported GML version : " << aGmlVersion <<"\n";
      return false;
   }
}
