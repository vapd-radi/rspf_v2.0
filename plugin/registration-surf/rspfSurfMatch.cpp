
#include <fstream>
#include <iostream> //TBR DEBUG
using namespace std;

#include "rspfSurfMatch.h"
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>

static rspfTrace traceDebug("rspfSurfMatch:debug");

#define TILE_SIZE	512

RTTI_DEF2(rspfSurfMatch, "rspfSurfMatch",
          rspfOutputSource, rspfProcessInterface);

rspfSurfMatch::rspfSurfMatch(rspfImageSource* inputSource)
      :
      rspfOutputSource(NULL, // owner
                        1,
                        0,
                        true,
                        true),
      rspfProcessInterface(),
      theAreaOfInterest(),
      theFileStream(),
      theStoreFlag(false),
	  theHessianThreshold(50),
	  theMasterBand(0),
	  theSlaverBand(0),
	  theMasterFilename(""),
	  theSlaverFilename(""),
	  theOutputFilename("")
{
   connectMyInputTo(0, inputSource);
   theAreaOfInterest.makeNan();
}

rspfSurfMatch::~rspfSurfMatch()
{
}
void rspfSurfMatch::setProperty(rspfRefPtr<rspfProperty> property)
{
	if(!property) return;
	rspfString name = property->getName();

	if(name == "hessian_threshold")
	{
		theHessianThreshold = property->valueToString().toDouble();
	}
	if(name == "master_filename")
	{
		theMasterFilename = property->valueToString();
	}
	if(name == "slaver_filename")
	{
		theSlaverFilename = property->valueToString();
	}
	if(name == "master_band")
	{
		theMasterBand = property->valueToString().toInt();
	}
	if(name == "slaver_band")
	{
		theSlaverBand = property->valueToString().toInt();
	}
	if(name == "output_filename")
	{
		theOutputFilename = property->valueToString();
	}
	else
	{
		rspfOutputSource::setProperty(property);
	}
}

rspfRefPtr<rspfProperty> rspfSurfMatch::getProperty(const rspfString& name)const
{
	if(name == "hessian_threshold")
	{
		rspfNumericProperty* numeric = new rspfNumericProperty(name,
			rspfString::toString(theHessianThreshold));
		numeric->setNumericType(rspfNumericProperty::rspfNumericPropertyType_FLOAT64);
		numeric->setCacheRefreshBit();
		return numeric;
	}
	if(name == "master_filename")
	{
		rspfNumericProperty* numeric = new rspfNumericProperty(name, theMasterFilename);
		return numeric;
	}
	if(name == "slaver_filename")
	{
		rspfNumericProperty* numeric = new rspfNumericProperty(name, theSlaverFilename);
		return numeric;
	}
	if(name == "master_band")
	{
		rspfNumericProperty* numeric = new rspfNumericProperty(name,
			rspfString::toString(theMasterBand));
		numeric->setNumericType(rspfNumericProperty::rspfNumericPropertyType_INT);
		numeric->setCacheRefreshBit();
		return numeric;
	}
	if(name == "slaver_band")
	{
		rspfNumericProperty* numeric = new rspfNumericProperty(name,
			rspfString::toString(theSlaverBand));
		numeric->setNumericType(rspfNumericProperty::rspfNumericPropertyType_INT);
		numeric->setCacheRefreshBit();
		return numeric;
	}
	if(name == "output_filename")
	{
		rspfNumericProperty* numeric = new rspfNumericProperty(name, theOutputFilename);
		return numeric;
	}

	return rspfOutputSource::getProperty(name);
}

void rspfSurfMatch::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
	rspfOutputSource::getPropertyNames(propertyNames);
	propertyNames.push_back("hessian_threshold");
	propertyNames.push_back("master_filename");
	propertyNames.push_back("slave_filename");
	propertyNames.push_back("master_band");
	propertyNames.push_back("slaver_band");
	propertyNames.push_back("output_filename");
}

