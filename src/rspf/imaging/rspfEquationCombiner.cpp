//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfEquationCombiner.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <cstdlib>
#include <sstream>
using namespace std;

#include <rspf/imaging/rspfEquationCombiner.h>
#include <rspf/imaging/rspfCastTileSourceFilter.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfConvolutionSource.h>
#include <rspf/imaging/rspfSubImageTileSource.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/matrix/newmatio.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfNotifyContext.h>

RTTI_DEF1(rspfEquationCombiner, "rspfEquationCombiner", rspfImageCombiner);

static const char* EQUATION_KW = "equation";


class rspfBinaryOpAdd : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return v1 + v2;
      }
};

class rspfBinaryOpAnd : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return (double)(((rspf_uint32)v1) & ((rspf_uint32)v2));
      }
};

class rspfBinaryOpOr : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return (double)(((rspf_uint32)v1) | ((rspf_uint32)v2));
      }
};

class rspfBinaryOpXor : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return (double)(((rspf_uint32)v1) ^ ((rspf_uint32)v2));
      }
};

class rspfBinaryOpSub : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return v1 - v2;
      }
};

class rspfBinaryOpMax : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return std::max(v1, v2);
      }
};

class rspfBinaryOpMin : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return std::min(v1, v2);
      }
};

class rspfBinaryOpMul : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return v1 * v2;
      }
};

class rspfBinaryOpDiv : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         if(fabs(v2)>FLT_EPSILON)
            return v1 / v2;
         
         return 1.0/FLT_EPSILON;
      }
};

class rspfBinaryOpMod : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         if(fabs(v2)>FLT_EPSILON)
            return fmod(v1,v2);
         
         return 1.0/FLT_EPSILON;
      }
};

class rspfBinaryOpPow : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return pow(v1, v2);
      }
};
// boolean operators
class rspfBinaryOpEqual : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return (v1==v2)?1.0:0.0;
      }
};
class rspfBinaryOpGreater : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return (v1>v2)?1.0:0.0;
      }
};
class rspfBinaryOpGreaterOrEqual : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return (v1>=v2)?1.0:0.0;
      }
};
class rspfBinaryOpLess : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return (v1<v2)?1.0:0.0;
      }
};
class rspfBinaryOpLessOrEqual : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return (v1<=v2)?1.0:0.0;
      }
};
class rspfBinaryOpDifferent : public rspfEquationCombiner::rspfBinaryOp
{
public:
   virtual double apply(double v1, double v2)const
      {
         return (v1!=v2)?1.0:0.0;
      }
};
class rspfUnaryOpNot : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return 1-v;
      }
};

class rspfUnaryOpAbs : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return fabs(v);
      }
};

class rspfUnaryOpOnesComplement : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return (double)((rspf_uint8)~((rspf_uint8)v));
      }
};

class rspfUnaryOpLog : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return log(v);
      }
};

class rspfUnaryOpLog10 : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return log10(v);
      }
};

class rspfUnaryOpNeg : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return -v;
      }
};

class rspfUnaryOpSqrt : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         if(v >= 0)
         {
            return sqrt(v);
         }
         
         return -1;
      }
};

class rspfUnaryOpExp : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return exp(v);
      }
};

class rspfUnaryOpSin : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return sin(v);
      }
};

class rspfUnaryOpSind : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return sin(v*M_PI/180.0);
      }
};

class rspfUnaryOpASin : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         if(v > 1) v = 1;
         if(v < -1) v = -1;
         return asin(v);
      }
};

class rspfUnaryOpASind : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         if(v > 1) v = 1;
         if(v < -1) v = -1;
         return (180/M_PI)*asin(v);
      }
};

class rspfUnaryOpACos : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         if(v > 1) v = 1;
         if(v < -1) v = -1;
         return acos(v);
      }
};

class rspfUnaryOpACosd : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         if(v > 1) v = 1;
         if(v < -1) v = -1;
         return (180/M_PI)*acos(v);
      }
};

class rspfUnaryOpCos : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return cos(v);
      }
};

class rspfUnaryOpCosd : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return cos(v*M_PI/180.0);
      }
};

class rspfUnaryOpTan : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return tan(v);
      }
};

class rspfUnaryOpTand : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return tan(v*M_PI/180.0);
      }
};

class rspfUnaryOpATan : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return atan(v);
      }
};

class rspfUnaryOpATand : public rspfEquationCombiner::rspfUnaryOp
{
public:
   virtual double apply(double v)const
      {
         return (180/M_PI)*atan(v);
      }
};


rspfEquationCombiner::rspfEquationCombiner()
   :rspfImageCombiner(),
    theOutputScalarType(RSPF_FLOAT64),
    theEquation(""),
    theLexer(NULL),
    theTile(NULL),
    theCastFilter(NULL),
    theCastOutputFilter(NULL)
{
   theLexer      = new rspfEquTokenizer;
   theCastFilter = new rspfCastTileSourceFilter;
   theCastFilter->setOutputScalarType(RSPF_FLOAT64);
}

rspfEquationCombiner::~rspfEquationCombiner()
{
   if(theLexer)
   {
      delete theLexer;
      theLexer = NULL;
   }

   if(theCastFilter.valid())
   {
      theCastFilter->disconnect();
      theCastFilter = 0;
   }

   if(theCastOutputFilter.valid())
   {
      theCastOutputFilter->disconnect();
      theCastOutputFilter = 0;
   }
   // make sure they are cleared
   clearStacks();
}

double rspfEquationCombiner::getNullPixelValue(rspf_uint32 band)const
{
   
   if(theEquation == "")
   {
      if(getInput())
      {
         rspfImageSource* inter = PTR_CAST(rspfImageSource, getInput());
         if(inter)
         {
            return inter->getNullPixelValue(band);
         }
      }
   }
   return rspf::defaultNull(getOutputScalarType());
}   

double rspfEquationCombiner::getMinPixelValue(rspf_uint32 band)const
{
   if(theEquation == "")
   {
      if(getInput())
      {
         rspfImageSource* inter = PTR_CAST(rspfImageSource, getInput());
         if(inter)
         {
            return inter->getMinPixelValue(band);
         }
      }
   }
   return rspf::defaultMin(getOutputScalarType());
}

double rspfEquationCombiner::getMaxPixelValue(rspf_uint32 band)const
{
   if(theEquation == "")
   {
      if(getInput())
      {
         rspfImageSource* inter = PTR_CAST(rspfImageSource, getInput());
         if(inter)
         {
            return inter->getMaxPixelValue(band);
         }
      }
   }
   return rspf::defaultMax(getOutputScalarType());         
}

rspfScalarType rspfEquationCombiner::getOutputScalarType() const
{

   if(theEquation == "")
   {
      if(getInput())
      {
         rspfImageSource* inter = PTR_CAST(rspfImageSource, getInput());
         if(inter)
         {
            return inter->getOutputScalarType();
         }
      }
   }
   
   return theOutputScalarType;
}


rspfRefPtr<rspfImageData> rspfEquationCombiner::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(!theTile)
   {
      initialize();
   }
   long w = tileRect.width();
   long h = tileRect.height();
   long tw = theTile->getWidth();
   long th = theTile->getHeight();
   
   if(theEquation != "")
   {
      theTile->setImageRectangle(tileRect);
      
      if(w*h != tw*th)
      {
         theTile->initialize();
      }
      else
      {
         theTile->makeBlank();
      }
      theCurrentResLevel = resLevel;
      
      rspfRefPtr<rspfImageData> outputTile =  parseEquation();

      if(theCastOutputFilter.valid())
      {
         outputTile = theCastOutputFilter->applyCast(outputTile);
      }
      
      return outputTile;
   }
   else
   {
      if(getInput())
      {
         rspfImageSource* inter =
            PTR_CAST(rspfImageSource, getInput());
         if(inter)
         {
            return inter->getTile(tileRect, resLevel);
         }
      }
   }

   return rspfRefPtr<rspfImageData>();
}

