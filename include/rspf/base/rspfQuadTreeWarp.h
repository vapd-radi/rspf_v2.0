//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfQuadTreeWarp.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfQuadTreeWarp_HEADER
#define rspfQuadTreeWarp_HEADER

#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspf2dTo2dTransform.h>
#include <vector>
#include <iosfwd>

class rspfQuadTreeWarpNode;

class RSPFDLLEXPORT rspfQuadTreeWarpVertex
{
public:
   friend std::ostream& operator<<(std::ostream& out,
                              const rspfQuadTreeWarpVertex& rhs);
   
   rspfQuadTreeWarpVertex(const rspfDpt& position=rspfDpt(0,0),
                           const rspfDpt& delta=rspfDpt(0,0));

   ~rspfQuadTreeWarpVertex();
   
   void setPosition(const rspfDpt& position);

   void setDelta(const rspfDpt& delta);

   const rspfDpt& getDelta()const;

   const rspfDpt& getPosition()const;

   void addSharedNode(rspfQuadTreeWarpNode* node);

   void removeNode(rspfQuadTreeWarpNode* node);

   bool isShared()const;

   bool saveState(rspfKeywordlist& kwl,
                  const char* prefix)const;
   
   bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix);
   
   std::vector<rspfQuadTreeWarpNode*> theSharedNodeList;
   
   rspfDpt                       thePosition;
   rspfDpt                       theDelta;
   bool                           theLockedFlag;
   
};

class rspfQuadTreeWarpNode
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfQuadTreeWarpNode& rhs);

   rspfQuadTreeWarpNode();

   rspfQuadTreeWarpNode(const rspfDrect& bounds,
                         rspfQuadTreeWarpNode* parent=NULL,
                         rspfQuadTreeWarpVertex* ulVertex=(rspfQuadTreeWarpVertex*)NULL,
                         rspfQuadTreeWarpVertex* urVertex=(rspfQuadTreeWarpVertex*)NULL,
                         rspfQuadTreeWarpVertex* lrVertex=(rspfQuadTreeWarpVertex*)NULL,
                         rspfQuadTreeWarpVertex* llVertex=(rspfQuadTreeWarpVertex*)NULL);

   ~rspfQuadTreeWarpNode();

   bool hasValidVertices()const;
   
   void removeVertex(rspfQuadTreeWarpVertex* v);

   void removeVertices();
   
   bool isLeaf()const;

   void clear();

   void removeChild(rspfQuadTreeWarpNode* node);
   bool saveState(rspfKeywordlist& kwl,
                  const char* prefix = 0)const;
   
   bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix = 0);

   rspfDrect                 theBoundingRect;
   rspfQuadTreeWarpVertex*   theUlVertex;
   rspfQuadTreeWarpVertex*   theUrVertex;
   rspfQuadTreeWarpVertex*   theLrVertex;
   rspfQuadTreeWarpVertex*   theLlVertex;
   rspfQuadTreeWarpNode*     theParent;
   
   std::vector<rspfQuadTreeWarpNode*> theChildren;
};

class rspfQuadTreeWarp : public rspf2dTo2dTransform
{
public:
   friend std::ostream& operator<<(std::ostream& out,
                                   const rspfQuadTreeWarp& rhs);

   rspfQuadTreeWarp();
   
   rspfQuadTreeWarp(const rspfDrect& boundingRect,
                     const rspfDpt& ulShift=rspfDpt(0,0),
                     const rspfDpt& urShift=rspfDpt(0,0),
                     const rspfDpt& lrShift=rspfDpt(0,0),
                     const rspfDpt& llShift=rspfDpt(0,0));

   rspfQuadTreeWarp(const rspfQuadTreeWarp& rhs);

   virtual rspfObject* dup()const;

   
   virtual rspfDpt getOrigin()const;

   void create(const rspfDrect& boundingRect,
               const rspfDpt& ulShift=rspfDpt(0,0),
               const rspfDpt& urShift=rspfDpt(0,0),
               const rspfDpt& lrShift=rspfDpt(0,0),
               const rspfDpt& llShift=rspfDpt(0,0));
               
   /*!
    * Will warp the passed in point and placed the warped value in result
    */
   virtual void forward(const rspfDpt& pt,
                        rspfDpt& result)const;   
   /*!
    * Will warp the passed in point and overwrite it
    */
   virtual void forward(rspfDpt& pt)const;
   
//    void inverse(const rspfDpt& input,
//                 rspfDpt&       output) const;
   
   virtual std::ostream& print(std::ostream& out) const;
   virtual void printVertices(std::ostream& out)const;

   rspfQuadTreeWarpNode* getRoot();

   const rspfQuadTreeWarpNode* getRoot()const;

   bool isEmpty()const;

