//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfQuadTreeWarp.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <rspf/base/rspfQuadTreeWarp.h>
#include <algorithm>
#include <stack>
#include <iostream>

#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>

static rspfTrace traceExec  ("rspfQuadTreeWarp:exec");
static rspfTrace traceDebug ("rspfQuadTreeWarp:debug");

RTTI_DEF1(rspfQuadTreeWarp, "rspfQuadTreeWarp", rspf2dTo2dTransform);

rspfQuadTreeWarpVertex::rspfQuadTreeWarpVertex(
   const rspfDpt& position, const rspfDpt& delta)
   :thePosition(position),
    theDelta(delta),
    theLockedFlag(false)
{
}

rspfQuadTreeWarpVertex::~rspfQuadTreeWarpVertex()
{
   for(rspf_uint32 i = 0;i< theSharedNodeList.size();++i)
   {
      theSharedNodeList[i]->removeVertex(this);
   }
                  
}

void rspfQuadTreeWarpVertex::setPosition(const rspfDpt& position)
{
   thePosition = position;
}

void rspfQuadTreeWarpVertex::setDelta(const rspfDpt& delta)
{
   theDelta    = delta;
}

const rspfDpt& rspfQuadTreeWarpVertex::getDelta()const
{
   return theDelta;
}

const rspfDpt& rspfQuadTreeWarpVertex::getPosition()const
{
   return thePosition;
}

void rspfQuadTreeWarpVertex::addSharedNode(rspfQuadTreeWarpNode* node)
{
   if(node)
   {
      theSharedNodeList.push_back(node);
   }
}

void rspfQuadTreeWarpVertex::removeNode(rspfQuadTreeWarpNode* node)
{
   std::vector<rspfQuadTreeWarpNode*>::iterator iter = theSharedNodeList.begin();
   bool found = false;
   rspfQuadTreeWarpNode* removedNode = NULL;
   while((iter != theSharedNodeList.end())&&!found)
   {
      if(*iter == node)
      {
         removedNode = *iter;
         iter = theSharedNodeList.erase(iter);
         found = true;
      }
      else
      {
         ++iter;
      }
   }

   if(removedNode)
   {
      removedNode->removeVertex(this);
   }
}

bool rspfQuadTreeWarpVertex::isShared()const
{
   return (theSharedNodeList.size() > 0);
}

bool rspfQuadTreeWarpVertex::saveState(rspfKeywordlist& kwl,
                                      const char* prefix)const
{
   kwl.add(prefix,
           "x",
           thePosition.x,
           true);
   kwl.add(prefix,
           "y",
           thePosition.y,
           true);
   kwl.add(prefix,
           "dx",
           theDelta.x,
           true);
   kwl.add(prefix,
           "dy",
           theDelta.y,
           true);
   kwl.add(prefix,
           "lock_flag",
           (int)theLockedFlag,
           true);
   
   return true;
}

bool rspfQuadTreeWarpVertex::loadState(const rspfKeywordlist& kwl,
                                        const char* prefix)
{
   const char* x = kwl.find(prefix,
                            "x");
   const char* y = kwl.find(prefix,
                            "y");
   const char* dx = kwl.find(prefix,
                             "dx");
   const char* dy = kwl.find(prefix,
                             "dy");
   const char* lockedFlag = kwl.find(prefix,
                                     "lock_flag");
   if(x&&y&&dx&&dy&&lockedFlag)
   {
      thePosition.x = rspfString(x).toDouble();
      thePosition.y = rspfString(y).toDouble();
      theDelta.x    = rspfString(dx).toDouble();
      theDelta.y    = rspfString(dy).toDouble();
      theLockedFlag = rspfString(lockedFlag).toBool();
      
      return true;
   }
   
   return false;
}

rspfQuadTreeWarpNode::rspfQuadTreeWarpNode()
   :
   theUlVertex(NULL),
   theUrVertex(NULL),
   theLrVertex(NULL),
   theLlVertex(NULL),
   theParent(NULL)
{
   theBoundingRect.makeNan();
}

rspfQuadTreeWarpNode::rspfQuadTreeWarpNode(
   const rspfDrect& bounds,
   rspfQuadTreeWarpNode* parent,
   rspfQuadTreeWarpVertex* ulVertex,
   rspfQuadTreeWarpVertex* urVertex,
   rspfQuadTreeWarpVertex* lrVertex,
   rspfQuadTreeWarpVertex* llVertex)
   :
   theBoundingRect(bounds),
   theUlVertex(ulVertex),
   theUrVertex(urVertex),
   theLrVertex(lrVertex),
   theLlVertex(llVertex),
   theParent(parent)
{
}

rspfQuadTreeWarpNode::~rspfQuadTreeWarpNode()
{
   removeVertices();

   theUlVertex = NULL;
   theUrVertex = NULL;
   theLrVertex = NULL;
   theLlVertex = NULL;
   
}

bool rspfQuadTreeWarpNode::hasValidVertices()const
{
   return (theUlVertex&&theUrVertex&&theLrVertex&&theLlVertex);
}

void rspfQuadTreeWarpNode::removeVertex(rspfQuadTreeWarpVertex* v)
{
   if(!v) return;
   
   if(theUlVertex == v)
   {
      theUlVertex = NULL;
      v->removeNode(this);
   }
   if(theUrVertex == v)
   {
      theUrVertex = NULL;
      v->removeNode(this);
   }
   if(theLrVertex == v)
   {
      theLrVertex = NULL;
      v->removeNode(this);
   }
   if(theLlVertex == v)
   {
      theLlVertex = NULL;
      v->removeNode(this);
   }
}
void rspfQuadTreeWarpNode::removeVertices()
{
   removeVertex(theUlVertex);
   removeVertex(theUrVertex);
   removeVertex(theLrVertex);
   removeVertex(theLlVertex);
}