void rspfEquationCombiner::setOutputScalarType(rspfScalarType scalarType)
{
   if(theOutputScalarType != scalarType)
   {
      theOutputScalarType = scalarType;

      if(theOutputScalarType == RSPF_SCALAR_UNKNOWN)
      {
         theOutputScalarType = RSPF_FLOAT64;
      }
      if(theCastOutputFilter.valid())
      {
         theCastOutputFilter = 0;
      }

      if(theOutputScalarType != RSPF_FLOAT64)
      {
         theCastOutputFilter = new rspfCastTileSourceFilter;
         theCastOutputFilter->setOutputScalarType(theOutputScalarType);
         theCastOutputFilter->connectMyInputTo(0, this);
         theCastOutputFilter->initialize();
      }
   }
}

void rspfEquationCombiner::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property) return;
   
   if(property->getName() == "Equation")
   {
      theEquation = property->valueToString();
   }
   else if(property->getName() == "Output scalar type")
   {
      setOutputScalarType(rspfScalarTypeLut::instance()->
                          getScalarTypeFromString(property->valueToString()));
   }
   else
   {
      rspfImageCombiner::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfEquationCombiner::getProperty(const rspfString& name)const
{
   if(name == "Equation")
   {
      rspfStringProperty* stringProp = new rspfStringProperty("Equation",
								theEquation,
								false);
      stringProp->clearChangeType();
      stringProp->setReadOnlyFlag(false);
      stringProp->setCacheRefreshBit();
      
      return stringProp;
   }
   else if(name == "Output scalar type")
   {
      rspfScalarTypeLut* sl = rspfScalarTypeLut::instance();
      
      std::vector<rspfString> scalarNames;

      rspf_int32 tableSize = (rspf_int32)sl->getTableSize();
      rspf_int32 idx;

      for(idx = 0; idx < tableSize; ++idx)
      {
         scalarNames.push_back(sl->getEntryString(idx));
      }
      rspfStringProperty* stringProp = new rspfStringProperty("Output scalar type",
								sl->getEntryString((rspf_int32)theOutputScalarType),
								false,
								scalarNames);
      stringProp->clearChangeType();
      stringProp->setReadOnlyFlag(false);
      stringProp->setCacheRefreshBit();
      
      return stringProp;
      
   }

   return rspfImageCombiner::getProperty(name);
}

void rspfEquationCombiner::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageCombiner::getPropertyNames(propertyNames);
   propertyNames.push_back("Equation");
   propertyNames.push_back("Output scalar type");
}


void rspfEquationCombiner::initialize()
{
   rspfImageCombiner::initialize();

   theTile = rspfImageDataFactory::instance()->create(this, RSPF_FLOAT64, getNumberOfOutputBands(), getTileWidth(), getTileHeight());
   theTile->initialize();
   if(theCastOutputFilter.valid())
   {
      theCastOutputFilter->initialize();
   }
}

void rspfEquationCombiner::assignValue()
{
   if(!theValueStack.empty())
   {
      if(theValueStack.top().type == RSPF_EQU_IMAGE_DATA_TYPE)
      {
         rspfImageData* topData = theValueStack.top().d.imageDataValue;
         rspf_uint32 minBands = std::min(theTile->getNumberOfBands(),
                                    topData->getNumberOfBands());
         rspf_uint32 maxBands = theTile->getNumberOfBands();
         rspf_uint32 band   = 0;
         rspf_uint32 offset = 0;
         rspf_uint32 size = theTile->getWidth()*theTile->getHeight();

         if(topData->getDataObjectStatus() == RSPF_PARTIAL)
         {
            for(band = 0; band < minBands; ++band)
            {
               double* inBuf  = (double*)topData->getBuf(band);
               double* outBuf = (double*)theTile->getBuf(band);
               double np      = topData->getNullPix(band);
               if(outBuf && inBuf)
               {
                  for(offset = 0; offset < size; ++offset)
                  {
                     if(*inBuf != np)
                     {
                        *outBuf = *inBuf;
                     }
                     ++outBuf;
                     ++inBuf;
                  }
               }
            }
            for(;band < maxBands; ++band)
            {
               double* inBuf  = (double*)topData->getBuf(minBands-1);
               double* outBuf = (double*)theTile->getBuf(band);
               double np      = topData->getNullPix(band);
               
               if(outBuf && inBuf)
               {
                  for(offset = 0; offset < size; ++offset)
                  {
                     if(*inBuf != np)
                     {
                        *outBuf = *inBuf;
                     }
                     ++outBuf;
                     ++inBuf;
                  }
               }
            }

         }
         else if(topData->getDataObjectStatus() == RSPF_FULL)
         {
            for(band = 0; band < minBands; ++band)
            {
               double* inBuf  = (double*)theValueStack.top().d.imageDataValue->getBuf(band);
               double* outBuf = (double*)theTile->getBuf(band);
               if(outBuf && inBuf)
               {
                  for(offset = 0; offset < size; ++offset)
                  {
                     *outBuf = *inBuf;
                     ++outBuf;
                     ++inBuf;
                  }
               }
            }
            for(;band < maxBands; ++band)
            {
               double* inBuf  = (double*)theValueStack.top().d.imageDataValue->getBuf(minBands-1);
               double* outBuf = (double*)theTile->getBuf(band);
               
               if(outBuf && inBuf)
               {
                  for(offset = 0; offset < size; ++offset)
                  {
                     *outBuf = *inBuf;
                     ++outBuf;
                     ++inBuf;
                  }
               }
            }
         }
         
         // Delete the object indirectly through an rspfRefPtr.
         rspfRefPtr<rspfImageData> id = theValueStack.top().d.imageDataValue;
         id = NULL;
      }
      else
      {
         double* buf  = static_cast<double*>(theTile->getBuf());
         rspf_uint32 size = theTile->getSize();
         double value = (double)theValueStack.top().d.doubleValue;
         
         for(rspf_uint32 offset = 0; offset < size; ++offset)
         {
            *buf = value;
            ++buf;
         }
      }
      
      theValueStack.pop();
   }
}

void rspfEquationCombiner::clearStacks()
{

   while(!theValueStack.empty())
   {
      if(theValueStack.top().type == RSPF_EQU_IMAGE_DATA_TYPE)
      {
         // Delete the object indirectly through an rspfRefPtr.
         rspfRefPtr<rspfImageData> id = theValueStack.top().d.imageDataValue;
         id = NULL;
      }

      theValueStack.pop();
   }
}

void rspfEquationCombiner::clearArgList(vector<rspfEquValue>& argList)
{
   for(rspf_uint32 i = 0; i < argList.size(); ++i)
   {
      if(argList[i].type == RSPF_EQU_IMAGE_DATA_TYPE)
      {
         if(argList[i].d.imageDataValue)
         {
            // Delete the object indirectly through an rspfRefPtr.
            rspfRefPtr<rspfImageData> id = argList[i].d.imageDataValue;
            id = NULL;
            argList[i].d.imageDataValue = (rspfImageData*)NULL;
         }
      }
   }
   argList.clear();
}

void rspfEquationCombiner::deleteArgList(vector<rspfEquValue>& args)
{
   int i = 0;

   for(i = 0; i < (int)args.size(); ++i)
   {
      if(args[i].type == RSPF_EQU_IMAGE_DATA_TYPE)
      {
         if(args[i].d.imageDataValue)
         {
            // Delete the object indirectly through an rspfRefPtr.
            rspfRefPtr<rspfImageData> id = args[i].d.imageDataValue;
            id = NULL;
            args[i].d.imageDataValue = NULL;
         }
      }
   }
   
   args.clear();
}