void rspfSurfMatch::addElementToList(CvSeq** root, CvSeq *elem)
{
	//if(*root)
	//{
	//	elem->h_next = *root;
	//	(*root)->h_prev = elem;
	//}
	//else
	//	elem->h_next = 0;

	//*root = elem;
	//(*root)->h_prev = 0;

	int length = (int)((*root)->elem_size/sizeof(float));
	int length1 = (int)(elem->elem_size/sizeof(float));
	assert( length == length1);

	CvSeqWriter seqWriter;
	cvStartAppendToSeq(*root, &seqWriter);
	CvSeqReader obj_reader;
	cvStartReadSeq( elem, &obj_reader );
	//int elem_size = (seqWriter).seq->elem_size;
	//seqWriter.ptr += elem_size * seqWriter.seq->total;

	for (int i = 0;i < (int)elem->total;++i)
	{
		CV_NEXT_SEQ_ELEM( obj_reader.seq->elem_size, obj_reader );
		//int memLength = sizeof(data);
		//int length = (int)(data->elem_size/sizeof(float));
		CV_WRITE_SEQ_ELEM_VAR( obj_reader.ptr, seqWriter );
		//CV_WRITE_SEQ_ELEM(*(CvSURFPoint*)cvGetSeqElem(elem, i), seqWriter);
		//assert( elem_size == sizeof(data));
		//if( (seqWriter).ptr >= (seqWriter).block_max )
		//{
		//	cvCreateSeqBlock( &seqWriter);
		//}
		//assert( (seqWriter).ptr <= (seqWriter).block_max - elem_size);
		//memcpy((seqWriter).ptr, obj_reader.ptr, elem_size);
		//(seqWriter).ptr += elem_size;
	}
	cvEndWriteSeq(&seqWriter);
	//int num = seqWriter.seq->total;
	//cout<<num<<endl;
}

bool rspfSurfMatch::execute()
{
   bool status = true;
   static const char MODULE[] = "rspfSurfMatch::execute";

   if (traceDebug()) CLOG << " Entered..." << endl;


   // -- 1 -- create source handlers
   //rspfImageChain* theMChain = new rspfImageChain;
   //rspfImageChain* theSChain = new rspfImageChain;
   rspfRefPtr<rspfImageHandler> handlerM = rspfImageHandlerRegistry::instance()->open(theMasterFilename);

   if (!handlerM)
   {
	   cerr<<"rspfImageCorrelator"<<"::execute can't create handler for master image "<< theMasterFilename <<endl;
	   return false;
   }
   //rspfImageSourceSequencer* theImageSourceSequencer = PTR_CAST(rspfImageSourceSequencer, handlerM.get());
   //theImageSourceSequencer->setTileSize(rspfIpt(1024,1024));
   theMasterSource = handlerM.get();
   rspfRefPtr<rspfImageHandler> handlerS = rspfImageHandlerRegistry::instance()->open(theSlaverFilename);
   //handlerM->connectMyInputTo(0, theImageSourceSequencer);
   if (!handlerS)
   {
	   cerr<<"rspfImageCorrelator"<<"::execute can't create handler for slave image  "<< theSlaverFilename <<endl;
	   return false;
   }
   //theSChain->add(handlerS.get());
   theSlaverSource = handlerS.get();
   
   //open stream
   //open();

   //setProcessStatus(rspfProcessInterface::PROCESS_STATUS_EXECUTING);
   

   //do the actual work there
   CvSeq* MasterKeypoints = 0;
   CvSeq* MasterDescriptors = 0;
   CvSeq* SlaverKeypoints = 0;
   CvSeq* SlaverDescriptors = 0;


   cv::initModule_nonfree();

   CvMemStorage* storage = cvCreateMemStorage(0);
   MasterKeypoints = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvSURFPoint), storage);
   SlaverKeypoints = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvSURFPoint), storage);
   if (status) status = getMasterFeatures( &MasterKeypoints, &MasterDescriptors);
   if (status) status = getSlaverFeatures( &SlaverKeypoints, &SlaverDescriptors);
   

   vector<int> ptpairs;
   flannFindPairs( MasterKeypoints, MasterDescriptors, SlaverKeypoints, SlaverDescriptors, ptpairs );

   //-- Create input data
   arma::mat dataPoints((int)ptpairs.size()/2, 4);// = "0 0; 1 1; 2 2; 3 3";
   for(int i = 0;i < (int)ptpairs.size()/2;++i)
   {
	   CvSURFPoint* r1 = (CvSURFPoint*)cvGetSeqElem( MasterKeypoints, ptpairs[2*i] );
	   CvSURFPoint* r2 = (CvSURFPoint*)cvGetSeqElem( SlaverKeypoints, ptpairs[2*i+1] );

	   dataPoints(i, 0) = r1->pt.x;
	   dataPoints(i, 1) = r1->pt.y;
	   dataPoints(i, 2) = r2->pt.x;
	   dataPoints(i, 3) = r2->pt.y;
   }

   // RANSAC detect outliers
   auto_ptr< estimators::Solver<mat,vec> > ptrSolver(
	   new estimators::affineSolver<mat,vec>);
   vector<int> inliers;
   vector<vec> models;

   ransac::Ransac_Handler ransac_fun_Handler;
   bool result = ransac::Ransac_RobustEstimator
	   (
	   dataPoints, // the input data
	   estimators::affineSolver<mat,vec>::extractor, // How select sampled point from indices
	   dataPoints.n_rows,  // the number of putatives data
	   *(ptrSolver.get()),  // compute the underlying model given a sample set
	   estimators::affineSolver<mat,vec>::defaultEvaluator,  // the function to evaluate a given model
	   //Ransac Object that contain function:
	   // CandidatesSelector, Sampler and TerminationFunction
	   ransac_fun_Handler, // the basic ransac object
	   1000,  // the maximum rounds for RANSAC routine
	   inliers, // inliers to the final solution
	   models, // models array that fit input data
	   0.95 // the confidence want to achieve at the end
	   );

   FILE* pf = fopen(theOutputFilename, "w+");

   //write out projection info
   if (theOutputFilename != rspfFilename::NIL)
   {
	   rspfRefPtr<rspfImageGeometry> geom = theMasterSource->getImageGeometry();
	   rspfKeywordlist mpk;
	   if(geom.valid())
	   {
		   geom->saveState(mpk);
	   }
	   fprintf(pf, "%s", mpk.toString());
   }

   for(int i = 0;i < (int)inliers.size();++i)
   {
	   fprintf(pf, "%6d%15lf%15lf", i+1, dataPoints(inliers[i], 0), dataPoints(inliers[i], 1));
	   fprintf(pf, "%15lf%15lf%15lf\n", dataPoints(inliers[i], 2), dataPoints(inliers[i], 3), 0.0);
   }
   fclose(pf);

   //setProcessStatus(rspfProcessInterface::PROCESS_STATUS_NOT_EXECUTING);
   if (traceDebug()) CLOG << " Exited..." << endl;
   
   return status;
}

