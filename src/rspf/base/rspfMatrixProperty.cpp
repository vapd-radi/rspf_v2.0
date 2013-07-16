//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfMatrixProperty.cpp 17074 2010-04-14 18:49:11Z dburken $
#include <sstream>
#include <rspf/base/rspfMatrixProperty.h>
#include <rspf/base/rspfCommon.h>

RTTI_DEF1(rspfMatrixProperty, "rspfMatrixProperty", rspfProperty);

rspfMatrixProperty::rspfMatrixProperty(const rspfString& name,
                                         const std::vector<double>& values,
                                         int numberOfRows,
                                         int numberOfCols)
   :rspfProperty(name),
    theMinNumberOfCols(-1),
    theMaxNumberOfCols(-1),
    theMinNumberOfRows(-1),
    theMaxNumberOfRows(-1)
{
   resize(numberOfRows, numberOfCols);

   if((int)values.size() == numberOfRows*numberOfCols)
   {
      int rowIdx = 0;
      int colIdx = 0;
      int linearIdx = 0;
      for(rowIdx = 0; rowIdx < getNumberOfRows(); ++rowIdx)
      {
         for(colIdx = 0; colIdx < getNumberOfCols(); ++colIdx)
         {
            theValueArray[rowIdx][colIdx] = values[linearIdx];
            ++linearIdx;
         }
      }
   }
}


rspfMatrixProperty::rspfMatrixProperty(const rspfMatrixProperty& rhs)
   :rspfProperty(rhs),
    theValueArray(rhs.theValueArray),
    theMinNumberOfCols(rhs.theMinNumberOfCols),
    theMaxNumberOfCols(rhs.theMaxNumberOfCols),
    theMinNumberOfRows(rhs.theMaxNumberOfCols),
    theMaxNumberOfRows(rhs.theMaxNumberOfRows)
{
}

rspfMatrixProperty::~rspfMatrixProperty()
{
   
}

rspfObject* rspfMatrixProperty::dup()const
{
   return new rspfMatrixProperty(*this);
}


const rspfProperty& rspfMatrixProperty::assign(const rspfProperty& rhs)
{
   rspfMatrixProperty* rhsPtr = PTR_CAST(rspfMatrixProperty,
                                          &rhs);
   if(rhsPtr)
   {
      theValueArray      = rhsPtr->theValueArray;
      theMinNumberOfCols = rhsPtr->theMinNumberOfCols;
      theMaxNumberOfCols = rhsPtr->theMaxNumberOfCols;
      theMinNumberOfRows = rhsPtr->theMinNumberOfRows;
      theMaxNumberOfRows = rhsPtr->theMaxNumberOfRows;
   }

   return rspfProperty::assign(rhs);
}

bool rspfMatrixProperty::setValue(const rspfString& value)
{
   std::istringstream in(value);
   
   rspfString numberOfRows;
   rspfString numberOfCols;
   rspfString tempValue;
   
   in>>numberOfRows >> numberOfCols;

   if(!in.bad())
   {
      resize(numberOfRows.toInt32(), numberOfCols.toInt32());

      int rowIdx = 0;
      int colIdx = 0;
      for(rowIdx = 0; ((rowIdx < getNumberOfRows())&&(!in.bad())); ++rowIdx)
      {
         for(colIdx = 0; ((colIdx < getNumberOfCols())&&(!in.bad()));++ colIdx)
         {
            in >> tempValue;
            
            theValueArray[rowIdx][colIdx] = tempValue.toDouble();
         }
      }
   }

   return !in.bad();
}

void rspfMatrixProperty::valueToString(rspfString& valueResult)const
{
   if(theValueArray.size()>0)
   {
      if(theValueArray[0].size() > 0)
      {
         valueResult = "";
         
         int rowIdx = 0;
         int colIdx = 0;
         valueResult += rspfString::toString(getNumberOfRows()) + " " +
                        rspfString::toString(getNumberOfCols()) + " ";
         for(rowIdx = 0; rowIdx < getNumberOfRows();++rowIdx)
         {
            for(colIdx = 0; colIdx < getNumberOfCols(); ++colIdx)
            {
               valueResult += rspfString::toString(theValueArray[rowIdx][colIdx]);
               valueResult += " ";
            }
         }
      }
   }
}