bool rspfQuadTreeWarpNode::isLeaf()const
{
   return (theChildren.size() == 0);
}

void rspfQuadTreeWarpNode::clear()
{
   theBoundingRect.makeNan();
   theChildren.clear();
   theParent = NULL;
   removeVertex(theUlVertex);
   removeVertex(theUrVertex);
   removeVertex(theLrVertex);
   removeVertex(theLlVertex);
}

void rspfQuadTreeWarpNode::removeChild(rspfQuadTreeWarpNode* node)
{
   std::vector<rspfQuadTreeWarpNode*>::iterator iter =  theChildren.begin();

   while(iter != theChildren.end())
   {
      if(*iter == node)
      {
         theChildren.erase(iter);
         return;
      }
      ++iter;
   }
}

bool rspfQuadTreeWarpNode::saveState(rspfKeywordlist& kwl,
                                      const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::UL_X_KW,
           theBoundingRect.ul().x,
           true);
   kwl.add(prefix,
           rspfKeywordNames::UL_Y_KW,
           theBoundingRect.ul().y,
           true);
   kwl.add(prefix,
           rspfKeywordNames::LR_X_KW,
           theBoundingRect.lr().x,
           true);
   kwl.add(prefix,
           rspfKeywordNames::LR_Y_KW,
           theBoundingRect.lr().y,
           true);
   
   return true;
}

bool rspfQuadTreeWarpNode::loadState(const rspfKeywordlist& kwl,
                                      const char* prefix)
{
   const char* ul_x = kwl.find(prefix, rspfKeywordNames::UL_X_KW);
   const char* ul_y = kwl.find(prefix, rspfKeywordNames::UL_Y_KW);
   const char* lr_x = kwl.find(prefix, rspfKeywordNames::LR_X_KW);
   const char* lr_y = kwl.find(prefix, rspfKeywordNames::LR_Y_KW);

   if(ul_x&&ul_y&&lr_x&&lr_y)
   {
      theBoundingRect = rspfDrect(rspfString(ul_x).toDouble(),
                                   rspfString(ul_y).toDouble(),
                                   rspfString(lr_x).toDouble(),
                                   rspfString(lr_y).toDouble());
      return true;
   }
   
   return false;
}


rspfQuadTreeWarp::rspfQuadTreeWarp()
   :theTree(NULL)
{
}

rspfQuadTreeWarp::rspfQuadTreeWarp(const rspfDrect& boundingRect,
                                     const rspfDpt& ulShift,
                                     const rspfDpt& urShift,
                                     const rspfDpt& lrShift,
                                     const rspfDpt& llShift)
   :theTree(NULL)
{
   create(boundingRect, ulShift, urShift, lrShift, llShift);
}

rspfQuadTreeWarp::rspfQuadTreeWarp(const rspfQuadTreeWarp& rhs)
   :rspf2dTo2dTransform(),
    theWarpEnabledFlag(true),
    theTree(NULL)
{
   rspfKeywordlist kwl;
   
   rhs.saveState(kwl);
   
   loadState(kwl);
}

rspfObject* rspfQuadTreeWarp::dup()const
{
   return new rspfQuadTreeWarp(*this);
}

rspfQuadTreeWarp::~rspfQuadTreeWarp()
{
   clear();
}
   
rspfDpt rspfQuadTreeWarp::getOrigin()const
{
   if(!isEmpty())
   {
      if(!theTree->theBoundingRect.hasNans())
      {
         return theTree->theBoundingRect.midPoint();
      }
      
   }
   
   return rspfDpt(0,0);
}

void rspfQuadTreeWarp::create(const rspfDrect& boundingRect,
                               const rspfDpt& ulShift,
                               const rspfDpt& urShift,
                               const rspfDpt& lrShift,
                               const rspfDpt& llShift)
{
   clear();

   theTree = new rspfQuadTreeWarpNode(boundingRect);

   rspfQuadTreeWarpVertex* ul = new rspfQuadTreeWarpVertex(boundingRect.ul(),
                                                             ulShift);
   rspfQuadTreeWarpVertex* ur = new rspfQuadTreeWarpVertex(boundingRect.ur(),
                                                             urShift);
   rspfQuadTreeWarpVertex* lr = new rspfQuadTreeWarpVertex(boundingRect.lr(),
                                                             lrShift);
   rspfQuadTreeWarpVertex* ll = new rspfQuadTreeWarpVertex(boundingRect.ll(),
                                                             llShift);
   ul->addSharedNode(theTree);
   ur->addSharedNode(theTree);
   lr->addSharedNode(theTree);
   ll->addSharedNode(theTree);

   theVertexList.push_back(ul);
   theVertexList.push_back(ur);
   theVertexList.push_back(lr);
   theVertexList.push_back(ll);
   
   theTree->theUlVertex = ul;
   theTree->theUrVertex = ur;
   theTree->theLrVertex = lr;
   theTree->theLlVertex = ll;
   
}

void rspfQuadTreeWarp::printVertices(std::ostream& out)const
{
   for(rspf_uint32 i = 0; i < theVertexList.size();++i)
   {
      out << *theVertexList[i] << "\n";
   }
}

rspfQuadTreeWarpNode* rspfQuadTreeWarp::getRoot()
{
   return theTree;
}

const rspfQuadTreeWarpNode* rspfQuadTreeWarp::getRoot()const
{
   return theTree;
}

bool rspfQuadTreeWarp::isEmpty()const
{
   return (theTree==NULL);
}