bool rspfSurfMatch::ExtractTile(rspfImageSource* imageSource,
								const rspfIrect& tileRect,
								int iband/* = 0*/,
								rspf_uint32 resLevel/* = 0*/,
								CvSeq** Keypoints/* = NULL*/,
								CvSeq** Descriptors/* = NULL*/)
{

	if(!isSourceEnabled())
	{
		return false;
	}
	long w     = tileRect.width();
	long h     = tileRect.height();
	
	rspfRefPtr<rspfImageData> data = 0;
	if(imageSource)
	{
		data  = imageSource->getTile(tileRect, resLevel);
	} else {
		return false;
	}

	if(!data.valid()) return 0;
	if(data->getDataObjectStatus() == RSPF_NULL ||  data->getDataObjectStatus() == RSPF_EMPTY)
	{
		return false;
	}

	IplImage *input = cvCreateImageHeader(cvSize(w, h),8,1);
	
	input->imageData = static_cast<char*>(data->getBuf(iband));

	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSURFParams params = cvSURFParams(theHessianThreshold, 1);
	CvSeq* KeypointsTemp = 0;
	CvSeq* DescriptorsTemp = 0;
	cvExtractSURF(input, NULL, &KeypointsTemp, &DescriptorsTemp, storage, params);


	//CvSeq *KeypointsNew=NULL;
	//KeypointsNew = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvSURFPoint), storage);

	for (int i = 0;i < KeypointsTemp->total;++i)
	{
		CvSURFPoint* corner = (CvSURFPoint*)cvGetSeqElem(KeypointsTemp,i);
		corner->pt.x += tileRect.ul().x;
		corner->pt.y += tileRect.ul().y;
		//int t = sizeof(corner);
		//CV_WRITE_SEQ_ELEM( corner, seqWriter );
		cvSeqPush(*Keypoints, corner);
	}
	//for( int i = 0; i < DescriptorsTemp->total; i++ )
	//{
	//	if( _keypoints )
	//	{
	//		CvSURFPoint pt = cvSURFPoint(kpt[i].pt, kpt[i].class_id, cvRound(kpt[i].size), kpt[i].angle, kpt[i].response);
	//		cvSeqPush(*_keypoints, &pt);
	//	}
	//	if( _descriptors )
	//		cvSeqPush(*_descriptors, descr.ptr((int)i));
	//}
	
	
	//if (!*Keypoints)
	//{
	//	*Keypoints = cvCloneSeq(KeypointsNew);
	//}
	//else
	//{
	//	addElementToList(Keypoints, KeypointsNew);

	//}
	
	if (!*Descriptors)
	{
		*Descriptors = cvCloneSeq(DescriptorsTemp);
	}
	else
	{
		addElementToList(Descriptors, DescriptorsTemp);
	}

	int num = (*Keypoints)->total;

	cvReleaseImageHeader(&input);

	return true;
}