bool rspfEquationCombiner::parseArgList(vector<rspfEquValue>& args,
                                         bool popValueStack)
{
   bool result = true;
   
   if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
   {
      theCurrentId = theLexer->yylex();
      do
      {
         if(parseExpression())
         {
            if(!theValueStack.empty())
            {
               args.push_back(theValueStack.top());
               if(popValueStack)
               {
                  theValueStack.pop();
               }
            }
            else
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "The expression at  arg " << (args.size()+1)
                  << " is empty" << endl;
               
               result = false;
            }
         }
         else
         {
            rspfNotify(rspfNotifyLevel_WARN)
               <<"Unable to parse expression" << endl;
            result = false;
         }
         
         if(theCurrentId == RSPF_EQU_TOKEN_COMMA)
         {
            theCurrentId = theLexer->yylex();
         }
         else if(theCurrentId != RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            rspfNotify(rspfNotifyLevel_WARN)
               <<"Missing comma in argument list" << endl;
            result = false;
         }
         
      }while(result&&(theCurrentId != RSPF_EQU_TOKEN_RIGHT_PAREN));
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "Starting left parenthesis missing from arg list" << endl;
      result = false;
   }
   
   if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
   {
      theCurrentId = theLexer->yylex(); // skip past right parenthesis
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         <<"No matching right parenthesis for arg list" << endl;
      result = false;
   }

   if(!result && popValueStack)
   {
      clearArgList(args);
   }
   return result;
}

bool rspfEquationCombiner::parseAssignBand()
{
   bool result = true;
   
   vector<rspfEquValue> argList;

   if(parseArgList(argList))
   {
      if((argList.size() == 3) ||
         (argList.size() == 4))
      {
         rspfEquValue v3 = argList[2];
         rspfEquValue v2 = argList[1];
         rspfEquValue v1 = argList[0];
         
         if(argList.size() == 3)
         {
            if((v1.type == RSPF_EQU_IMAGE_DATA_TYPE) &&
               (v2.type == RSPF_EQU_DOUBLE_TYPE))
            {
               rspfImageData *data = (rspfImageData*)v1.d.imageDataValue->dup();
               rspfEquValue v;
               
               if(v3.type == RSPF_EQU_IMAGE_DATA_TYPE)
               {
                  if(data->getBuf()&&
                     v3.d.imageDataValue->getBuf())
                  {
                     if((rspf_uint32)(v2.d.doubleValue) < data->getNumberOfBands())
                     {
                        data->assignBand(v3.d.imageDataValue,
                                         0,
                                         (rspf_uint32)v2.d.doubleValue);
                        
                     }
                  }
               }
               else
               {
                  if(data->getBuf()&&
                     (rspf_uint32)v2.d.doubleValue < data->getNumberOfBands())
                  {  
                     rspf_uint32 upper = data->getWidth()*data->getHeight();
                     double* buf = (double*)data->getBuf((rspf_uint32)v2.d.doubleValue);
                     double value = v3.d.doubleValue;
                     if(buf)
                     {
                        for(rspf_uint32 i = 0; i < upper; ++i)
                        {
                           *buf = value;
                           ++buf;
                        }  
                     }
                     else
                     {
                        result = false;
                     }
                  }
               }
               if(result)
               {
                  data->validate();
                  v.type = RSPF_EQU_IMAGE_DATA_TYPE;
                  v.d.imageDataValue = data;
                  theValueStack.push(v);
               }
            }
            else
            {
               result = false;
            }
         }
         else 
         {
            rspfEquValue v4 = argList[3];
            if((v1.type == RSPF_EQU_IMAGE_DATA_TYPE) &&
               (v2.type == RSPF_EQU_DOUBLE_TYPE)&&
               (v3.type == RSPF_EQU_IMAGE_DATA_TYPE)&&
               (v4.type == RSPF_EQU_DOUBLE_TYPE))
            {
               rspfImageData *data = (rspfImageData*)v1.d.imageDataValue->dup();
               rspfEquValue v;
               v.type = RSPF_EQU_IMAGE_DATA_TYPE;
               v.d.imageDataValue = data;
               if(data->getBuf()&&v3.d.imageDataValue->getBuf())
               {
                  
                  rspf_uint32 index1 = (rspf_uint32)v4.d.doubleValue;
                  rspf_uint32 index2 = (rspf_uint32)v2.d.doubleValue;
                  
                  if((index1 > data->getNumberOfBands()) ||
                     (index1 > v3.d.imageDataValue->getNumberOfBands()))
                  {
                     result = false;
                  }
                  else
                  {     
                     data->assignBand(v3.d.imageDataValue,
                                      index1,
                                      index2);
                     data->validate();
                  }
                  
               }
               
               theValueStack.push(v);
            }
         }
      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "Invalid number of arguments to assign_band" << endl;
         result = false;
      }
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "unable to parse arguments for assign band" << endl;
      result = false;
   }

   clearArgList(argList);

   return result;
}