std::ostream& rspfQuadTreeWarp::print(std::ostream& out) const
{
   if(!isEmpty())
   {
      out << "___________VERTEX LIST____________________\n";
      printVertices(out);
      out << "___________TREE LIST____________________\n";
      
      recursivePrint(out, theTree);
   }
   else
   {
      out << "<empty tree>\n";
   }
   return out;
}

rspfQuadTreeWarpVertex* rspfQuadTreeWarp::findClosestVertex(rspfDpt& position)
{
   if(position.hasNans()) return (rspfQuadTreeWarpVertex*)NULL;
      
   double dist = 1.0/DBL_EPSILON;
   rspfQuadTreeWarpVertex* result = (rspfQuadTreeWarpVertex*)NULL;
   for(rspf_uint32 i = 0; i < theVertexList.size(); ++i)
   {
      double d = (position-theVertexList[i]->thePosition).length();
      if( d < dist)
      {
         result = theVertexList[i];
         dist = d;
      }
   }

   return result;
//    rspfQuadTreeWarpNode* node = findNode(position);

//    if(node&&node->hasValidVertices())
//    {
//       double ulDist = (position-node->theUlVertex->thePosition).length();
//       double urDist = (position-node->theUrVertex->thePosition).length();
//       double lrDist = (position-node->theLrVertex->thePosition).length();
//       double llDist = (position-node->theLlVertex->thePosition).length();

//       double minDist = std::min(ulDist, std::min(urDist, std::min(lrDist, llDist)));

//       if(minDist == ulDist)
//       {
//          return node->theUlVertex;
//       }
//       else if(minDist == urDist)
//       {
//          return node->theUrVertex;
//       }
//       else if(minDist == lrDist)
//       {
//          return node->theLrVertex;
//       }
//       else if(minDist == llDist)
//       {
//          return node->theLlVertex;
//       }
//    }

//    return ((rspfQuadTreeWarpVertex*)NULL);
}


rspfQuadTreeWarpVertex* rspfQuadTreeWarp::findVertex(const rspfDpt& position)
{
   rspf_uint32 i = 0;
   rspfQuadTreeWarpVertex* result = (rspfQuadTreeWarpVertex*)NULL;
   rspfQuadTreeWarpNode*   currentNode = theTree;
   
   if((currentNode)&&
      (currentNode->theBoundingRect.pointWithin(position)))
   {
      while((currentNode)&&
            (!currentNode->isLeaf()))
      {
         for(i = 0; i < currentNode->theChildren.size(); ++i)
         {
            if(currentNode->theChildren[i]->theBoundingRect.pointWithin(position))
            {
               currentNode = currentNode->theChildren[i];
               break;
            }
         }
      }

      if(currentNode&&currentNode->hasValidVertices())
      {
         if(currentNode->theUlVertex->thePosition == position)
         {
            result = currentNode->theUlVertex;
         }
         else if(currentNode->theUrVertex->thePosition == position)
         {
            result = currentNode->theUrVertex;            
         }
         else if(currentNode->theLrVertex->thePosition == position)
         {
            result = currentNode->theLrVertex;
         }
         else if(currentNode->theLlVertex->thePosition == position)
         {
            result = currentNode->theLlVertex;
         }
      }
               
   }

   return result;   
}

rspfQuadTreeWarpVertex* rspfQuadTreeWarp::getVertex(const rspfDpt& position)
{
   std::vector<rspfQuadTreeWarpVertex*>::iterator iter = theVertexList.begin();

   while(iter != theVertexList.end())
   {
      if(position == (*iter)->getPosition())
      {
         return (*iter);
      }
      ++iter;
   }
   
   return (rspfQuadTreeWarpVertex*)NULL;
}

void rspfQuadTreeWarp::clear()
{
   if(theTree)
   {     
      recursiveDelete(theTree);
      theTree = NULL;
   }

   for(rspf_uint32 i = 0; i < theVertexList.size(); ++i)
   {
      delete theVertexList[i];
   }
   
   theVertexList.clear();
}

void rspfQuadTreeWarp::forward(const rspfDpt& pt,
                                rspfDpt& result)const
{
   if(theWarpEnabledFlag)
   {
      rspfDpt shift;
      
      getShift(shift,
               pt);
      
      result = pt + shift;
   }
   else
   {
      result = pt;
   }
}

void rspfQuadTreeWarp::forward(rspfDpt& pt)const
{
   if(theWarpEnabledFlag)
   {
      rspfDpt shift;
      
      getShift(shift,
               pt);
      
      pt += shift;
   }
}

void rspfQuadTreeWarp::getShift(rspfDpt& result,
                                 const rspfDpt& pt)const
{
   getShift(result,
            findNode(pt),
            pt);
}

void rspfQuadTreeWarp::split(const rspfDpt& point,
                              double splitHoriCoefficient,
                              double splitVertCoefficient)
{
   rspfQuadTreeWarpNode* node = findNode(point);

   if(node)
   {
      if(splitHoriCoefficient == 0.0)
      {
         splitHoriCoefficient = (point.x - node->theBoundingRect.ul().x)/
                                node->theBoundingRect.width();
      }
      if(splitVertCoefficient == 0.0)
      {
         splitVertCoefficient = (point.y - node->theBoundingRect.ul().y)/
                                node->theBoundingRect.height();
      }
      split(node,
            splitHoriCoefficient,
            splitVertCoefficient);
   }
}