bool rspfSurfMatch::getMasterFeatures(CvSeq** Keypoints, CvSeq** Descriptors)
{
	static const char MODULE[] = "rspfTieGenerator::getAllFeatures";

	if (traceDebug()) CLOG << " Entered..." << endl;

	rspfImageSource* master = theMasterSource.get();
	if (!master)
	{
		rspfNotify(rspfNotifyLevel_WARN)
			<< "WARN rspfTieGenerator::scanForEdges():"
			<< "\nInput source is not a rspfImageChip.  Returning..." << std::endl;
		return false;
	}
	// Some constants needed throughout...
	//const rspf_int32 TILE_HEIGHT    = master->getTileHeight();
	//const rspf_int32 TILE_WIDTH     = master->getTileWidth();
	//const rspf_int32 START_LINE = master->getBoundingRect().ul().y;
	//const rspf_int32 STOP_LINE  = master->getBoundingRect().lr().y;
	//const rspf_int32 START_SAMP = master->getBoundingRect().ul().x;
	//const rspf_int32 STOP_SAMP  = master->getBoundingRect().lr().x;

	const rspf_int32 TILE_HEIGHT    = TILE_SIZE;
	const rspf_int32 TILE_WIDTH     = TILE_SIZE;
	const rspf_int32 START_LINE = master->getBoundingRect().ul().y;
	const rspf_int32 STOP_LINE  = master->getBoundingRect().lr().y;
	const rspf_int32 START_SAMP = master->getBoundingRect().ul().x;
	const rspf_int32 STOP_SAMP  = master->getBoundingRect().lr().x;

	// For percent complete status.
	rspf_int32 tilerows=(STOP_LINE-START_LINE+TILE_HEIGHT) / TILE_HEIGHT; //ceil : (stop-start+1+size-1)/size
	rspf_int32 tilecols=(STOP_SAMP-START_SAMP+TILE_WIDTH) / TILE_WIDTH;
	double total_tiles = ((double)tilerows)*tilecols;
	double tiles_processed = 0.0;

	// Set the status message to be "scanning source for edges..."
	rspfNotify(rspfNotifyLevel_INFO) << "Getting tie points..." << std::endl;

	// Start off with a percent complete at 0...
	setPercentComplete(0.0);

	// loop through all tiles
	// need to use a sequencer for parallelism in the future TBD
	//theTiePoints.clear();
	rspf_int32 line=START_LINE;
	rspf_int32 i,j;
	
	rspf_uint32 resLevel = 0;
	for (i=0;(i<tilerows)&&!needsAborting();++i)
	{
		rspf_int32 samp=START_SAMP;
		for (j=0;(j<tilecols)&&!needsAborting();++j )
		{
			// Get the tie points
			//TBC : can we go further than source edges with edge tiles?? TBD
			rspfIrect tileRect = rspfIrect(rspfIpt(samp, line),rspfIpt(samp+TILE_WIDTH-1,line+TILE_HEIGHT-1));
			
			ExtractTile(master, tileRect, theMasterBand, resLevel, Keypoints, Descriptors);

			samp+=TILE_WIDTH;
			// Set the percent complete.
			tiles_processed += 1.0;
			setPercentComplete(tiles_processed/total_tiles*50.0);

			//DEBUG TBR TBC
			// std::cout<<"p="<<tiles_processed/total_tiles*100.0<<std::endl;

		}
		line+=TILE_HEIGHT;
	}

	setPercentComplete(50.0);
	if (traceDebug()) CLOG << " Exited." << endl;
	return true;
}

