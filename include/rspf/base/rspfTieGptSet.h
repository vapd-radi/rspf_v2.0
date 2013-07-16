#ifndef rspfTieGptSet_HEADER
#define rspfTieGptSet_HEADER

#include <iostream>
#include <vector>
#include <rspf/base/rspfXmlNode.h>
#include <rspf/base/rspfTieGpt.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/matrix/newmat.h>

class rspfDpt;

/**
 * storage class for a set of geographic tie points, between master and slave images
 * + GML (OGC) serialization
 *
 * NOTES
 * master points are stored on the ground
 * ground SRS is EPSG:4326 only (WGS84) + height in meters above ellispoid
 *
 * slave points are stored as image positions
 *
 * general ground/image accuracy values are stored
 *
 * TODO :
 */
class RSPFDLLEXPORT rspfTieGptSet
{
public:

   inline rspfTieGptSet() {}

   inline rspfTieGptSet(const rspfTieGptSet& aSet)
      :
      theTies(aSet.getTiePoints()),
      theMasterPath(aSet.getMasterPath()),
      theSlavePath(aSet.getSlavePath()),
      theImageCov(aSet.getImageCov()),
      theGroundCov(aSet.getGroundCov())
   {}

   inline ~rspfTieGptSet() {}

   const rspfTieGptSet& operator=(const rspfTieGptSet&);

   // accessors   
   inline void  setTiePoints(const vector<rspfRefPtr<rspfTieGpt> >& aTieSet) { theTies = aTieSet; }
   inline const vector<rspfRefPtr<rspfTieGpt> >& getTiePoints()const         { return theTies; }
   inline       vector<rspfRefPtr<rspfTieGpt> >& refTiePoints()              { return theTies; }

   inline void  setMasterPath(const rspfString& aPath) { theMasterPath = aPath; }
   inline const rspfString& getMasterPath()const       { return theMasterPath; }

   inline void  setSlavePath(const rspfString& aPath) { theSlavePath = aPath; }
   inline const rspfString& getSlavePath()const       { return theSlavePath; }

   inline void  setImageCov(const NEWMAT::SymmetricMatrix& aCovMat) { theImageCov = aCovMat; }
   inline const NEWMAT::SymmetricMatrix& getImageCov()const       { return theImageCov; }
   inline       NEWMAT::SymmetricMatrix& refImageCov()            { return theImageCov; }

   inline void  setGroundCov(const NEWMAT::SymmetricMatrix& aCovMat) { theGroundCov = aCovMat; }
   inline const NEWMAT::SymmetricMatrix& getGroundCov()const       { return theGroundCov; }
   inline       NEWMAT::SymmetricMatrix& refGroundCov()            { return theGroundCov; }
  
   void getSlaveMasterPoints(std::vector<rspfDpt>& imv, std::vector<rspfGpt>& gdv)const;

   inline unsigned int size()const { return (unsigned int)theTies.size(); }

   /**
    * operations
    */
   void addTiePoint(rspfRefPtr<rspfTieGpt> aTiePt);
   void clearTiePoints();

   void getGroundBoundaries(rspfGpt& gBoundInf, rspfGpt& gBoundSup)const;

   /**
    * text output : header + tab separated tie points
    */
   std::ostream& printTab(std::ostream& os) const;

   /**
    * GML features (XML) serialization
    */
   rspfRefPtr<rspfXmlNode> exportAsGmlNode(rspfString aGmlVersion="2.1.2")const;
   bool importFromGmlNode(rspfRefPtr<rspfXmlNode> aGmlNode, rspfString aGmlVersion="2.1.2");
   
  /**
   * Public data members
   */
   static const char* TIEPTSET_TAG;

protected:
   /**
    * Protected data members
    */
   std::vector<rspfRefPtr<rspfTieGpt> > theTies;      //store by reference so derived classes can be used
   rspfString                       theMasterPath; //!full or relative path to master dataset
   rspfString                       theSlavePath; //!full or relative path to slave dataset
   NEWMAT::SymmetricMatrix           theImageCov;  //! image error covariance matrix
   NEWMAT::SymmetricMatrix           theGroundCov; //! ground error covariance matrix

   /**
    * Protected methods
    */
   rspfString symMatrixToText(const NEWMAT::SymmetricMatrix& sym, 
                               const rspfString& el_sep=" ",
                               const rspfString& row_sep=";")const;

   NEWMAT::SymmetricMatrix textToSymMatrix(const rspfString& text,
                               unsigned int dim,
                               const rspfString& seps=" ;\t\r\n")const; //list of possible elements or row separators

};

#endif /* #ifndef rspfTieGptSet_HEADER */