void rspfQuadTreeWarp::split(rspfQuadTreeWarpNode* node,
                              double splitHoriCoefficient,
                              double splitVertCoefficient)
{
   if(!node) return;

   splitHoriCoefficient = splitHoriCoefficient>1?1:splitHoriCoefficient;
   splitHoriCoefficient = splitHoriCoefficient<0?0:splitHoriCoefficient;
   splitVertCoefficient = splitVertCoefficient>1?1:splitVertCoefficient;
   splitVertCoefficient = splitVertCoefficient<0?0:splitVertCoefficient;

   if( ((splitHoriCoefficient == 0)&&(splitVertCoefficient == 0)) ||
       ((splitHoriCoefficient == 1)&&(splitVertCoefficient == 1)))
   {
      return;
   }

   if(node->isLeaf())
   {
      if(node->theBoundingRect.hasNans())
      {
         rspfNotify(rspfNotifyLevel_WARN)  << "WARNING: " << "rspfQuadTreeWarp::split, Node has nans for the rect and can't split\n";
      }
      else
      {
         rspfDpt ul = node->theBoundingRect.ul();
         rspfDpt ur = node->theBoundingRect.ur();
         rspfDpt lr = node->theBoundingRect.lr();
         rspfDpt ll = node->theBoundingRect.lr();
         
         
         double xmid = ul.x + (ur.x - ul.x)*splitHoriCoefficient;
         double ymid = ul.y + (ll.y - ul.y)*splitVertCoefficient;
         
         rspfDpt left(ul.x, ymid);
         
         rspfDpt right(ur.x,
                        ymid);
         
         rspfDpt top(xmid,
                      ul.y);
         
         rspfDpt bottom(xmid,
                         lr.y);


         rspfDrect ulRect(ul.x,
                           ul.y,
                           xmid,
                           ymid);
         
         rspfDrect urRect(top.x,
                           top.y,
                           right.x,
                           right.y);
         
         rspfDrect lrRect(xmid,
                           ymid,
                           lr.x,
                           lr.y);
         
         rspfDrect llRect(left.x,
                           left.y,
                           bottom.x,
                           bottom.y);
         
         rspfQuadTreeWarpNode* ulNode=(rspfQuadTreeWarpNode*)NULL;
         rspfQuadTreeWarpNode* urNode=(rspfQuadTreeWarpNode*)NULL;
         rspfQuadTreeWarpNode* lrNode=(rspfQuadTreeWarpNode*)NULL;
         rspfQuadTreeWarpNode* llNode=(rspfQuadTreeWarpNode*)NULL;

         getNewQuads(node,
                     ulRect,
                     urRect,
                     lrRect,
                     llRect,
                     ulNode,
                     urNode,
                     lrNode,
                     llNode);

         if(ulNode&&urNode&&lrNode&&llNode)
         {
            node->theChildren.push_back(ulNode);
            node->theChildren.push_back(urNode);
            node->theChildren.push_back(lrNode);
            node->theChildren.push_back(llNode);            
            
            // it's no longer a leaf so remove the vertices
            // from the list
            node->removeVertex(node->theUlVertex);
            node->removeVertex(node->theUrVertex);
            node->removeVertex(node->theLrVertex);
            node->removeVertex(node->theLlVertex);
         }         
      }
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING: " << "rspfQuadTreeWarp::split, can only split leaf nodes\n";
   }
   
   updateAllVericeLockFlags();
}

void rspfQuadTreeWarp::getNewQuads(rspfQuadTreeWarpNode* parent,
                                    const rspfDrect& ul,
                                    const rspfDrect& ur,
                                    const rspfDrect& lr,
                                    const rspfDrect& ll,
                                    rspfQuadTreeWarpNode*& ulNode,
                                    rspfQuadTreeWarpNode*& urNode,
                                    rspfQuadTreeWarpNode*& lrNode,
                                    rspfQuadTreeWarpNode*& llNode)
{
   rspfDpt midShift;
   getShift(midShift,
            parent,
            ul.lr());
   
   rspfQuadTreeWarpVertex* midV = new rspfQuadTreeWarpVertex(ul.lr(),
                                                               midShift);

   ulNode = new rspfQuadTreeWarpNode(ul,
                                      parent);
   urNode = new rspfQuadTreeWarpNode(ur,
                                      parent);
   lrNode = new rspfQuadTreeWarpNode(lr,
                                      parent);
   llNode = new rspfQuadTreeWarpNode(ll,
                                      parent);

   midV->addSharedNode(ulNode);
   midV->addSharedNode(urNode);
   midV->addSharedNode(lrNode);
   midV->addSharedNode(llNode);
   
   // get the shared vertices first.  We will add ourself
   // to the pointer list. when we construct
   // the quad nodes.  Note the mid point will be shared
   // by all quads and will be marked as adjustable
   //
   rspfQuadTreeWarpVertex* ulSharedV = getVertex(ul.ul());
   rspfQuadTreeWarpVertex* urSharedV = getVertex(ur.ur());
   rspfQuadTreeWarpVertex* lrSharedV = getVertex(lr.lr());
   rspfQuadTreeWarpVertex* llSharedV = getVertex(ll.ll());
   
   if(!ulSharedV||
      !urSharedV||
      !lrSharedV||
      !llSharedV)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL: " << "rspfQuadTreeWarp::split, can't locating shared vertices.  This Shouldn't happen!!!\n";
      return;
   }

   // we know that the midpoint is new and is shared by all quads
   // so we have 2 more to check
   rspfQuadTreeWarpVertex* topSharedV    = getVertex(ul.ur());
   rspfQuadTreeWarpVertex* bottomSharedV = getVertex(lr.ll());

   rspfQuadTreeWarpVertex* leftSharedV   = getVertex(ul.ll());
   rspfQuadTreeWarpVertex* rightSharedV  = getVertex(ur.lr());
   
   rspfDpt tempShift;
   
   if(!topSharedV)
   {
      getShift(tempShift, parent, ul.ur());
      topSharedV = new rspfQuadTreeWarpVertex(ul.ur(),
                                               tempShift);
      theVertexList.push_back(topSharedV);
   }
   if(!bottomSharedV)
   {
      getShift(tempShift, parent, ll.lr());
      bottomSharedV = new rspfQuadTreeWarpVertex(ll.lr(),
                                                  tempShift);
      
      theVertexList.push_back(bottomSharedV);
   }
   if(!leftSharedV)
   {
      getShift(tempShift, parent, ul.ll());
      leftSharedV = new rspfQuadTreeWarpVertex(ul.ll(),
                                               tempShift);
      theVertexList.push_back(leftSharedV);
   }
   if(!rightSharedV)
   {
      getShift(tempShift, parent, ur.lr());
      rightSharedV = new rspfQuadTreeWarpVertex(ur.lr(),
                                               tempShift);
      theVertexList.push_back(rightSharedV);
   }
   theVertexList.push_back(midV);

   topSharedV->addSharedNode(ulNode);
   topSharedV->addSharedNode(urNode);
   
   bottomSharedV->addSharedNode(llNode);
   bottomSharedV->addSharedNode(lrNode);
   leftSharedV->addSharedNode(ulNode);
   leftSharedV->addSharedNode(llNode);
   rightSharedV->addSharedNode(urNode);
   rightSharedV->addSharedNode(lrNode);

   ulSharedV->addSharedNode(ulNode); 
   urSharedV->addSharedNode(urNode);
   lrSharedV->addSharedNode(lrNode);
   llSharedV->addSharedNode(llNode);
  
   ulNode->theUlVertex = ulSharedV;
   ulNode->theUrVertex = topSharedV;
   ulNode->theLrVertex = midV;
   ulNode->theLlVertex = leftSharedV;

   urNode->theUlVertex = topSharedV;
   urNode->theUrVertex = urSharedV;
   urNode->theLrVertex = rightSharedV;
   urNode->theLlVertex = midV;

   lrNode->theUlVertex = midV;
   lrNode->theUrVertex = rightSharedV;
   lrNode->theLrVertex = lrSharedV;
   lrNode->theLlVertex = bottomSharedV;

   llNode->theUlVertex = leftSharedV;
   llNode->theUrVertex = midV;
   llNode->theLrVertex = bottomSharedV;
   llNode->theLlVertex = llSharedV;   
}

