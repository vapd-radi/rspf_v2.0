// Copyright (C) 2010 Argongra 
//
// RSPF is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//
// You should have received a copy of the GNU General Public License
// along with this software. If not, write to the Free Software 
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-
// 1307, USA.
//
// See the GPL in the COPYING.GPL file for more details.
//
//*************************************************************************


#include "rspfOpenCVLsdFilter.h"

#include <rspf/base/rspfRefPtr.h>
#include <rspf/imaging/rspfU8ImageData.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/imaging/rspfImageSourceFactoryBase.h>
#include <rspf/imaging/rspfImageSourceFactoryRegistry.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfNumericProperty.h>

#include "lsd.h"

RTTI_DEF1(rspfOpenCVLsdFilter, "rspfOpenCVLsdFilter", rspfImageSourceFilter)

rspfOpenCVLsdFilter::rspfOpenCVLsdFilter(rspfObject* owner)
   :rspfImageSourceFilter(owner),
	m_nCount(0),
    theTile(NULL),
    theScale(0.8),
	theOutASCII("")
{
	FILE *pf = fopen(theOutASCII, "w+");
	if(pf)	fclose(pf);	
}

rspfOpenCVLsdFilter::rspfOpenCVLsdFilter(rspfImageSource* inputSource,
                                           double scale = 0.8, rspfFilename outASCII = "", int saveImage = 0)
   : rspfImageSourceFilter(NULL, inputSource),
	m_nCount(0),
     theTile(NULL),
	 theScale(scale),
	 theOutASCII(outASCII),
	 theSaveImage(saveImage)
{
	FILE *pf = fopen(theOutASCII, "w+");
	if(pf)	fclose(pf);	
}

rspfOpenCVLsdFilter::rspfOpenCVLsdFilter(rspfObject* owner,
                                           rspfImageSource* inputSource,
                                           double scale = 0.8, rspfFilename outASCII = "", int saveImage = 0)
   : rspfImageSourceFilter(owner, inputSource),
	m_nCount(0),
     theTile(NULL),
	 theScale(scale),
	 theOutASCII(outASCII),
	 theSaveImage(saveImage)
{
}

rspfOpenCVLsdFilter::~rspfOpenCVLsdFilter()
{
}

rspfRefPtr<rspfImageData> rspfOpenCVLsdFilter::getTile(const rspfIrect& tileRect, rspf_uint32 resLevel) 
{

	if(!isSourceEnabled())
   	{
	      return rspfImageSourceFilter::getTile(tileRect, resLevel);
	}
	long w     = tileRect.width();
	long h     = tileRect.height();
   
	if (1 == theSaveImage)
	{
		if(!theTile.valid()) initialize();
		if(!theTile.valid()) return 0;
	}	
  
	rspfRefPtr<rspfImageData> data = 0;
	if(theInputConnection)
	{
		data  = theInputConnection->getTile(tileRect, resLevel);
   	} else {
	      return 0;
   	}

	if(!data.valid()) return 0;
	if(data->getDataObjectStatus() == RSPF_NULL ||  data->getDataObjectStatus() == RSPF_EMPTY)
   	{
	     return 0;
   	}

	if (1 == theSaveImage)
	{
		theTile->setImageRectangle(tileRect);
		theTile->makeBlank();

		theTile->setOrigin(tileRect.ul());
	}
	runUcharTransformation(data.get(), tileRect.ul());
   
	//printf("Tile (%d,%d) finished!\n",tileRect.ul().x,tileRect.ul().y); 	
   	return theTile;
  
   
}

void rspfOpenCVLsdFilter::initialize()
{
  if(theInputConnection)
  {
      rspfImageSourceFilter::initialize();
	  
	  theTile = new rspfU8ImageData(this,
		  theInputConnection->getNumberOfOutputBands(),   
		  theInputConnection->getTileWidth(),
		  theInputConnection->getTileHeight());
	  theTile->initialize();
   }
}

rspfScalarType rspfOpenCVLsdFilter::getOutputScalarType() const
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getOutputScalarType();
   }
   return RSPF_UCHAR;
}

rspf_uint32 rspfOpenCVLsdFilter::getNumberOfOutputBands() const
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getNumberOfOutputBands();
   }
   return theInputConnection->getNumberOfOutputBands();
}

bool rspfOpenCVLsdFilter::saveState(rspfKeywordlist& kwl,
                                     const char* prefix)const
{
   rspfImageSourceFilter::saveState(kwl, prefix);

   kwl.add(prefix,
           "scale",
           theScale,
		   true);
   kwl.add(prefix,
	   "outASCII",
	   theOutASCII,
	   true);
   kwl.add(prefix,
	   "saveImage",
	   theSaveImage,
	   true);
   kwl.add(prefix,
	   "lineSegments",
	   theLineSegments,
	   true);
   
   return true;
}

bool rspfOpenCVLsdFilter::loadState(const rspfKeywordlist& kwl,
                                     const char* prefix)
{
   rspfImageSourceFilter::loadState(kwl, prefix);

   const char* lookup = kwl.find(prefix, "scale");
   if(lookup)
   {
      theScale = rspfString(lookup).toDouble();
      printf("Read from spec file. scale: %f\n",theScale);
   }
   lookup = kwl.find(prefix, "outASCII");
   if(lookup)
   {
	   theOutASCII = rspfString(lookup);
	   printf("Read from spec file. outASCII: %s\n", theOutASCII);
   }
   lookup = kwl.find(prefix, "saveImage");
   if(lookup)
   {
	   theSaveImage = rspfString(lookup).toInt();
	   printf("Read from spec file. saveImage: %d\n", theSaveImage);
   }
   lookup = kwl.find(prefix, "lineSegments");
   if(lookup)
   {
	   theLineSegments = rspfString(lookup).toInt();
	   printf("Read from spec file. lineSegments: %d\n", theLineSegments);
   }
   return true;
}