void rspfMatrixProperty::resize(int numberOfRows,
                                 int numberOfCols)
{
   int tempNumberOfRows = numberOfRows;
   int tempNumberOfCols = numberOfCols;

   if(theMinNumberOfCols > 0)
   {
      if(tempNumberOfCols < theMinNumberOfCols)
      {
         tempNumberOfCols = theMinNumberOfCols;
      }
   }
   if(theMinNumberOfRows > 0)
   {
      if(tempNumberOfRows < theMinNumberOfRows)
      {
         tempNumberOfRows = theMinNumberOfRows;
      }
   }
   if(theMaxNumberOfCols > 0)
   {
      if(tempNumberOfCols > theMaxNumberOfCols)
      {
         tempNumberOfCols = theMaxNumberOfCols;
      }
   }
   if(theMaxNumberOfRows > 0)
   {
      if(tempNumberOfRows > theMaxNumberOfRows)
      {
         tempNumberOfRows = theMaxNumberOfRows;
      }
   }

   int currentNumberOfRows = getNumberOfRows();
   int currentNumberOfCols = getNumberOfCols();
   
   if((currentNumberOfRows != tempNumberOfRows) ||
      (currentNumberOfCols != tempNumberOfCols))
   {
      std::vector< std::vector<double> > tempValue = theValueArray;
      
      theValueArray.resize(tempNumberOfRows);

      int minCols = rspf::min((int)currentNumberOfCols,
                             (int)tempNumberOfCols);
      int rowIdx = 0;
      int colIdx = 0;
      
      for(rowIdx = 0 ; rowIdx < tempNumberOfRows; ++rowIdx)
      {
         theValueArray[rowIdx].resize(tempNumberOfCols);
         std::fill(theValueArray[rowIdx].begin(), theValueArray[rowIdx].end(), 0);

         if(tempNumberOfRows < currentNumberOfRows)
         {
            for(colIdx = 0 ; colIdx < minCols; ++colIdx)
            {
               theValueArray[rowIdx][colIdx] = tempValue[rowIdx][colIdx];
            }
         }
      }
   }
}

double& rspfMatrixProperty::operator()(int rowIdx,
                                        int colIdx)
{
   return theValueArray[rowIdx][colIdx];
}

const double& rspfMatrixProperty::operator()(int rowIdx,
                                              int colIdx)const
{
   return theValueArray[rowIdx][colIdx];
}

int rspfMatrixProperty::getNumberOfRows()const
{
   return (int)theValueArray.size();
}

int rspfMatrixProperty::getNumberOfCols()const
{
   if(getNumberOfRows())
   {
      return (int)theValueArray[0].size();
   }

   return 0;
}

void rspfMatrixProperty::clearConstraints()
{
   theMinNumberOfCols = -1;
   theMaxNumberOfCols = -1;
   theMinNumberOfRows = -1;
   theMaxNumberOfRows = -1;
}

void rspfMatrixProperty::setColConstraints(int minNumberOfCols,
                                            int maxNumberOfCols)
{
   theMinNumberOfCols = minNumberOfCols;
   theMaxNumberOfCols = maxNumberOfCols;
}

void rspfMatrixProperty::setRowConstraints(int minNumberOfRows,
                                            int maxNumberOfRows)
{
   theMinNumberOfRows = minNumberOfRows;
   theMaxNumberOfRows = maxNumberOfRows;
}

void rspfMatrixProperty::getColConstraints(int& minNumberOfCols,
                                            int& maxNumberOfCols) const
{
   minNumberOfCols = theMinNumberOfCols;
   maxNumberOfCols = theMaxNumberOfCols;
}

void rspfMatrixProperty::getRowConstraints(int& minNumberOfRows,
                                            int& maxNumberOfRows) const
{
   minNumberOfRows = theMinNumberOfRows;
   maxNumberOfRows = theMaxNumberOfRows;
}

void rspfMatrixProperty::normalize()
{
   double densityValue = density();

   if(fabs(densityValue) <= DBL_EPSILON)
   {
      return;
   }
   int rowIdx = 0;
   int colIdx = 0;
   for(rowIdx = 0 ; rowIdx < (int)theValueArray.size(); ++rowIdx)
   {
      for(colIdx = 0 ; colIdx < (int)theValueArray[rowIdx].size(); ++colIdx)
      {
         theValueArray[rowIdx][colIdx]/=densityValue;
      }
   }
   
}

void rspfMatrixProperty::zero()
{
   int rowIdx = 0;
   int colIdx = 0;
   for(rowIdx = 0 ; rowIdx < (int)theValueArray.size(); ++rowIdx)
   {
      for(colIdx = 0 ; colIdx < (int)theValueArray[rowIdx].size(); ++colIdx)
      {
         theValueArray[rowIdx][colIdx] = 0.0;
      }
   }
}

double rspfMatrixProperty::density()const
{
   double result = 0.0;

   if((getNumberOfRows() > 0)&&
      (getNumberOfCols() > 0))
   {
      int rowIdx = 0;
      int colIdx = 0;
      
      for(rowIdx = 0 ; rowIdx < (int)theValueArray.size(); ++rowIdx)
      {
         for(colIdx = 0 ; colIdx < (int)theValueArray[rowIdx].size(); ++colIdx)
         {
            result += theValueArray[rowIdx][colIdx];
         }
      }
   }

   return result;
}