void rspfQuadTreeWarp::getShift(rspfDpt& result,
                                 const rspfQuadTreeWarpNode* node,
                                 const rspfDpt& pt)const
{
   result.x = 0.0;
   result.y = 0.0;

   if(!node)
   {
      return;
   }
   if(!node->isLeaf())
   {
      return;
   }

   if(node->hasValidVertices())
   {
      rspfDpt ulShift = node->theUlVertex->getDelta();
      rspfDpt urShift = node->theUrVertex->getDelta();
      rspfDpt lrShift = node->theLrVertex->getDelta();
      rspfDpt llShift = node->theLlVertex->getDelta();

      rspfDpt ul = node->theBoundingRect.ul();
      rspfDpt ur = node->theBoundingRect.ur();
      rspfDpt ll = node->theBoundingRect.ll();
      
      double horizontalPercent = fabs((pt.x-ul.x))/
                                 (ur.x-ul.x);
      
      double verticalPercent   = fabs((pt.y - ul.y))/
                                 (ll.y-ul.y);

      rspfDpt upper = ulShift + (urShift - ulShift)*horizontalPercent;
      rspfDpt lower = llShift + (lrShift - llShift)*horizontalPercent;
      
      result = upper + (lower - upper)*verticalPercent;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING: rspfQuadTreeWarp::getShift, " << "Node does not have valid vertices in rspfQuadTreeWarp::getShift\n";
   }
}

void rspfQuadTreeWarp::pruneTree(rspfQuadTreeWarpNode* node)
{
   if(node&&
      !node->isLeaf())
   {
      rspfQuadTreeWarpVertex* ulV = getVertex(node->theBoundingRect.ul());
      rspfQuadTreeWarpVertex* urV = getVertex(node->theBoundingRect.ur());
      rspfQuadTreeWarpVertex* lrV = getVertex(node->theBoundingRect.lr());
      rspfQuadTreeWarpVertex* llV = getVertex(node->theBoundingRect.ll());
      
      recursivePruneTree(node);

      if(ulV&&urV&&lrV&&llV)
      {
         node->theUlVertex = ulV;
         node->theUrVertex = urV;
         node->theLrVertex = lrV;
         node->theLlVertex = llV;

         ulV->addSharedNode(node);
         urV->addSharedNode(node);
         lrV->addSharedNode(node);
         llV->addSharedNode(node);
      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN) << "WARNING: rspfQuadTreeWarp::pruneTree, invlaid vertex find\n";
      }
      pruneSharedVertices();
      updateAllVericeLockFlags();
   }
}

void rspfQuadTreeWarp::recursivePruneTree(rspfQuadTreeWarpNode* node)
{
   if(!node||node->isLeaf()) return;
   
   for(rspf_uint32 i = 0; i < node->theChildren.size(); ++i)
   {
      recursivePruneTree(node->theChildren[i]);
      delete node->theChildren[i];
      node->theChildren[i] = NULL;
   }
   node->theChildren.clear();
}


rspfQuadTreeWarpNode* rspfQuadTreeWarp::findNode(const rspfDpt& pt)
{
   if((!pt.hasNans())&&(!isEmpty()))
   {
      if(theTree->theBoundingRect.pointWithin(pt))
      {
         return findNode(theTree,
                         pt);
      }
   }
   
   return (rspfQuadTreeWarpNode*)NULL;
}

rspfDpt rspfQuadTreeWarp::getShift(const rspfDpt& pt)const
{
   rspfDpt result;
   
   getShift(result, pt);
   
   return result;
}