bool rspfSurfMatch::getSlaverFeatures(CvSeq** Keypoints, CvSeq** Descriptors)
{
	static const char MODULE[] = "rspfTieGenerator::getAllFeatures";

	if (traceDebug()) CLOG << " Entered..." << endl;

	rspfImageSource* slaver = theSlaverSource.get();
	if (!slaver)
	{
		rspfNotify(rspfNotifyLevel_WARN)
			<< "WARN rspfTieGenerator::scanForEdges():"
			<< "\nInput source is not a rspfImageChip.  Returning..." << std::endl;
		return false;
	}

	// Some constants needed throughout...
	//const rspf_int32 TILE_HEIGHT    = slaver->getTileHeight();
	//const rspf_int32 TILE_WIDTH     = slaver->getTileWidth();
	////const rspf_int32 START_LINE = theAreaOfInterest.ul().y;
	////const rspf_int32 STOP_LINE  = theAreaOfInterest.lr().y;
	////const rspf_int32 START_SAMP = theAreaOfInterest.ul().x;
	////const rspf_int32 STOP_SAMP  = theAreaOfInterest.lr().x;
	//const rspf_int32 START_LINE = slaver->getBoundingRect().ul().y;
	//const rspf_int32 STOP_LINE  = slaver->getBoundingRect().lr().y;
	//const rspf_int32 START_SAMP = slaver->getBoundingRect().ul().x;
	//const rspf_int32 STOP_SAMP  = slaver->getBoundingRect().lr().x;

	const rspf_int32 TILE_HEIGHT    = TILE_SIZE;
	const rspf_int32 TILE_WIDTH     = TILE_SIZE;
	const rspf_int32 START_LINE = slaver->getBoundingRect().ul().y;
	const rspf_int32 STOP_LINE  = slaver->getBoundingRect().lr().y;
	const rspf_int32 START_SAMP = slaver->getBoundingRect().ul().x;
	const rspf_int32 STOP_SAMP  = slaver->getBoundingRect().lr().x;

	// For percent complete status.
	rspf_int32 tilerows=(STOP_LINE-START_LINE+TILE_HEIGHT) / TILE_HEIGHT; //ceil : (stop-start+1+size-1)/size
	rspf_int32 tilecols=(STOP_SAMP-START_SAMP+TILE_WIDTH) / TILE_WIDTH;
	double total_tiles = ((double)tilerows)*tilecols;
	double tiles_processed = 0.0;

	
	// Set the status message to be "scanning source for edges..."
	rspfNotify(rspfNotifyLevel_INFO) << "Getting tie points..." << std::endl;

	// Start off with a percent complete at 0...
	setPercentComplete(50.0);

	// loop through all tiles
	// need to use a sequencer for parallelism in the future TBD
	//theTiePoints.clear();
	rspf_int32 line=START_LINE;
	rspf_int32 i,j;
	
	rspf_uint32 resLevel = 0;
	for (i=0;(i<tilerows)&&!needsAborting();++i)
	{
		rspf_int32 samp=START_SAMP;
		for (j=0;(j<tilecols)&&!needsAborting();++j )
		{
			// Get the tie points
			//TBC : can we go further than source edges with edge tiles?? TBD
			rspfIrect tileRect = rspfIrect(rspfIpt(samp, line),rspfIpt(samp+TILE_WIDTH-1,line+TILE_HEIGHT-1));

			ExtractTile(slaver, tileRect, theSlaverBand, resLevel, Keypoints, Descriptors);

			samp+=TILE_WIDTH;
			// Set the percent complete.
			tiles_processed += 1.0;
			setPercentComplete(50.0+tiles_processed/total_tiles*50.0);

			//DEBUG TBR TBC
			// std::cout<<"p="<<tiles_processed/total_tiles*100.0<<std::endl;

		}
		line+=TILE_HEIGHT;
	}

	setPercentComplete(100.0);
	if (traceDebug()) CLOG << " Exited." << endl;
	return true;
}

void rspfSurfMatch::setAreaOfInterest(const rspfIrect& rect)
{
   theAreaOfInterest = rect;
}

bool rspfSurfMatch::isOpen()const
{
	return const_cast<ofstream*>(&theFileStream)->is_open();
}

bool rspfSurfMatch::open()
{
	if(isOpen())
	{
		close();
	}

	if (theOutputFilename == rspfFilename::NIL)
	{
		return false;
	}

	theFileStream.open(theOutputFilename.c_str());

	return theFileStream.good();
}

void rspfSurfMatch::close()
{
	if (isOpen()) theFileStream.close();
}