bool rspfEquationCombiner::parseStdFuncs()
{
   bool result = true;

   switch(theCurrentId)
   {
   case RSPF_EQU_TOKEN_ASSIGN_BAND:
   {
      theCurrentId = theLexer->yylex();
      if(!parseAssignBand())
      {
         result = false;
      }
      
      break;
   }
   case RSPF_EQU_TOKEN_CONV:
   {
      theCurrentId = theLexer->yylex();
      vector<rspfEquValue> args;
      if(parseArgList(args))
      {
         rspfImageData* resultImage = (rspfImageData*)NULL;
         if(applyConvolution(resultImage,
                              args))
         {
            if(resultImage)
            {
               rspfEquValue v;
               v.type = RSPF_EQU_IMAGE_DATA_TYPE;
               v.d.imageDataValue = resultImage;
               
               theValueStack.push(v);
            }
            else
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "function conv error: resulting image is NULL" << endl;
               result = false;
            }
         }
         else
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "Unable to apply convolution" << endl;
            result = false;
         }
      }
      else
      {
         result = false;
      }
      
      break;
   }
   case RSPF_EQU_TOKEN_CLAMP:
   {
      theCurrentId = theLexer->yylex();
      vector<rspfEquValue> args;
      if(parseArgList(args))
      {
         rspfImageData* resultImage = (rspfImageData*)NULL;
         if(applyClamp(resultImage,
                       args))
         {
            if(resultImage)
            {
               rspfEquValue v;
               v.type = RSPF_EQU_IMAGE_DATA_TYPE;
               v.d.imageDataValue = resultImage;
               
               theValueStack.push(v);
            }
            else
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "function clamp error: resulting image is NULL" << endl;
               result = false;
            }
            
         }
         else
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "Unable to apply clamp" << endl;
            result = false;
         }
      }
      else
      {
         result = false;
      }
      
      break;
   }
   case RSPF_EQU_TOKEN_BAND:
   {
    // need to parse the following rule for blurr function
      //
      // band(image data, number)
      theCurrentId = theLexer->yylex();


      vector<rspfEquValue> argList;

      if(parseArgList(argList))
      {
         if(argList.size() == 2)
         {
            rspfEquValue v1 = argList[0];
            rspfEquValue v2 = argList[1];
            rspfImageData* tempData = NULL;
            rspf_uint32 bandNumber           = 0;
            if(v1.type == RSPF_EQU_IMAGE_DATA_TYPE)
            {
               tempData = v1.d.imageDataValue;
            }
            else
            {
               result = false;
            }
            if(v2.type == RSPF_EQU_DOUBLE_TYPE)
            {
               bandNumber = (rspf_uint32)(v2.d.doubleValue);
            }
            else
            {
               result = false;
            }
            if(bandNumber > tempData->getNumberOfBands())
            {
               result = false;
            }
            if(result)
            {
               rspfImageData* data = new rspfImageData(this,
                                                         RSPF_FLOAT64,
                                                         1);
               data->setWidthHeight(tempData->getWidth(),
                                    tempData->getHeight());
               data->setOrigin(tempData->getOrigin());
               data->setNullPix(tempData->getNullPix(bandNumber),
                                0);
               data->setMinPix(tempData->getMinPix(bandNumber),
                               0);
               data->setMaxPix(tempData->getMaxPix(bandNumber),
                               0);
               data->initialize();
               
               if((tempData->getBuf())&&
                  (bandNumber < tempData->getNumberOfBands()))
               {
                  data->assignBand(tempData,
                                   bandNumber,
                                   0);                           
                  data->validate();
               }
               rspfEquValue v;
               v.type = RSPF_EQU_IMAGE_DATA_TYPE;
               v.d.imageDataValue = data;
               theValueStack.push(v);
            }
            
            if(tempData)
            {
               // Delete the object indirectly through an rspfRefPtr.
               rspfRefPtr<rspfImageData> id = tempData;
               tempData = NULL;
            }
         }
         else
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "Invalid number of args in function band" << endl;
            
            result = false;
         }
      }
      else
      {
         result = false;
      }
      
      break;
   }
   case RSPF_EQU_TOKEN_BLURR:
   {
      theCurrentId = theLexer->yylex();
      vector<rspfEquValue> args;
      if(parseArgList(args))
      {
         rspfImageData* resultImage = (rspfImageData*)NULL;
         if(applyBlurr(resultImage,
                       args))
         {
            if(resultImage)
            {
               rspfEquValue v;
               v.type = RSPF_EQU_IMAGE_DATA_TYPE;
               v.d.imageDataValue = resultImage;
               
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
         else
         {
            result = false;
         }
      }
      else
      {
         result = false;
      }
      
      break;
   }
   case RSPF_EQU_TOKEN_SHIFT:
   {
      theCurrentId = theLexer->yylex();
      vector<rspfEquValue> args;
      if(parseArgList(args))
      {
         rspfImageData* resultImage = (rspfImageData*)NULL;
         if(applyShift(resultImage,
                       args))
         {
            if(resultImage)
            {
               rspfEquValue v;
               v.type = RSPF_EQU_IMAGE_DATA_TYPE;
               v.d.imageDataValue = resultImage;
               
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
         else
         {
            result = false;
         }
      }
      else
      {
         result = false;
      }
      
      break;
   }
   case RSPF_EQU_TOKEN_MAX:
   case RSPF_EQU_TOKEN_MIN:
   {
      rspfBinaryOp* op = NULL;
      if(theCurrentId == RSPF_EQU_TOKEN_MIN) op = new rspfBinaryOpMin;
      else op = new rspfBinaryOpMax;
      
      int argCount = 0;
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         bool done = false;
         while(!done)
         {
            if(parseExpression())
            {
               
               ++argCount;
               if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
               {
                  theCurrentId = theLexer->yylex();
                  
                  done = true;
               }
               else if(theCurrentId == RSPF_EQU_TOKEN_COMMA)
               {
                  theCurrentId = theLexer->yylex();
               }
               else
               {
                  result = false;
                  done = true;
               }
            }
            else
            {
               done = true;
               result = false;
            }
         }
         if((argCount > 1)&&result)
         {
            result = true;
            
            rspfEquValue v;
            rspfEquValue v1;
            rspfEquValue v2;

            v2 = theValueStack.top();
            theValueStack.pop();
            v1 = theValueStack.top();
            theValueStack.pop();
            argCount -=2;

            do
            {
               if(applyOp(*op,
                          v,
                          v1,
                          v2))
               {
                  theValueStack.push(v);
               }
               else
               {
                  result = false;
                  argCount = 0;
                  
               }
               --argCount;
               
               if((argCount>0)&&result)
               {
                  v2 = theValueStack.top();
                  theValueStack.pop();
                  v1 = theValueStack.top();
                  theValueStack.pop();
               }
               
            }while((argCount > 0)&&(result));
         }
         else
         {
            result = false;
         }
      }
      else
      {
         result = false;
      }
      if(op)
      {
         delete op;
         op = NULL;
      }
      break;
   }
   case RSPF_EQU_TOKEN_ABS:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpAbs(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_SIN:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpSin(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_SIND:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpSind(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_ASIN:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpASin(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_ASIND:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpASind(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_COS:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpCos(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_COSD:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpCosd(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_ACOS:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpACos(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_ACOSD:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpACosd(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_TAN:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpTan(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_TAND:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpTand(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_ATAN:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpATan(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_ATAND:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpATand(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_LOG:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpLog(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_LOG10:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpLog10(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_SQRT:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpSqrt(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   case RSPF_EQU_TOKEN_EXP:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_PAREN)
      {
         theCurrentId = theLexer->yylex();
         result = parseExpression();
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            if(theValueStack.size() > 0)
            {
               theCurrentId = theLexer->yylex();
               rspfEquValue v;
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
               
               applyOp(rspfUnaryOpExp(),
                       v,
                       v1);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
      break;
   }
   default:
   {
      result = false;
   }
   }

   return result;
}

bool rspfEquationCombiner::parseUnaryFactor()
{
   bool result = false;

   
   if(theCurrentId == RSPF_EQU_TOKEN_MINUS)
   {
      theCurrentId = theLexer->yylex();

      if(parseFactor())
      {
         if(theValueStack.size() > 0)
         {
            rspfEquValue v;
            rspfEquValue v1 = theValueStack.top();
            theValueStack.pop();
            
            applyOp(rspfUnaryOpNeg(),
                    v,
                    v1);
            
            theValueStack.push(v);
         }
         else
         {
            result = false;
         }

         result = true;
      }
      else
      {
         result = false;
      }
   }
   else if(theCurrentId == RSPF_EQU_TOKEN_TILDE)
   {
      theCurrentId = theLexer->yylex();

      if(parseFactor())
      {
         if(theValueStack.size() > 0)
         {
            rspfEquValue v;
            rspfEquValue v1 = theValueStack.top();
            theValueStack.pop();
            
            applyOp(rspfUnaryOpOnesComplement(),
                    v,
                    v1);
            
            theValueStack.push(v);
         }
         else
         {
            result = false;
         }

         result = true;
      }
      else
      {
         result = false;
      }
   }
   return result;
}

bool rspfEquationCombiner::parseFactor()
{
   bool result = false;

   switch(theCurrentId)
   {
   case RSPF_EQU_TOKEN_CONSTANT:
   {
      rspfEquValue v;

      v.type = RSPF_EQU_DOUBLE_TYPE;
      v.d.doubleValue = atof(theLexer->YYText());
      theValueStack.push(v);
      
      theCurrentId = theLexer->yylex();
      
      result =  true;
      break;
   }
   case RSPF_EQU_TOKEN_PI:
   {
      rspfEquValue v;

      v.type = RSPF_EQU_DOUBLE_TYPE;
      v.d.doubleValue = M_PI;
      theValueStack.push(v);
      
      theCurrentId = theLexer->yylex();
      
      result =  true;
      break;      
   }
   case RSPF_EQU_TOKEN_IMAGE_VARIABLE:
   {
      theCurrentId = theLexer->yylex();
      if(theCurrentId == RSPF_EQU_TOKEN_LEFT_ARRAY_BRACKET)
      {
         theCurrentId = theLexer->yylex();
         if(parseExpression())
         {
            if(!theValueStack.empty())
            {
               if(theValueStack.top().type == RSPF_EQU_DOUBLE_TYPE)
               {
                  if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_ARRAY_BRACKET)
                  {
                     theCurrentId = theLexer->yylex();
                     rspf_uint32 index = (rspf_uint32)theValueStack.top().d.doubleValue;
                     theValueStack.pop();
                     rspfRefPtr<rspfImageData> data = getNewImageData(index);
                     result = true;
                     if(data.valid())
                     {
                        rspfEquValue v;
                        v.type = RSPF_EQU_IMAGE_DATA_TYPE;
                        v.d.imageDataValue = data.release();
                        theValueStack.push(v);

                     }
                     else
                     {
                        rspfNotify(rspfNotifyLevel_WARN)
                           <<"Data is NULL for array operation" << endl;
                     }
                     result = true;
                  }
                  else
                  {
                     rspfNotify(rspfNotifyLevel_WARN)
                        << "Mismatched Right array bracket" << endl;
                  }
               }
               else
               {
                  rspfNotify(rspfNotifyLevel_WARN)
                     << "Expression between array brackets is not a number"
                     << endl;
               }
            }
            else
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "no expression within array brackets" << endl;
            }
         }
         else
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "Unabel to parse expression"<<endl;
         }
      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN)
            <<"Need left array brackets to access an input source"<<endl;
      }
      
      break;
   }
   case RSPF_EQU_TOKEN_LEFT_PAREN:
   {
      theCurrentId = theLexer->yylex();
      if(parseExpression())
      {
         if(theCurrentId == RSPF_EQU_TOKEN_RIGHT_PAREN)
         {
            result       =  true;
            theCurrentId = theLexer->yylex();
         }
         else
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "Right parenthesis missing" << endl;
            result = false;
         }
      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "Unable to parse expression within parenthesis" << endl;
         result = false;
      }
      
      break;
   }
   }
   if(!result) result = parseUnaryFactor();
   
   if(!result) result = parseStdFuncs();
   
   return result;
}

bool rspfEquationCombiner::parseRestOfTerm()
{
   //---
   // Parse the following rule:
   // RestOfTerm: * Factor RestOfTerm | / Factor RestOfTerm |
   // ^ Factor RestOfTerm
   //---
   bool result = true;
   
   switch(theCurrentId)
   {
      case RSPF_EQU_TOKEN_MULT:
      {
         theCurrentId = theLexer->yylex();
         if(parseFactor())
         {
            if(theValueStack.size() > 1)
            {
               rspfEquValue v;
            
               rspfEquValue v2 = theValueStack.top();
               theValueStack.pop();
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
            
               applyOp(rspfBinaryOpMul(),
                       v,
                       v1,
                       v2);
            
               theValueStack.push(v);
            }
            else
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "Multiplication requires two arguments" << endl;
               result = false;
            }
            if(result)
            {
               result = parseRestOfTerm();
            }
         }
         else
         {
            result = false;
         }
         break;
      }
      case RSPF_EQU_TOKEN_DIV:
      {
         theCurrentId = theLexer->yylex();
         if(parseFactor())
         {
            if(theValueStack.size() > 1)
            {
               rspfEquValue v;
            
               rspfEquValue v2 = theValueStack.top();
               theValueStack.pop();
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
            
               applyOp(rspfBinaryOpDiv(),
                       v,
                       v1,
                       v2);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
            if(result)
            {
               result = parseRestOfTerm();
            }
         }
         else
         {
            result = false;
         }

         break;
      }
      case RSPF_EQU_TOKEN_XOR:
      {
         theCurrentId = theLexer->yylex();
         if(parseFactor())
         {
            if(theValueStack.size() > 1)
            {
               rspfEquValue v;
            
               rspfEquValue v2 = theValueStack.top();
               theValueStack.pop();
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
            
               applyOp(rspfBinaryOpXor(),
                       v,
                       v1,
                       v2);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
            if(result)
            {
               result = parseRestOfTerm();
            }
         }
         else
         {
            result = false;
         }

         break;
      }
      case RSPF_EQU_TOKEN_AMPERSAND:
      {
         theCurrentId = theLexer->yylex();
         if(parseFactor())
         {
            if(theValueStack.size() > 1)
            {
               rspfEquValue v;
            
               rspfEquValue v2 = theValueStack.top();
               theValueStack.pop();
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
            
               applyOp(rspfBinaryOpAnd(),
                       v,
                       v1,
                       v2);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
            if(result)
            {
               result = parseRestOfTerm();
            }
         }
         else
         {
            result = false;
         }

         break;
      }
      case RSPF_EQU_TOKEN_OR_BAR:
      {
         theCurrentId = theLexer->yylex();
         if(parseFactor())
         {
            if(theValueStack.size() > 1)
            {
               rspfEquValue v;
            
               rspfEquValue v2 = theValueStack.top();
               theValueStack.pop();
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
            
               applyOp(rspfBinaryOpOr(),
                       v,
                       v1,
                       v2);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
            if(result)
            {
               result = parseRestOfTerm();
            }
         }
         else
         {
            result = false;
         }

         break;
      }
      case RSPF_EQU_TOKEN_MOD:
      {
         theCurrentId = theLexer->yylex();
         if(parseFactor())
         {
            if(theValueStack.size() > 1)
            {
               rspfEquValue v;
            
               rspfEquValue v2 = theValueStack.top();
               theValueStack.pop();
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
            
               applyOp(rspfBinaryOpMod(),
                       v,
                       v1,
                       v2);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
            if(result)
            {
               result = parseRestOfTerm();
            }
         }
         else
         {
            result = false;
         }

         break;
      }
      case RSPF_EQU_TOKEN_POWER:
      {
         theCurrentId = theLexer->yylex();
         if(parseFactor())
         {
            if(theValueStack.size() > 1)
            {
               rspfEquValue v;
            
               rspfEquValue v2 = theValueStack.top();
               theValueStack.pop();
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
            
               applyOp(rspfBinaryOpPow(),
                       v,
                       v1,
                       v2);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
            if(result)
            {
               result = parseRestOfTerm();
            }
         }
         else
         {
            result = false;
         }
         break;
      }
      case RSPF_EQU_TOKEN_BEQUAL:
      {
         theCurrentId = theLexer->yylex();
         if(parseFactor())
         {
            if(theValueStack.size() > 1)
            {
               rspfEquValue v;
            
               rspfEquValue v2 = theValueStack.top();
               theValueStack.pop();
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
            
               applyOp(rspfBinaryOpEqual(),
                       v,
                       v1,
                       v2);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
            if(result)
            {
               result = parseRestOfTerm();
            }
         }
         else
         {
            result = false;
         }

         break;
      }
      case RSPF_EQU_TOKEN_BGREATER:
      {
         theCurrentId = theLexer->yylex();
         if(parseFactor())
         {
            if(theValueStack.size() > 1)
            {
               rspfEquValue v;
            
               rspfEquValue v2 = theValueStack.top();
               theValueStack.pop();
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
            
               applyOp(rspfBinaryOpGreater(),
                       v,
                       v1,
                       v2);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
            if(result)
            {
               result = parseRestOfTerm();
            }
         }
         else
         {
            result = false;
         }

         break;
      }
      case RSPF_EQU_TOKEN_BGREATEROREQUAL:
      {
         theCurrentId = theLexer->yylex();
         if(parseFactor())
         {
            if(theValueStack.size() > 1)
            {
               rspfEquValue v;
            
               rspfEquValue v2 = theValueStack.top();
               theValueStack.pop();
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
            
               applyOp(rspfBinaryOpGreaterOrEqual(),
                       v,
                       v1,
                       v2);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
            if(result)
            {
               result = parseRestOfTerm();
            }
         }
         else
         {
            result = false;
         }

         break;
      }
      case RSPF_EQU_TOKEN_BLESS:
      {
         theCurrentId = theLexer->yylex();
         if(parseFactor())
         {
            if(theValueStack.size() > 1)
            {
               rspfEquValue v;
            
               rspfEquValue v2 = theValueStack.top();
               theValueStack.pop();
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
            
               applyOp(rspfBinaryOpLess(),
                       v,
                       v1,
                       v2);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
            if(result)
            {
               result = parseRestOfTerm();
            }
         }
         else
         {
            result = false;
         }

         break;
      }
      case RSPF_EQU_TOKEN_BLESSOREQUAL:
      {
         theCurrentId = theLexer->yylex();
         if(parseFactor())
         {
            if(theValueStack.size() > 1)
            {
               rspfEquValue v;
            
               rspfEquValue v2 = theValueStack.top();
               theValueStack.pop();
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
            
               applyOp(rspfBinaryOpLessOrEqual(),
                       v,
                       v1,
                       v2);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
            if(result)
            {
               result = parseRestOfTerm();
            }
         }
         else
         {
            result = false;
         }

         break;
      }
      case RSPF_EQU_TOKEN_BDIFFERENT:
      {
         theCurrentId = theLexer->yylex();
         if(parseFactor())
         {
            if(theValueStack.size() > 1)
            {
               rspfEquValue v;
            
               rspfEquValue v2 = theValueStack.top();
               theValueStack.pop();
               rspfEquValue v1 = theValueStack.top();
               theValueStack.pop();
            
               applyOp(rspfBinaryOpDifferent(),
                       v,
                       v1,
                       v2);
            
               theValueStack.push(v);
            }
            else
            {
               result = false;
            }
            if(result)
            {
               result = parseRestOfTerm();
            }
         }
         else
         {
            result = false;
         }

         break;
      }

   }
   
   return result;
}

bool rspfEquationCombiner::parseTerm()
{
   // parse the following rule:
   //
   // Term : Factor RestOfTerm
   
   bool result = false;

   result = parseFactor();
   
   if(result)
   {
      result = parseRestOfTerm();
   }
   
   return result;
}

bool rspfEquationCombiner::parseRestOfExp()
{
   // parse the following rule:
   // RestOfExpression : + Term RestOfExpression | - Term RestOfExpression | epsilon
   //
   bool result = true;
   
   if(theCurrentId == RSPF_EQU_TOKEN_PLUS)
   {
      theCurrentId = theLexer->yylex();
      if(parseTerm())
      {

         if(theValueStack.size() > 1)
         {
            rspfEquValue v;
            
            rspfEquValue v2 = theValueStack.top();
            theValueStack.pop();
            rspfEquValue v1 = theValueStack.top();
            theValueStack.pop();
            
            
            applyOp(rspfBinaryOpAdd(),
                    v,
                    v1,
                    v2);              
            
            theValueStack.push(v);
         }
         else
         {
            result = false;
         }
         if(result)
         {
            result = parseRestOfExp();
         }
      }
      else
      {
        result =  false;
      }
   }
   else if(theCurrentId == RSPF_EQU_TOKEN_MINUS)
   {
      theCurrentId = theLexer->yylex();
      if(parseTerm())
      {
         if(theValueStack.size() > 1)
         {
            rspfEquValue v;
            
            rspfEquValue v2 = theValueStack.top();
            theValueStack.pop();
            rspfEquValue v1 = theValueStack.top();
            theValueStack.pop();
            
            applyOp(rspfBinaryOpSub(),
                    v,
                    v1,
                    v2);
            
            theValueStack.push(v);
         }
         else
         {
            result = false;
         }
         if(result)
         {
            result = parseRestOfExp();
         }
      }
      else
      {
         result =  false;
      }
   }
   
   return result;
}

rspfRefPtr<rspfImageData> rspfEquationCombiner::getImageData(rspf_uint32 index)
{
   rspfRefPtr<rspfImageData> result;
   rspfConnectableObject* obj = getInput(index);
   
   if(obj)
   {
      theCastFilter->connectMyInputTo(0, obj);
      result= (theCastFilter->getTile(theTile->getImageRectangle(),
                                      theCurrentResLevel));
      
      if(result.valid())
      {
         result->setMinPix(theTile->getMinPix(), theTile->getNumberOfBands());
         result->setMaxPix(theTile->getMaxPix(), theTile->getNumberOfBands());
      }
   }

   return result;
}

rspfRefPtr<rspfImageData> rspfEquationCombiner::getNewImageData(
   rspf_uint32 index)
{
   rspfRefPtr<rspfImageData> result = getImageData(index);

   if(result.valid())
   {
      if(result->getBuf())
      {
         result = (rspfImageData*)result->dup();
      }
   }

   return result;
}

bool rspfEquationCombiner::parseExpression()
{
   // parse the following rule:
   // expression : Term ResOfExpression
   //
   
   bool result = false;
   
   if(parseTerm())
   {
      result =  parseRestOfExp();
   }

   return result;
}

rspfRefPtr<rspfImageData> rspfEquationCombiner::parseEquation()
{
   ostringstream s;

   s << theEquation;

   istringstream inS(s.str());
   theLexer->switch_streams(&inS, &rspfNotify(rspfNotifyLevel_WARN));

   theCurrentId = theLexer->yylex();

   while(theCurrentId)
   {
      if(!parseExpression())
      {
         break;
      }
   }

   if(!theValueStack.empty())
   {
      assignValue();
      theTile->validate();
      clearStacks();
   }
   
   return theTile;
}    

bool rspfEquationCombiner::applyClamp(rspfImageData* &result,
                                       const vector<rspfEquValue>& argList)
{
   if(result)
   {
      // Delete the object indirectly through an rspfRefPtr.
      rspfRefPtr<rspfImageData> id = result;
      id = NULL;
      result = (rspfImageData*) NULL;
   }
   if(argList.size() <3)
   {
      return false;
   }

   if(argList[0].type == RSPF_EQU_DOUBLE_TYPE)
   {
      return false;
   }
   else if( (argList[1].type == RSPF_EQU_DOUBLE_TYPE)&&
            (argList[2].type == RSPF_EQU_DOUBLE_TYPE))
   {
      result = argList[0].d.imageDataValue;
      
      if(argList[0].d.imageDataValue)
      {
         rspfDataObjectStatus status = result->getDataObjectStatus();
         if((status != RSPF_NULL) &&
            (status != RSPF_EMPTY))
         {
            double minValue = argList[1].d.doubleValue;
            double maxValue = argList[2].d.doubleValue;
            
            if(minValue > maxValue)
            {
               std::swap(minValue, maxValue);
            }
            
            int band   = 0;
            int offset = 0;
            
            int upperBoundBand   = result->getNumberOfBands();
            int offsetUpperBound = result->getWidth()*result->getHeight();

            if(status == RSPF_PARTIAL)
            {
               for(band = 0; band < upperBoundBand; ++band)
               {
                  double np = static_cast<double>(result->getNullPix(band));
                  double *buf = static_cast<double*>(result->getBuf(band));
                  for(offset = 0; offset < offsetUpperBound; ++ offset)
                  {
                     if( *buf != np )
                     {
                        if( (*buf) < minValue) *buf = minValue;
                        else if( (*buf) >maxValue) *buf = maxValue;
                     }
                     ++buf;
                  }
               }
            }
            else
            {
               for(band = 0; band < upperBoundBand; ++band)
               {
                  double *buf = static_cast<double*>(result->getBuf(band));
                  for(offset = 0; offset < offsetUpperBound; ++ offset)
                  {
                     if( (*buf) < minValue) *buf = minValue;
                     else if( (*buf) >maxValue) *buf = maxValue;
                     ++buf;
                  }
               }
            }
         }
      }
         
      return true;
   }
   return false;
}

bool rspfEquationCombiner::applyConvolution(rspfImageData* &result,
                                             const vector<rspfEquValue>& argList)
{
   if(result)
   {
      // Delete the object indirectly through an rspfRefPtr.
      rspfRefPtr<rspfImageData> id = result;
      id = NULL;
      result = (rspfImageData*) NULL;
   }
   if(argList.size() <4) return false;
   
   for(rspf_uint32 i = 0; i < argList.size(); ++i)
   {
      if(argList[i].type != RSPF_EQU_DOUBLE_TYPE)
      {
         return false;
      }
   }

   rspf_uint32 index = (rspf_uint32)argList[0].d.doubleValue;
   int rows = (int)argList[1].d.doubleValue;
   int cols = (int)argList[2].d.doubleValue;
   
   if((rows*cols) != (int)(argList.size()-3))
   {
      return false;
   }
   
   NEWMAT::Matrix m(rows,cols);

    int count = 3;
    for(int r = 0; r< rows;++r)
    {
       for(int c=0;c<cols;++c)
       {
          m[r][c] = argList[count].d.doubleValue;
          ++count;
       }
    }
   rspfConnectableObject* obj = getInput(index);
   if(obj)
   {
      rspfRefPtr<rspfConvolutionSource> conv = new rspfConvolutionSource(NULL, m);

      conv->connectMyInputTo(0, obj);
      theCastFilter->connectMyInputTo(0, conv.get());
      
      rspfRefPtr<rspfImageData> tempData =
         theCastFilter->getTile(theTile->getImageRectangle(),
                                theCurrentResLevel);
      if(tempData.valid())
      {
         result = (rspfImageData*)tempData->dup();
      }
      else
      {
         result = (rspfImageData*)theTile->dup();
      }
      conv->disconnect();
      conv = 0;
   }
   if(result)
   {
      return true;
   }

   return false;
}

bool rspfEquationCombiner::applyBlurr(rspfImageData* &result,
                                       const vector<rspfEquValue>& argList)
{
   if(result)
   {
      // Delete the object indirectly through an rspfRefPtr.
      rspfRefPtr<rspfImageData> id = result;
      id = NULL;
      result = (rspfImageData*) NULL;
   }
   if(argList.size() !=3) return false;
   
   for(rspf_uint32 i = 0; i < argList.size(); ++i)
   {
      if(argList[i].type != RSPF_EQU_DOUBLE_TYPE)
      {
         return false;
      }
   }

   rspf_uint32 index = (rspf_uint32)argList[0].d.doubleValue;
   int rows = (int)argList[1].d.doubleValue;
   int cols = (int)argList[2].d.doubleValue;
   
   NEWMAT::Matrix m(rows, cols);

   m = 1.0/(rows*cols);

   rspfConnectableObject* obj = getInput(index);
   if(obj)
   {
      rspfRefPtr<rspfConvolutionSource> conv = new rspfConvolutionSource(NULL,
                                                                m);

      conv->connectMyInputTo(0, obj);
      theCastFilter->connectMyInputTo(0, conv.get());
      theCastFilter->initialize();

      rspfRefPtr<rspfImageData> tempData =
         theCastFilter->getTile(theTile->getImageRectangle(),
                                theCurrentResLevel);
      if(tempData.valid())
      {
         result = (rspfImageData*)tempData->dup();
      }
      conv->disconnect();
      conv = 0;
   }

   if(result)
   {
      return true;
   }

   return false;
}

bool rspfEquationCombiner::applyShift(rspfImageData* &result,
                                       const vector<rspfEquValue>& argList)
{
   if(result)
   {
      // Delete the object indirectly through an rspfRefPtr.
      rspfRefPtr<rspfImageData> id = result;
      id = NULL;
      result = (rspfImageData*) NULL;
   }
   if(argList.size() !=3) return false;
   
   for(rspf_uint32 i = 0; i < argList.size(); ++i)
   {
      if(argList[i].type != RSPF_EQU_DOUBLE_TYPE)
      {
         return false;
      }
   }

   rspf_uint32 index = (rspf_uint32)argList[0].d.doubleValue;
   int x = (int)argList[1].d.doubleValue;
   int y = (int)argList[2].d.doubleValue;

   rspfConnectableObject* obj = getInput(index);
   if(obj)
   {
      rspfRefPtr<rspfSubImageTileSource> shiftSource =
         new rspfSubImageTileSource(NULL, rspfIpt(x, y));

      shiftSource->connectMyInputTo(0, obj);
      theCastFilter->connectMyInputTo(0, shiftSource.get());

       rspfRefPtr<rspfImageData> tempData =
          theCastFilter->getTile(theTile->getImageRectangle(),
                                 theCurrentResLevel);
       if(tempData.valid())
       {
          result = (rspfImageData*)tempData->dup();
       }
      shiftSource->disconnect();
      shiftSource = 0;
   }

   if(result)
   {
      return true;
   }

   return false;
}

bool rspfEquationCombiner::applyOp(const rspfBinaryOp& op,
                                    rspfEquValue& result,
                                    rspfEquValue& v1,
                                    rspfEquValue& v2)
{
   bool returnValue = true;
   
   if(v1.type == RSPF_EQU_DOUBLE_TYPE)
   {
      if(v2.type == RSPF_EQU_DOUBLE_TYPE)
      {
         result.type = RSPF_EQU_DOUBLE_TYPE;
         result.d.doubleValue = op.apply(v1.d.doubleValue, v2.d.doubleValue);
      }
      else if(v2.type == RSPF_EQU_IMAGE_DATA_TYPE)
      {
         returnValue = applyOp(op,
                               v1.d.doubleValue,
                               v2.d.imageDataValue);
         
         result.type = RSPF_EQU_IMAGE_DATA_TYPE;
         result.d.imageDataValue = v2.d.imageDataValue;
      }
      else
      {
         returnValue = false;
      }
   }
   else if(v1.type == RSPF_EQU_IMAGE_DATA_TYPE)
   {
      if(v2.type == RSPF_EQU_DOUBLE_TYPE)
      {
         
         returnValue = applyOp(op,
                               v1.d.imageDataValue,
                               v2.d.doubleValue);
         
         result.type = RSPF_EQU_IMAGE_DATA_TYPE;
         result.d.imageDataValue = v1.d.imageDataValue;
         returnValue = true;
      }
      else if(v2.type == RSPF_EQU_IMAGE_DATA_TYPE)
      {
         returnValue = applyOp(op,
                               v1.d.imageDataValue,
                               v2.d.imageDataValue);
         result.type = RSPF_EQU_IMAGE_DATA_TYPE;
         result.d.imageDataValue = v1.d.imageDataValue;

         // Delete the object indirectly through an rspfRefPtr.
         rspfRefPtr<rspfImageData> id = v2.d.imageDataValue;
         id = NULL;
         v2.d.imageDataValue = (rspfImageData*)NULL;
         returnValue = true;
      }
      else
      {
         returnValue = false;
      }
   }
   else
   {
      returnValue = false;
   }
   
   return returnValue;
}

bool rspfEquationCombiner::applyOp(const rspfBinaryOp& op,
                                    rspfImageData* v1,
                                    double          v2)
{   
   double* buf = static_cast<double*>(v1->getBuf());
   if(!buf) return true;
   rspfDataObjectStatus status = v1->getDataObjectStatus();

   if(status == RSPF_EMPTY || status == RSPF_NULL)
   {
      return true;
   }

   if(status == RSPF_FULL )
   {
      rspf_uint32 size = v1->getSize();
      double value = (static_cast<double>(v2));
     
      for(rspf_uint32 i = 0; i < size; ++i)
      {
         *buf = (double)op.apply(*buf, value);
         ++buf;
      }
   }
   else
   {
      rspf_uint32 sizePerBand = v1->getSizePerBand();
      rspf_uint32 numberOfBands = v1->getNumberOfBands();

      if(numberOfBands)
      {
         for(rspf_uint32 band = 0; band < numberOfBands; ++band)
         {
            double* buf = static_cast<double*>(v1->getBuf(band));

            if(buf)
            {
               double  np  = static_cast<double>(v1->getNullPix()[band]);
               
               for(rspf_uint32 offset = 0; offset < sizePerBand;++offset)
               {
                  if(*buf != np)
                  {
                     *buf = (double)op.apply(*buf, v2);
                  }
                  ++buf;
               }
            }
         }
      }
   }
   return true;
}

bool rspfEquationCombiner::applyOp(const rspfBinaryOp& op,
                                    double          v1,
                                    rspfImageData* v2)
{   
   double* buf = static_cast<double*>(v2->getBuf());
   if(!buf) return true;
   rspfDataObjectStatus status = v2->getDataObjectStatus();

   if(status == RSPF_EMPTY || status == RSPF_NULL)
   {
      return true;
   }

   if(status == RSPF_FULL )
   {
      rspf_uint32 size = v2->getSize();
      double value = (static_cast<double>(v1));
     
      for(rspf_uint32 i = 0; i < size; ++i)
      {
         *buf = (double)op.apply(value, *buf);
         ++buf;
      }
   }
   else
   {
      rspf_uint32 sizePerBand = v2->getSizePerBand();
      rspf_uint32 numberOfBands = v2->getNumberOfBands();

      if(numberOfBands)
      {
         for(rspf_uint32 band = 0; band < numberOfBands; ++band)
         {
            double* buf = static_cast<double*>(v2->getBuf(band));

            if(buf)
            {
               double  np  = static_cast<double>(v2->getNullPix()[band]);
               
               for(rspf_uint32 offset = 0; offset < sizePerBand; ++offset)
               {
                  if(*buf != np)
                  {
                     *buf = (double)op.apply((double)v1, *buf);
                  }
                  ++buf;
               }
            }
         }
      }
   }
   
   return true;
}

bool rspfEquationCombiner::applyOp(const rspfBinaryOp& op,
                                    rspfImageData* v1,
                                    rspfImageData* v2)
{
   rspf_uint32 minNumberOfBands = std::min(v1->getNumberOfBands(), v2->getNumberOfBands());
   rspf_uint32 maxNumberOfBands = std::max(v1->getNumberOfBands(), v2->getNumberOfBands());
   
   rspf_uint32 size = v1->getWidth()*v1->getHeight();
   rspfDataObjectStatus status1 = v1->getDataObjectStatus();
   rspfDataObjectStatus status2 = v2->getDataObjectStatus();

   double** bandV1   = new double*[maxNumberOfBands];
   double** bandV2   = new double*[maxNumberOfBands];
   double* bandV1Np  = new double[maxNumberOfBands];
   double* bandV2Np  = new double[maxNumberOfBands];
   rspf_uint32 band = 0;
   for(band = 0; band < minNumberOfBands; ++band)
   {
      bandV1[band]   = (double*)v1->getBuf(band);
      bandV2[band]   = (double*)v2->getBuf(band);
      bandV1Np[band] = (double)v1->getNullPix(band);
      bandV2Np[band] = (double)v2->getNullPix(band);
   }
   if(v1->getNumberOfBands() < v2->getNumberOfBands())
   {
      for(band = 0; band < maxNumberOfBands; ++band)
      {
         bandV1[band]   = (double*)v1->getBuf(minNumberOfBands-1);
         bandV2[band]   = (double*)v2->getBuf(band);
         bandV1Np[band] = (double)v1->getNullPix(minNumberOfBands-1);
         bandV2Np[band] = (double)v2->getNullPix(band);
      }
   }
   else if(v2->getNumberOfBands() < v1->getNumberOfBands())
   {
      for(band = 0; band < maxNumberOfBands; ++band)
      {
         bandV1[band]   = (double*)v1->getBuf(band);
         bandV2[band]   = (double*)v2->getBuf(minNumberOfBands-1);
         bandV1Np[band] = (double)v1->getNullPix(band);
         bandV2Np[band] = (double)v2->getNullPix(minNumberOfBands-1);
      }
   }

   if(status1 == RSPF_EMPTY)
   {
      if(status2 == RSPF_FULL)
      {
         for(band = 0; band < maxNumberOfBands; ++band)
         {
            double* buf1 = bandV1[band];
            double* buf2 = bandV2[band];
            
            for(rspf_uint32 i = 0; i < size; ++i)
            {
               *buf1 = *buf2;
               ++buf1;
               ++buf2;
            }
         }
      }
      else if(status2 == RSPF_PARTIAL)
      {
         for(band = 0; band < maxNumberOfBands; ++band)
         {
            double* buf1 = bandV1[band];
            double* buf2 = bandV2[band];
            double  nullPix2 = bandV2Np[band];
            for(rspf_uint32 i = 0; i < size; ++i)
            {
               if(*buf2 != nullPix2)
               {
                  *buf1 = *buf2;
               }
               ++buf1;
               ++buf2;
            }
         }
      }
      v1->setDataObjectStatus(status2);
   }
   else if((status1 == RSPF_FULL)&&
           (status2 == RSPF_FULL))
   { 
      for(band = 0; band < maxNumberOfBands; ++band)
      {
         double* buf1 = bandV1[band];
         double* buf2 = bandV2[band];
         
         for(rspf_uint32 i = 0; i < size; ++i)
         {
            *buf1 = op.apply(*buf1, *buf2);
            ++buf1;
            ++buf2;
         }
      }
   }
   else if((status1 == RSPF_FULL)&&
           (status2 == RSPF_PARTIAL))
   {
      for(band = 0; band < maxNumberOfBands; ++band)
      {
         double* buf1 = bandV1[band];
         double* buf2 = bandV2[band];
         double  nullPix2 = bandV2Np[band];
         for(rspf_uint32 i = 0; i < size; ++i)
         {
            if(*buf2 != nullPix2)
            {
               *buf1 = op.apply(*buf1, *buf2);
            }
               
            ++buf1;
            ++buf2;
         }
      }
   }
   else if((status1 == RSPF_PARTIAL)&&
           (status2 == RSPF_FULL))
   {
      for(band = 0; band < maxNumberOfBands; ++band)
      {
         double* buf1 = bandV1[band];
         double* buf2 = bandV2[band];
         double  nullPix1 = bandV1Np[band];
         for(rspf_uint32 i = 0; i < size; ++i)
         {
            if(*buf1 != nullPix1)
            {
               *buf1 = op.apply(*buf1, *buf2);
            }
               
            ++buf1;
            ++buf2;
         }
      }
   }
   else if((status1 == RSPF_PARTIAL)&&
           (status2 == RSPF_PARTIAL))
   {
      for(band = 0; band < maxNumberOfBands; ++band)
      {
         double* buf1 = bandV1[band];
         double* buf2 = bandV2[band];
         double  nullPix1 = bandV1Np[band];
         double  nullPix2 = bandV2Np[band];
         for(rspf_uint32 i = 0; i < size; ++i)
         {
            if((*buf1 != nullPix1)&&
               (*buf2 != nullPix2))
            {
               *buf1 = op.apply(*buf1, *buf2);
            }
               
            ++buf1;
            ++buf2;
         }
      }
   }

   delete [] bandV1;
   delete [] bandV2;
   delete [] bandV1Np;
   delete [] bandV2Np;
   
   return true;
}


bool rspfEquationCombiner::applyOp(const rspfUnaryOp& op,
                                    rspfEquValue& result,
                                    rspfEquValue& v)
{
   bool returnValue = true;
   
   if(v.type == RSPF_EQU_DOUBLE_TYPE)
   {
      result.type = RSPF_EQU_DOUBLE_TYPE;
      result.d.doubleValue = op.apply(v.d.doubleValue);
   }
   else if(v.type == RSPF_EQU_IMAGE_DATA_TYPE)
   {
      returnValue = applyOp(op,
                            v.d.imageDataValue);
      result.type = RSPF_EQU_IMAGE_DATA_TYPE;
      result.d.imageDataValue = v.d.imageDataValue;
      returnValue = true;
   }
   else
   {
      returnValue = false;
   }
   
   return returnValue;
}

bool rspfEquationCombiner::applyOp(const rspfUnaryOp& op,
                                    rspfImageData* v)
{   
   double* buf = static_cast<double*>(v->getBuf());
   if(!buf) return true;
   rspfDataObjectStatus status = v->getDataObjectStatus();

   if(status == RSPF_EMPTY || status == RSPF_NULL)
   {
      return true;
   }

   if(status == RSPF_FULL )
   {
      rspf_uint32 size = v->getSize();
     
      for(rspf_uint32 i = 0; i < size; ++i)
      {
         *buf = (double)op.apply(*buf);
         ++buf;
      }
   }
   else
   {
      rspf_uint32 sizePerBand   = v->getSizePerBand();
      rspf_uint32 numberOfBands = v->getNumberOfBands();

      if(numberOfBands)
      {
         for(rspf_uint32 band = 0; band < numberOfBands; ++band)
         {
            double* buf = static_cast<double*>(v->getBuf(band));

            if(buf)
            {
               double  np  = static_cast<double>(v->getNullPix()[band]);
               
               for(rspf_uint32 offset = 0; offset < sizePerBand;++offset)
               {
                  if(*buf != np)
                  {
                     *buf = (double)op.apply(*buf);
                  }
                  ++buf;
               }
            }
         }
      }
   }
   
   return true;
}

bool rspfEquationCombiner::saveState(rspfKeywordlist& kwl,
                                      const char* prefix)const
{
   rspfString outputScalarType =
      rspfScalarTypeLut::instance()->getEntryString(theOutputScalarType);
   
   kwl.add(prefix,
           EQUATION_KW,
           theEquation.c_str(),
           true);

   kwl.add(prefix,
           "output_scalar_type",
           outputScalarType.c_str(),
           true);
   
   return rspfImageCombiner::saveState(kwl,
                                        prefix);
}

bool rspfEquationCombiner::loadState(const rspfKeywordlist& kwl,
                                      const char* prefix)
{
   const char* equ    = kwl.find(prefix, EQUATION_KW);
   const char* scalar = kwl.find(prefix, "output_scalar_type");
   
   bool result = rspfImageCombiner::loadState(kwl,
                                               prefix);
   
   if(equ)
   {
      theEquation = equ;
   }

   if(scalar)
   {      
      setOutputScalarType(rspfScalarTypeLut::instance()->
                          getScalarTypeFromString(scalar));
   }

   return result;
}