   rspfQuadTreeWarpVertex* findClosestVertex(rspfDpt& position);
   
   /*!
    * This method will go through the tree and try to locate the
    * vertex.  This is faster sine the tree is sorted.  Use
    * the getVertex to just search the internal shared vertex list.
    */
   rspfQuadTreeWarpVertex* findVertex(const rspfDpt& position);

   /*!
    * Searches the shared vetex list.  This will not go through the tree.
    * this is needed for load states to perform correctly.
    */
   rspfQuadTreeWarpVertex* getVertex(const rspfDpt& position);
   
   /*!
    * Will get the shift or delta value for the passed in pt.
    */
   void getShift(rspfDpt& result,
                 const rspfDpt& pt)const;

   /*!
    * The initial passed in node must not be a leaf.  It will
    * turn the passed in node into a leaf by recursively
    * pruning its children.  
    */
   void pruneTree(rspfQuadTreeWarpNode* node);
   
   // Search methods
   rspfQuadTreeWarpNode* findNode(const rspfDpt& pt);
   
   const rspfQuadTreeWarpNode* findNode(const rspfDpt& pt)const;
   
    /*!
    * Will get the shift or delta value for the passed in pt.
    */
   rspfDpt getShift(const rspfDpt& pt)const;
   
   void clear();

   void split(const rspfDpt& point,
              double splitHoriCoefficient=0.0,
              double splitVertCoefficient=0.0);

   /*!
    * The passed in node is suppose to be a leaf node.
    * it splits the leaf node horizontally and vertically
    * by the passed in percentage.  Use find node to
    * find the leaf node of a given point 
    */
   void split(rspfQuadTreeWarpNode* node,
              double splitHoriCoefficient=.5,
              double splitVertCoefficient=.5);


   const rspfQuadTreeWarpNode* findNode(const rspfQuadTreeWarpNode* node,
                                         const rspfDpt& pt)const;
   
   rspfQuadTreeWarpNode* findNode(rspfQuadTreeWarpNode* node,
                                   const rspfDpt& pt);

   void findAllNodes(std::vector<rspfQuadTreeWarpNode*>& result,
                     const rspfDpt& pt);
   void findAllNodes(std::vector<const rspfQuadTreeWarpNode*>& result,
                     const rspfDpt& pt)const;
   void findAllNodes(std::vector<rspfQuadTreeWarpNode*>& result,
                     rspfQuadTreeWarpNode* node,
                     const rspfDpt& pt);
   void findAllNodes(std::vector<const rspfQuadTreeWarpNode*>& result,
                     rspfQuadTreeWarpNode* node,
                     const rspfDpt& pt)const;
  
   bool isOnEdge(rspfQuadTreeWarpNode* node,
                 const rspfDpt& point)const;

   bool isOnPoint(rspfQuadTreeWarpNode* node,
                  const rspfDpt& point)const;

   void updateAllVericeLockFlags();

   const std::vector<rspfQuadTreeWarpVertex*>& getVertices()const;

   void setWarpEnabledFlag(bool flag);

   void setToIdentity();
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
      
protected:
   virtual ~rspfQuadTreeWarp();
   bool                             theWarpEnabledFlag;
   rspfQuadTreeWarpNode*           theTree;
   std::vector<rspfQuadTreeWarpVertex*> theVertexList;

   void getShift(rspfDpt& result,
                 const rspfQuadTreeWarpNode* node,
                 const rspfDpt& pt)const;

   void recursivePruneTree(rspfQuadTreeWarpNode* node);
   void pruneSharedVertices();
   void removeSharedVertex(rspfQuadTreeWarpVertex* v);

   void recursivePrint(std::ostream& out,
                       rspfQuadTreeWarpNode* node)const;
   void recursiveDelete(rspfQuadTreeWarpNode* node);

   void getNewQuads(rspfQuadTreeWarpNode* parent,
                    const rspfDrect& ul,
                    const rspfDrect& ur,
                    const rspfDrect& lr,
                    const rspfDrect& ll,
                    rspfQuadTreeWarpNode*& ulNode,
                    rspfQuadTreeWarpNode*& urNode,
                    rspfQuadTreeWarpNode*& lrNode,
                    rspfQuadTreeWarpNode*& llNode);
   
   void updateLockFlag(rspfQuadTreeWarpVertex* v);
   void updateDelta(rspfQuadTreeWarpVertex* v);
   
   bool recursiveSave(rspfQuadTreeWarpNode* node,
                      rspfKeywordlist& kwl,
                      const char* prefix)const;

   bool recursiveLoad(rspfQuadTreeWarpNode* node,
                      const rspfKeywordlist& kwl,
                      const char* prefix);

//   bool resetAllSharedLists();
   
TYPE_DATA
};

#endif
