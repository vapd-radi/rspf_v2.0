//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Oscar Kramer (okramer@imagelinks.com)
//
// DESCRIPTION: Contains implementation of class rspfAtbController. This is the
//    controller managing the tonal balancing process. It's product is the
//    initialization of specialized remappers in each of the mosaic's member
//    images that results in a tonally balanced mosaic.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfAtbController.cpp 17195 2010-04-23 17:32:18Z dburken $

#include <rspf/imaging/rspfAtbController.h>

RTTI_DEF2(rspfAtbController, "rspfAtbController", rspfProcessInterface, rspfConnectableObject );

#include <stdio.h>
#include <rspf/imaging/rspfGridRemapEngineFactory.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/imaging/rspfAtbMatchPoint.h>
#include <rspf/imaging/rspfGridRemapSource.h>
#include <rspf/base/rspfConnectableContainer.h>
#include <rspf/imaging/rspfImageCombiner.h>

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfAtbController:exec");
static rspfTrace traceDebug ("rspfAtbController:debug");

static const rspfIpt DEFAULT_GRID_SPACING (64,64);
static const char* DEFAULT_ATB_REMAP_ENGINE = "rspfHsvGridRemapEngine";

//*****************************************************************************
//  CONSTRUCTOR:  rspfAtbController()
//   
//*****************************************************************************
rspfAtbController::rspfAtbController()
   :
      rspfConnectableObject(NULL, 0, 0, false, true),
      theContainer (0),
      theGridRemapEngine (0),
      theGridSpacing (DEFAULT_GRID_SPACING)
{
   static const char MODULE[] = "rspfAtbController Default Constructor";
   if (traceExec())  CLOG << "entering..." << endl;
   
   //***
   // Instantiate a default remap engine:
   //***
   theGridRemapEngine
     = rspfGridRemapEngineFactory::create(DEFAULT_ATB_REMAP_ENGINE);
   
   if (traceExec())  CLOG << "returning..." << endl;
}

//*****************************************************************************
//  CONSTRUCTOR:  rspfAtbController()
//   
//*****************************************************************************
rspfAtbController::rspfAtbController(rspfImageCombiner* combiner,
                                       rspfGridRemapEngine* engine)
   :
      rspfConnectableObject(NULL, 0, 0, false, true),
      theContainer (0),
      theGridRemapEngine (0),
      theGridSpacing (DEFAULT_GRID_SPACING)
{
   static const char MODULE[] = "rspfAtbController Constructor #1";
   if (traceExec())  CLOG << "entering..." << endl;
   
   initializeWithCombiner(combiner, engine);
   
   if (traceExec())  CLOG << "returning..." << endl;
}

//*****************************************************************************
//  CONSTRUCTOR:  rspfAtbController()
//   
//*****************************************************************************
rspfAtbController::rspfAtbController(rspfGridRemapEngine* engine)
   :
      theContainer (0),
      theGridRemapEngine (0),
      theGridSpacing (DEFAULT_GRID_SPACING)
{
   static const char MODULE[] = "rspfAtbController Constructor #1";
   if (traceExec())  CLOG << "entering..." << endl;
   
   //***
   // Assign default engine if non provided:
   //***
   if (engine)
   {
      theGridRemapEngine = engine;
   }
   else
   {
      theGridRemapEngine
      = rspfGridRemapEngineFactory::create(DEFAULT_ATB_REMAP_ENGINE);
   }
   
   if (traceExec())  CLOG << "returning..." << endl;
}

//*****************************************************************************
//  DESTRUCTOR: ~rspfAtbController()
//
//  The remappers are left behind, owned by the combiner's container. If the
//  combiner was owned by something other than a container (see initialize()),
//  then this controller will assume ownership of the allocated remappers and
//  delete them here.
//
//*****************************************************************************
rspfAtbController::~rspfAtbController()
{
   theContainer = 0;
   theGridRemapEngine = 0;;
}