const rspfQuadTreeWarpNode* rspfQuadTreeWarp::findNode(const rspfDpt& pt)const
{
   if((!pt.hasNans())&&(!isEmpty()))
   {
      if(theTree->theBoundingRect.pointWithin(pt))
      {
         return findNode(theTree,
                         pt);
      }
   }
   
   return (const rspfQuadTreeWarpNode*)NULL;
}

rspfQuadTreeWarpNode* rspfQuadTreeWarp::findNode(rspfQuadTreeWarpNode* node,
                                                   const rspfDpt& pt)
{
   rspfQuadTreeWarpNode* result = (rspfQuadTreeWarpNode*)NULL;
   
   if(!node)
   {
      return result;
   }
   if(node->isLeaf())
   {
      result = node;
   }
   else
   {
      bool found = false;
      for(rspf_uint32 i = 0; (i < node->theChildren.size())&&(!found); ++i)
      {
         if(node->theChildren[i]->theBoundingRect.pointWithin(pt))
         {
            result = findNode(node->theChildren[i],
                              pt);
            found = true;
         }
      }
   }

   return result;
}

void rspfQuadTreeWarp::findAllNodes(std::vector<rspfQuadTreeWarpNode*>& result,
                                     const rspfDpt& pt)
{
   if((!pt.hasNans())&&(!isEmpty()))
   {
      if(theTree->theBoundingRect.pointWithin(pt))
      {
         findAllNodes(result,
                      theTree,
                      pt);
      }
   }
   
}

void rspfQuadTreeWarp::findAllNodes(std::vector<const rspfQuadTreeWarpNode*>& result,
                                     const rspfDpt& pt)const
{
   if(!pt.hasNans()&&(!isEmpty()))
   {
      if(theTree->theBoundingRect.pointWithin(pt))
      {
         findAllNodes(result,
                      theTree,
                      pt);
      }
   }
   
}

const rspfQuadTreeWarpNode* rspfQuadTreeWarp::findNode(const rspfQuadTreeWarpNode* node,
                                                         const rspfDpt& pt)const
{
   const rspfQuadTreeWarpNode* result = (const rspfQuadTreeWarpNode*)NULL;
   
   if(!node)
   {
      return result;
   }
   if(node->isLeaf())
   {
      result = node;
   }
   else
   {
      bool found = false;
      for(rspf_uint32 i = 0; (i < node->theChildren.size())&&(!found); ++i)
      {
         if(node->theChildren[i]->theBoundingRect.pointWithin(pt))
         {
            result = findNode(node->theChildren[i],
                              pt);
            found = true;
         }
      }
   }

   return result;
}

void rspfQuadTreeWarp::findAllNodes(std::vector<rspfQuadTreeWarpNode*>& result,
                                     rspfQuadTreeWarpNode* node,
                                     const rspfDpt& pt)
{   
   if(node->isLeaf())
   {
      result.push_back(node);
   }
   else
   {
      for(rspf_uint32 i = 0;
          i < node->theChildren.size();
          ++i)
      {
         if(node->theChildren[i]->theBoundingRect.pointWithin(pt))
         {
            findAllNodes(result,
                         node->theChildren[i],
                         pt);
         }
      }
   }
}

void rspfQuadTreeWarp::findAllNodes(std::vector<const rspfQuadTreeWarpNode*>& result,
                                     rspfQuadTreeWarpNode* node,
                                     const rspfDpt& pt)const
{   
   if(!node) return;
   if(node->isLeaf())
   {
      result.push_back(node);
   }
   else
   {
      for(rspf_uint32 i = 0;
          i < node->theChildren.size();
          ++i)
      {
         if(node->theChildren[i]->theBoundingRect.pointWithin(pt))
         {
            findAllNodes(result,
                         node->theChildren[i],
                         pt);
         }
      }
   }
}
void rspfQuadTreeWarp::pruneSharedVertices()
{
   std::vector<rspfQuadTreeWarpVertex*>::iterator iter = theVertexList.begin();

   while(iter != theVertexList.end())
   {
      if( !(*iter)->isShared())
      {
         delete (*iter);
         iter = theVertexList.erase(iter);
      }
      else
      {
         ++iter;
      }
   }
}

void rspfQuadTreeWarp::removeSharedVertex(rspfQuadTreeWarpVertex* v)
{
   std::vector<rspfQuadTreeWarpVertex*>::iterator iter = std::find(theVertexList.begin(),
                                                                    theVertexList.end(),
                                                                    v);
   if(iter != theVertexList.end())
   {
      delete (*iter);
      iter = theVertexList.erase(iter);
   }
}


void rspfQuadTreeWarp::recursivePrint(std::ostream& out,
                                       rspfQuadTreeWarpNode* node)const
{
   if(node)
   {
      out << (*node) << "\n";
   }

   if(!node->isLeaf())
   {
      for(rspf_uint32 i =0; i < node->theChildren.size();++i)
      {
         recursivePrint(out, node->theChildren[i]);
      }
   }
}

void rspfQuadTreeWarp::recursiveDelete(rspfQuadTreeWarpNode* node)
{
   if(node->isLeaf())
   {
      delete node;
   }
   else
   {
      for(rspf_uint32 i = 0; i < node->theChildren.size(); ++ i)
      {
         recursiveDelete(node->theChildren[i]);
      }
      
      delete node;
   }
}
 
bool rspfQuadTreeWarp::isOnEdge(rspfQuadTreeWarpNode* node,
                                 const rspfDpt& point)const
{
   if(!node) return false;

   if(node->theBoundingRect.pointWithin(point))
   {
      double minx, maxx;
      double miny, maxy;
      node->theBoundingRect.getBounds(minx, miny, maxx, maxy);

      return ( (point.x == minx) ||
               (point.x == miny) ||
               (point.y == miny) ||
               (point.y == maxy) );
   }

   return false;
}