void rspfOpenCVLsdFilter::runUcharTransformation(rspfImageData* tile, rspfIpt offset)
{   

	IplImage *input;
	IplImage *output;

	char* bSrc;
	char* bDst;

	int nChannels = tile->getNumberOfBands();

	for(int k=0; k<nChannels; k++) {
		//printf("Channel %d\n",k);
		input=cvCreateImageHeader(cvSize(tile->getWidth(),tile->getHeight()),8,1);
		output=cvCreateImageHeader(cvSize(tile->getWidth(),tile->getHeight()),8,1);
		bSrc = static_cast<char*>(tile->getBuf(k));
		input->imageData=bSrc;
		bDst = static_cast<char*>(theTile->getBuf(k));
		
		output->imageData=bDst;
        	cvLsd(input, output, offset, theScale);
		cvReleaseImageHeader(&input);
		cvReleaseImageHeader(&output);
	}

	theTile->validate();   
}

void rspfOpenCVLsdFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
	if(!property) return;
    rspfString name = property->getName();

    if(name == "scale")
    {
            theScale = property->valueToString().toDouble();
	}
	else if(name == "outASCII")
	{
		theOutASCII = property->valueToString();
	}
	else if(name == "saveImage")
	{
		theSaveImage = property->valueToString().toInt();
	}
	else
	{
	  rspfImageSourceFilter::setProperty(property);
	}
}

rspfRefPtr<rspfProperty> rspfOpenCVLsdFilter::getProperty(const rspfString& name)const
{
	if(name == "scale")
    {
            rspfNumericProperty* numeric = new rspfNumericProperty(name,
                    rspfString::toString(theScale));
            numeric->setNumericType(rspfNumericProperty::rspfNumericPropertyType_FLOAT64);
            numeric->setCacheRefreshBit();
            return numeric;
	}
	if(name == "outASCII")
	{
		rspfNumericProperty* numeric = new rspfNumericProperty(name,
			rspfString::toString(theOutASCII));
		return numeric;
	}
	if(name == "saveImage")
	{
		rspfNumericProperty* numeric = new rspfNumericProperty(name,
			rspfString::toString(theSaveImage));
		numeric->setNumericType(rspfNumericProperty::rspfNumericPropertyType_INT);
		numeric->setCacheRefreshBit();
		return numeric;
	}
	if(name == "lineSegments")
	{
		rspfNumericProperty* numeric = new rspfNumericProperty(name, theLineSegments);
		return numeric;
	}
    return rspfImageSourceFilter::getProperty(name);
}

void rspfOpenCVLsdFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
	rspfImageSourceFilter::getPropertyNames(propertyNames);
	propertyNames.push_back("scale");
	propertyNames.push_back("outASCII");
	propertyNames.push_back("saveImage");
	propertyNames.push_back("lineSegments");
}

bool rspfOpenCVLsdFilter::cvLsd(IplImage* input, IplImage* output, rspfIpt offset, double scale/* = 0.8*/)
{
	double * image;
	double * out;

	int X = input->width;
	int Y = input->height;


	/* create a simple image: left half black, right half gray */
	image = (double *) malloc( X * Y * sizeof(double) );
	if( image == NULL )
	{
		fprintf(stderr,"error: not enough memory\n");
		exit(EXIT_FAILURE);
	}

	for(int i=0;i</*grey*/input->width;i++)
	{
		for(int j=0;j</*grey*/input->height;j++)
		{
			CvScalar s= cvGet2D(/*grey*/input,j,i);
			double pix= s.val[0];
			image[ i + j * X ]= pix;
		}
	}

	/* call LSD */
	int n;
	//out = lsd(&n, image, X, Y);
	out = lsd_scale(&n, image, X, Y, scale);

	//for (int i = 0;i < n;i++)
	//{
	//	//double r[7];
	//	//memcpy(r, out+i*7, n*sizeof(double));
	//	//theLinesegments.push_back(r);
	//	for (int j = 0;j < 7;j++)
	//	{
	//		rspfString str = rspfString::toString(out[i*7+j]);
	//		theLineSegments += str + "";
	//	}
	//	
	//}
	
	FILE* pf;
	if (0 == m_nCount++)
	{
		pf = fopen(theOutASCII.c_str(), "w+");
	}
	else
	{
		pf = fopen(theOutASCII.c_str(), "a+");
	}
	if (pf)
	{
		for (int i = 0;i < n;i++)
		{
			//x1,y1,x2,y2,width,p,-log10(NFA)
			fprintf(pf, "%15lf%15lf%15lf%15lf%15lf%15lf%15lf\n",
				out[ i * 7 + 0]+offset.x, out[ i * 7 + 1]+offset.y, out[ i * 7 + 2]+offset.x, out[ i * 7 + 3]+offset.y, out[ i * 7 + 4], out[ i * 7 + 5], out[ i * 7 + 6]);
		}
		fclose(pf);
	}

	if (1 == theSaveImage)
	{
		cvZero(output);
		for(int i=0;i<n;i++)
		{
			cvLine(output,cvPoint(out[ i * 7 + 0],out[ i * 7 + 1]),cvPoint(out[ i * 7 + 2],out[ i * 7 + 3]),CV_RGB(255,255,255),1, CV_AA);
		}
	}

	/* free memory */
	free( (void *) image );
	free( (void *) out );

	return true;
}