//*****************************************************************************
//  METHOD: rspfAtbController::initializeWithCombiner()
//  
//*****************************************************************************
void rspfAtbController::initializeWithCombiner(rspfImageCombiner* combiner,
						rspfGridRemapEngine* engine)
{
   static const char MODULE[] = "rspfAtbController::initializeWithCombiner()";
   if (traceExec())  CLOG << "entering..." << endl;
   
   //***
   // Assign default engine if non provided:
   //***
   if (engine)
      theGridRemapEngine = (rspfGridRemapEngine*)engine->dup();
   else
      theGridRemapEngine
         = rspfGridRemapEngineFactory::create(DEFAULT_ATB_REMAP_ENGINE);
   
   //***
   // This is the first combiner initializing the controller. The combiner
   // will have all of the input sources connected already. We attach to
   // these:
   //***
   theContainer = PTR_CAST(rspfConnectableContainer, combiner->getOwner());
   
   //***
   // Need to scan the input connections of the combiner and insert a 
   // remapper if there is not one already at the end of the input chain:
   //***
   vector<rspfConnectableObject*> del_list;
   vector<rspfConnectableObject*> add_list;
   int numInputs = combiner->getNumberOfInputs();
   for(int i=0; i<numInputs; i++)
   {
      rspfImageSource* input_image = PTR_CAST(rspfImageSource,
                                               combiner->getInput(i));
      
      if (input_image)
      {
         rspfGridRemapSource* remapper = PTR_CAST(rspfGridRemapSource,
                                                   input_image);
         if (remapper)
         {
            //***
            // Remapper already present, set its remap engine to this
            // controller's remap engine:
            //***
            remapper->setRemapEngine(theGridRemapEngine);
         }
         
         else
         {
            //***
            // Need to instantiate remapper:
            //***
            remapper = new rspfGridRemapSource(input_image,
                                                theGridRemapEngine);
            if (theContainer.valid())
               theContainer->addChild(remapper);
            
            add_list.push_back(remapper);
            del_list.push_back(input_image);
         }

         connectMyInputTo(remapper);
      }            
   }
   
   //***
   // Need to delete the old inputs and add the new remappers to the
   // combiner's input list:
   //***
   vector<rspfConnectableObject*>::iterator add_iter = add_list.begin();
   vector<rspfConnectableObject*>::iterator del_iter = del_list.begin();
   while (add_iter != add_list.end())
   {
      (*add_iter)->connectMyInputTo(*del_iter);
      combiner->disconnect(*del_iter);
      combiner->connectMyInputTo(*add_iter);
      add_iter++;
      del_iter++;
   }
   combiner->initialize();
      
   if (traceExec())  CLOG << "returning..." << endl;
}

//*****************************************************************************
//  METHOD: rspfAtbController::enableImage()
//  
//  Enable an image in the mosaic to be included the tonal balancing process.
//  
//*****************************************************************************
bool rspfAtbController::enableImage(unsigned int index)
{
   if (index >= getNumberOfInputs())
      return false;

   rspfSource* remapper = PTR_CAST(rspfSource,
                                    getInput(index));
   if (remapper)
      remapper->enableSource();
                                    
   return true;
}

//*****************************************************************************
//  METHOD: rspfAtbController::disableImage()
//  
//  Disable an image in the mosaic from the tonal balancing process.
//  
//*****************************************************************************
bool rspfAtbController::disableImage(unsigned int index)
{
   if (index >= getNumberOfInputs())
      return false;

   rspfSource* remapper = PTR_CAST(rspfSource,
                                    getInput(index));
   if (remapper)
      remapper->disableSource();

   return true;
}

//*****************************************************************************
//  METHOD: rspfAtbController::lockImage()
//  
//  Locks a particular source for adjustment. A locked image
//  effectively defines the target values for all overlapping imagery.
//  
//*****************************************************************************
bool rspfAtbController::lockImage(unsigned int index)
{
   if (index >= getNumberOfInputs())
      return false;
   
   rspfGridRemapSource* remapper = PTR_CAST(rspfGridRemapSource,
                                              getInput(index));
   if (remapper)
      remapper->lock();

   return true;
}


//*****************************************************************************
//  METHOD: rspfAtbController::unlockImage()
//  
//  Unlocks a particular source for allowing its adjustment.
//  
//*****************************************************************************
bool rspfAtbController::unlockImage(unsigned int index)
{
   if (index >= getNumberOfInputs())
      return false;
   
   rspfGridRemapSource* remapper = PTR_CAST(rspfGridRemapSource,
                                              getInput(index));
   if (remapper)
      remapper->lock();

   return true;
}