bool rspfQuadTreeWarp::isOnPoint(rspfQuadTreeWarpNode* node,
                                 const rspfDpt& point)const
{
   if(!node) return false;

   return ( (point == node->theBoundingRect.ul())||
            (point == node->theBoundingRect.ur())||
            (point == node->theBoundingRect.lr())||
            (point == node->theBoundingRect.ll()) );
}

void rspfQuadTreeWarp::updateLockFlag(rspfQuadTreeWarpVertex* v)
{
   std::vector<rspfQuadTreeWarpNode*> nodeList;
   
   findAllNodes(nodeList,
                v->getPosition());

   if(nodeList.size() != v->theSharedNodeList.size())
   {
      if(isOnEdge(theTree, v->getPosition()))
      {
         v->theLockedFlag = false;
      }
      else
      {
         v->theLockedFlag = true;
      }
   }
   else
   {
      v->theLockedFlag = false;
   }

   // if the original was not locked
   // then we need to make sure we change the delta
   // along the locked edge so to produce no artifacts
   //
   if(v->theLockedFlag)
   {
      updateDelta(v);
   }
}

void rspfQuadTreeWarp::updateDelta(rspfQuadTreeWarpVertex* v)
{
   rspfQuadTreeWarpVertex* top    = NULL;
   rspfQuadTreeWarpVertex* bottom = NULL;
   rspfQuadTreeWarpVertex* left   = NULL;
   rspfQuadTreeWarpVertex* right  = NULL;

   std::vector<rspfQuadTreeWarpVertex*>::iterator iter = theVertexList.begin();
   
   while(iter != theVertexList.end())
   {
      rspfQuadTreeWarpVertex* testV = (*iter);

      // test along the vertical
      if( (testV->thePosition.x == v->thePosition.x)&&
          (testV->thePosition.y != v->thePosition.y)&&
          (!testV->theLockedFlag))
      {
         if(testV->thePosition.y >  v->thePosition.y)
         {
            if(bottom)
            {
               if(bottom->thePosition.y > testV->thePosition.y)
               {
                  bottom = testV;
               }
            }
            else
            {
               bottom = testV;
            }
         }
         else
         {
            if(top)
            {
               if(top->thePosition.y < testV->thePosition.y)
               {
                  top = testV;
               }
            }
            else
            {
               top = testV;
            }            
         }
      }

      if( (testV->thePosition.y == v->thePosition.y)&&
          (testV->thePosition.x != v->thePosition.x)&&
          (!testV->theLockedFlag))
      {
         if(testV->thePosition.x >  v->thePosition.x)
         {
            if(right)
            {
               if(right->thePosition.x > testV->thePosition.x)
               {
                  right = testV;
               }
            }
            else
            {
               right = testV;
            }
         }
         else
         {
            if(left)
            {
               if(left->thePosition.x < testV->thePosition.x)
               {
                  left = testV;
               }
            }
            else
            {
               left = testV;
            }            
         }
      }
      
      ++iter;
   }
   rspfDpt topBottomDelta;
   rspfDpt leftRightDelta;

   topBottomDelta.makeNan();
   leftRightDelta.makeNan();
   
   if(top&&bottom)
   {
      double t = (v->thePosition.y - top->thePosition.y)/
                 (bottom->thePosition.y - top->thePosition.y);
      
      topBottomDelta = top->theDelta + (bottom->theDelta-top->theDelta)*t;
      v->theDelta = topBottomDelta;
      
   }
   if(left&&right)
   {
      double t = (v->thePosition.x - left->thePosition.x)/
                 (right->thePosition.x - left->thePosition.x);
      
      leftRightDelta = left->theDelta + (right->theDelta-left->theDelta)*t;
      v->theDelta = leftRightDelta;
   }

   if(top&&bottom&&left&&right)
   {
      v->theDelta = (topBottomDelta+leftRightDelta)*.5;
   }
}
   
void rspfQuadTreeWarp::updateAllVericeLockFlags()
{
   std::vector<rspfQuadTreeWarpVertex*>::iterator iter = theVertexList.begin();

   while(iter != theVertexList.end())
   {
      if(*iter)
      {
         updateLockFlag(*iter);
      }
      
      ++iter;
   }
}

const std::vector<rspfQuadTreeWarpVertex*>& rspfQuadTreeWarp::getVertices()const
{
   return theVertexList;
}

void rspfQuadTreeWarp::setWarpEnabledFlag(bool flag)
{
   theWarpEnabledFlag = flag;
}

void rspfQuadTreeWarp::setToIdentity()
{
   for(rspf_uint32 i = 0; i < theVertexList.size(); ++i)
   {
      theVertexList[i]->theDelta = rspfDpt(0,0);
   }
}

bool rspfQuadTreeWarp::saveState(rspfKeywordlist& kwl,
                                  const char* prefix)const
{
   for(rspf_uint32 i = 0; i < theVertexList.size(); ++i)
   {
      rspfString newPrefix = rspfString(prefix)+ "v" + rspfString::toString(i) +".";

      theVertexList[i]->saveState(kwl, newPrefix.c_str());
   }

   recursiveSave(theTree, kwl, prefix);
   
   return rspf2dTo2dTransform::saveState(kwl, prefix);   
}