//*****************************************************************************
//  METHOD: rspfAtbController::execute()
//  
//*****************************************************************************
bool rspfAtbController::execute()
{
   static const char MODULE[] = "rspfAtbController::execute()";
   if (traceExec())  CLOG << "entering..." << endl;
   
   setPercentComplete(0.0);

   rspfGridRemapSource* source;

   //***
   // First verify that all objects needed are initialized:
   //***
   int num_images = getNumberOfInputs();
   if ((!theGridRemapEngine) || (num_images < 2))
     {
     CLOG << "WARNING: execute method was called but the controller has not "
           << "been properly initialized. Ignoring request..." << endl;
      if (traceExec())  CLOG << "returning..." << endl;
      return false;
   }
   
   //***
   // establish the remap grids:
   //***
   initializeRemappers();
   setPercentComplete(10.0);
   
   //***
   // Establish grid of matchpoints:
   //***
   int numPoints = 0;
   rspfDpt mp_view_pt;
   theMatchPoints.clear();
   for (mp_view_pt.line  = theBoundingRect.ul().line;
        mp_view_pt.line <= theBoundingRect.lr().line;
        mp_view_pt.line += theGridSpacing.line)
   {
      for (mp_view_pt.samp  = theBoundingRect.ul().samp;
           mp_view_pt.samp <= theBoundingRect.lr().samp;
           mp_view_pt.samp += theGridSpacing.samp)
      {
         rspfAtbMatchPoint* mp = new rspfAtbMatchPoint(mp_view_pt,
                                                         theGridRemapEngine);
         theMatchPoints.push_back(mp);
         numPoints++;
      }
   }
   
   //***
   // Loop over each matchpoint to determine which images contribute statistics:
   //***
   setPercentComplete(20.0);
   for (int mp=0; mp<numPoints; mp++)
   {
      rspfDpt view_point (theMatchPoints[mp]->viewPoint());
      
      //***
      // Loop over each image source to check if point lies inside its bounding
      // rectangle, and add its contribution to the matchpoint's collection:
      //***
      for (int src=0; src<num_images; src++)
      {
         source = PTR_CAST(rspfGridRemapSource, getInput(src));
         if (source)
         {
            rspfDrect image_rect (source->getBoundingRect());
            if (image_rect.pointWithin(view_point))
               theMatchPoints[mp]->addImage(source);
         }
      }

      setPercentComplete(20.0 + 50.0*(mp/numPoints));
   }

   //***
   // All contributors have been included in all matchpoints' collections.
   // Compute the target pixel value for each matchpoint and communicate it
   // to the corresponding remappers:
   //***
   for (int mp=0; mp<numPoints; mp++)
   {
      setPercentComplete(70.0 + 30.0*(mp/numPoints));
      theMatchPoints[mp]->assignRemapValues();
   }

   //***
   // Finally, enable the remappers:
   //***
   for (int src=0; src<num_images; src++)
   {
      source = PTR_CAST(rspfGridRemapSource, getInput(src));
      if (source)
         source->enableSource();
   }
   setPercentComplete(100.0);
         
   if (traceExec())  CLOG << "returning..." << endl;
   return true;
}

//*****************************************************************************
//  METHOD: rspfAtbController::abort()
//  
//*****************************************************************************
void rspfAtbController::abort()
{
   static const char MODULE[] = "rspfAtbController::abort()";
   if (traceExec())  CLOG << "entering..." << endl;
   
   CLOG << "NOTICE: this method not yet implemented." << endl;
   
   if (traceExec())  CLOG << "returning..." << endl;
}

//*****************************************************************************
//  METHOD: rspfAtbController::saveState()
//  
//*****************************************************************************
bool rspfAtbController::saveState(rspfKeywordlist& /* kwl */) const
{
   static const char MODULE[] = "rspfAtbController::saveState()";
   if (traceExec())  CLOG << "entering..." << endl;
   
   CLOG << "NOTICE: this method not yet implemented." << endl;
   
   if (traceExec())  CLOG << "returning..." << endl;
   return true;
}

//*****************************************************************************
//  METHOD: rspfAtbController::loadState()
//  
//*****************************************************************************
bool rspfAtbController::loadState(const rspfKeywordlist& /* kwl */)
{
   static const char MODULE[] = "rspfAtbController::loadState()";
   if (traceExec())  CLOG << "entering..." << endl;
   
   CLOG << "NOTICE: this method not yet implemented." << endl;
   
   if (traceExec())  CLOG << "returning..." << endl;
   return true;
}


//*****************************************************************************
//  METHOD:  rspfAtbController::setKernelSize(N)
//  
//  Hook to set the size of the kernel used by all point sources in computing
//  their mean pixel value. The kernels will be resized to NxN.
//  
//*****************************************************************************
void rspfAtbController::setKernelSize(int side_size)
{
   static const char MODULE[] = "rspfAtbController::setKernelSize(N)";
   if (traceExec())  CLOG << "entering..." << endl;

   vector<rspfRefPtr<rspfAtbMatchPoint> >::iterator mpi = theMatchPoints.begin();
   while (mpi != theMatchPoints.end())
      (*mpi)->setKernelSize(side_size);

   if (traceExec())  CLOG << "returning..." << endl;
   return;
}


//*****************************************************************************
//  PRIVATE METHOD:  rspfAtbController::initializeRemappers()
//  
//*****************************************************************************
void rspfAtbController::initializeRemappers()
{
   static const char MODULE[] = "rspfAtbController::initializeRemappers()";
   if (traceExec())  CLOG << "entering..." << endl;

   int num_images = getNumberOfInputs();

   //***
   // Fetch the overall mosaic bounding rect:
   //***
   rspfGridRemapSource* remapper = PTR_CAST(rspfGridRemapSource,
                                             getInput(0));
   theBoundingRect = remapper->getBoundingRect();
   for (int src=1; src<num_images; src++)
   {
      theBoundingRect = theBoundingRect.combine(remapper->getBoundingRect());
   }
 
   //***
   // Loop over each contribution image source chain to fetch each image
   // bounding rect:
   //***
   double dx_ul, dy_ul, dx_lr, dy_lr;
   double grid_ul_x, grid_ul_y, grid_lr_x, grid_lr_y;
   for (int src=0; src<num_images; src++)
   {
      remapper = PTR_CAST(rspfGridRemapSource, getInput(src));
      rspfDrect image_rect (remapper->getBoundingRect());

      //***
      // Arrive at the number of grid posts from mosaic origin to image's grid
      // UL and LR corners:
      //***
      dx_ul = (image_rect.ul().x - theBoundingRect.ul().x)/theGridSpacing.x;
      dy_ul = (image_rect.ul().y - theBoundingRect.ul().y)/theGridSpacing.y;
      dx_lr = (image_rect.lr().x - theBoundingRect.lr().x)/theGridSpacing.x;
      dy_lr = (image_rect.lr().y - theBoundingRect.lr().y)/theGridSpacing.y;
   
      //***
      // Establish the view coordinates for the adjusted image grid:
      //***
      grid_ul_x =  theBoundingRect.ul().x + ceil(dx_ul)*theGridSpacing.x;
      grid_ul_y =  theBoundingRect.ul().y + ceil(dy_ul)*theGridSpacing.y;
      grid_lr_x =  theBoundingRect.lr().x + floor(dx_lr)*theGridSpacing.x;
      grid_lr_y =  theBoundingRect.lr().y + floor(dy_lr)*theGridSpacing.y;

      //***
      // Have the remapper initialize a new grid with the given rectangle and
      // spacing:
      //***
      rspfDrect grid_rect (grid_ul_x, grid_ul_y, grid_lr_x, grid_lr_y);
      remapper->initialize(grid_rect, theGridSpacing);
   }
      
   if (traceExec())  CLOG << "returning..." << endl;
   return;
}

//*****************************************************************************
//  METHOD: rspfAtbController::setGridRemapEngine()
//  
//*****************************************************************************
void rspfAtbController::setGridRemapEngine(rspfGridRemapEngine* engine)
{
   static const char MODULE[] = "rspfAtbController::setGridRemapEngine()";
   if (traceExec())  CLOG << "entering..." << endl;

   theGridRemapEngine = engine;

   //***
   // Communicate the new engine to all member's interested:
   //***
   rspfGridRemapSource* remapper;
   rspfConnectableObject::ConnectableObjectList::iterator iter=theInputObjectList.begin();
   while (iter != theInputObjectList.end())
   {
      remapper = PTR_CAST(rspfGridRemapSource, (*iter).get());
      remapper->setRemapEngine(engine);
      iter++;
   }

   vector<rspfRefPtr<rspfAtbMatchPoint> >::iterator match_point = theMatchPoints.begin();
   while (match_point != theMatchPoints.end())
   {
      (*match_point)->setGridRemapEngine(engine);
      match_point++;
   }

   if (traceExec())  CLOG << "returning..." << endl;
   return;
}


//*****************************************************************************
//  METHOD: rspfAtbController::canConnectMyInputTo()
//  
//*****************************************************************************
bool rspfAtbController::canConnectMyInputTo(rspf_int32 /* myInputIndex */,
                                             const rspfConnectableObject* object) const
{
   return (object&& PTR_CAST(rspfGridRemapSource, object));
}