bool rspfQuadTreeWarp::loadState(const rspfKeywordlist& kwl,
                                  const char* prefix)
{
   clear();
   rspfString newPrefix = rspfString(prefix);

   // load the vertices first
   //
   rspfString regExpression =  rspfString("^(") + newPrefix + "v[0-9]+\\.)";

   rspf_uint32 result = kwl.getNumberOfSubstringKeys(regExpression);
   
   rspf_uint32 numberOfMatches = 0;
   rspf_uint32 count = 0;
   while(numberOfMatches < result)
   {
      rspfString newPrefix = rspfString(prefix)+rspfString("v") + rspfString::toString(count) +".";
      
      rspfQuadTreeWarpVertex* vert = new rspfQuadTreeWarpVertex;
      
      if(!vert->loadState(kwl, newPrefix.c_str()))
      {
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL: "<< " rspfQuadTreeWarp::loadState, invalid load on vertex\n";
         delete vert;
         clear();
         
         return false;
      }
      else
      {
         ++numberOfMatches;
         theVertexList.push_back(vert);
      }
      
      ++count;
   }

   theTree = new rspfQuadTreeWarpNode;

   if(!theTree->loadState(kwl, prefix))
   {
      clear();
      return false;
   }
   if(!recursiveLoad(theTree, kwl, prefix))
   {
      clear();
      return false;
   }
   
   if(!rspf2dTo2dTransform::loadState(kwl, prefix))
   {
      clear();
      return false;
   }

   return true;
}

bool rspfQuadTreeWarp::recursiveSave(rspfQuadTreeWarpNode* node,
                                      rspfKeywordlist& kwl,
                                      const char* prefix)const
{
   if(!node) return false;
   
   if(!node->saveState(kwl,
                      prefix))
   {
      return false;
   }
   
   if(node->isLeaf())
   {
      return true;
   }
   else
   {
     for(rspf_uint32 i = 0; i < node->theChildren.size(); ++ i)
       {
	 rspfString newPrefix = rspfString(prefix) + rspfString::toString(i) + ".";
      
	 if(!recursiveSave(node->theChildren[i],
                           kwl,
                           newPrefix.c_str()))
         {
            return false;
         }
       }
   }

   return true;
}

bool rspfQuadTreeWarp::recursiveLoad(rspfQuadTreeWarpNode* node,
                                      const rspfKeywordlist& kwl,
                                      const char* prefix)
{
   if(!node) return false;
   rspfString copyPrefix = prefix;
   
   rspfQuadTreeWarpNode* ul = new rspfQuadTreeWarpNode;
   rspfQuadTreeWarpNode* ur = new rspfQuadTreeWarpNode;
   rspfQuadTreeWarpNode* lr = new rspfQuadTreeWarpNode;
   rspfQuadTreeWarpNode* ll = new rspfQuadTreeWarpNode;
   
   rspfString ulPre = copyPrefix + "0.";
   rspfString urPre = copyPrefix + "1.";
   rspfString lrPre = copyPrefix + "2.";
   rspfString llPre = copyPrefix + "3.";

   if(ul->loadState(kwl,
                    ulPre.c_str()))
   {
      ul->theParent = node;
      node->theChildren.push_back(ul);
      recursiveLoad(ul,
                    kwl,
                    ulPre.c_str());
   }
   else
   {
      delete ul;
      ul = NULL;
   }
   
   if(ur->loadState(kwl,
                    urPre.c_str()))
   {
      ur->theParent = node;
      node->theChildren.push_back(ur);
      recursiveLoad(ur,
                    kwl,
                    urPre.c_str());
   }
   else
   {
      delete ur;
      ur = NULL;
   }

   if(lr->loadState(kwl,
                    lrPre.c_str()))
   {
      lr->theParent = node;
      node->theChildren.push_back(lr);
      recursiveLoad(lr,
                    kwl,
                    lrPre.c_str());
   }
   else
   {
      delete lr;
      lr = NULL;
   }

   if(ll->loadState(kwl,
                    llPre.c_str()))
   {
      ll->theParent = node;
      node->theChildren.push_back(ll);
      recursiveLoad(ll,
                    kwl,
                    llPre.c_str());
   }
   else
   {
      delete ll;
      ll = NULL;
   }

   if(node->isLeaf())
   {
      node->theUlVertex = getVertex(node->theBoundingRect.ul());
      node->theUrVertex = getVertex(node->theBoundingRect.ur());
      node->theLrVertex = getVertex(node->theBoundingRect.lr());
      node->theLlVertex = getVertex(node->theBoundingRect.ll());

      if(node->hasValidVertices())
      {
         node->theUlVertex->addSharedNode(node);
         node->theUrVertex->addSharedNode(node);
         node->theLrVertex->addSharedNode(node);
         node->theLlVertex->addSharedNode(node);
      }
      else
      {
         return false;
      }
   }
   
   return true;
}

std::ostream& operator<<(std::ostream& out, const rspfQuadTreeWarpVertex& rhs)
{
   out << "Position:     " << rhs.thePosition
       << "\nDelta:        " << rhs.theDelta
       << "\nLocked flag:  " << rhs.theLockedFlag
       << "\nShared nodes: " << rhs.theSharedNodeList.size() << std::endl;
   
   return out;
}

std::ostream& operator <<(std::ostream& out,
                          const rspfQuadTreeWarpNode& rhs)
{
   out << "Bounding rect: " << rhs.theBoundingRect << std::endl;
   
   if(rhs.theUlVertex)
   {
      out << "ulVertex:\n" << *rhs.theUlVertex<< std::endl;
   }
   if(rhs.theUrVertex)
   {
      out << "urVertex:\n" << *rhs.theUrVertex<< std::endl;
   }
   if(rhs.theLrVertex)
   {
      out << "lrVertex:\n" << *rhs.theLrVertex<< std::endl;
   }
   if(rhs.theLlVertex)
   {
      out << "llVertex:\n" << *rhs.theLlVertex;
   }
   
   return out;
}

std::ostream& operator<<(std::ostream& out, const rspfQuadTreeWarp& rhs)
{
   rhs.print(out);
   
   return out;
}

