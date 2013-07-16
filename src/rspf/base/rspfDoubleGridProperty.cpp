#include <sstream>
#include <rspf/base/rspfDoubleGridProperty.h>

RTTI_DEF1(rspfDoubleGridProperty, "rspfDoubleGridProperty", rspfProperty);

rspfDoubleGridProperty::rspfDoubleGridProperty(const rspfString& name,
						 int /* numberOfRows */,
						 int /* numberOfCols */,
						 const std::vector<double>& /* values*/ )
  :rspfProperty(name),
   theMinNumberOfCols(-1),
   theMaxNumberOfCols(-1),
   theMinNumberOfRows(-1),
   theMaxNumberOfRows(-1)
{

}

rspfDoubleGridProperty::rspfDoubleGridProperty(const rspfDoubleGridProperty& rhs)
  :rspfProperty(rhs),
  theMinNumberOfCols(rhs.theMinNumberOfCols),
  theMaxNumberOfCols(rhs.theMaxNumberOfCols),
  theMinNumberOfRows(rhs.theMinNumberOfRows),
  theMaxNumberOfRows(rhs.theMaxNumberOfRows),
  theValues(rhs.theValues)
{
}

rspfDoubleGridProperty::~rspfDoubleGridProperty()
{
}

rspfObject* rspfDoubleGridProperty::dup()const
{
  return new rspfDoubleGridProperty(*this);
}

const rspfProperty& rspfDoubleGridProperty::assign(const rspfProperty& rhs)
{
  rspfProperty::assign(rhs);
  rspfDoubleGridProperty* rhsPtr = PTR_CAST(rspfDoubleGridProperty, &rhs);

  if(rhsPtr)
    {
      theMinNumberOfCols = rhsPtr->theMinNumberOfCols;
      theMaxNumberOfCols = rhsPtr->theMaxNumberOfCols;
      theMinNumberOfRows = rhsPtr->theMinNumberOfRows;
      theMaxNumberOfRows = rhsPtr->theMaxNumberOfRows;
      theValues          = rhsPtr->theValues;
    }
  else
    {
      setValue(rhs.valueToString());
      
    }
  
  return *this;
}

  
bool rspfDoubleGridProperty::setValue(const rspfString& value)
{
   std::istringstream in(value.c_str());
   rspfString nRows, nCols, v;
   int numberOfRows=0;
   int numberOfCols=0;
   int rowIdx = 0;
   int colIdx = 0;
   in >> nRows >> nCols;
   numberOfRows = nRows.toInt32();
   numberOfCols = nCols.toInt32();
   theValues.resize(numberOfRows);
   
   for(rowIdx = 0; rowIdx < numberOfRows; ++rowIdx)
   {
      theValues[rowIdx].resize(numberOfCols);
      for(colIdx = 0; colIdx < numberOfCols; ++ colIdx)
      {
         in >> v;
         theValues[rowIdx][colIdx] = v.toDouble();
      }
   }
   
   return true;
}

void rspfDoubleGridProperty::valueToString(rspfString& valueResult)const
{
  std::ostringstream out;
  int rowIdx = 0;
  int colIdx = 0;
  out << getNumberOfRows() << " " << getNumberOfCols() << " ";

  for(rowIdx = 0; rowIdx < (int)getNumberOfRows(); ++rowIdx)
    {
      for(colIdx = 0; colIdx < (int)getNumberOfCols(); ++colIdx)
	{
	  out << rspfString::toString(getValue(rowIdx, colIdx)) << " ";
	}
    }
  valueResult = out.str();
}

void rspfDoubleGridProperty::clearConstraints()
{
  theMinNumberOfCols = -1;
  theMaxNumberOfCols = -1;
  theMinNumberOfRows = -1;
  theMaxNumberOfRows = -1;
}

void rspfDoubleGridProperty::setColConstraints(int minNumberOfCols,
		  int maxNumberOfCols)
{
  theMinNumberOfCols = minNumberOfCols;
  theMaxNumberOfCols = maxNumberOfCols;
}

void rspfDoubleGridProperty::setRowConstraints(int minNumberOfRows,
						int maxNumberOfRows)
{
  theMinNumberOfRows = minNumberOfRows;
  theMaxNumberOfRows = maxNumberOfRows;
}

void rspfDoubleGridProperty::setContraints(int minNumberOfRows,
					    int maxNumberOfRows,
					    int minNumberOfCols,
					    int maxNumberOfCols)
{
  theMinNumberOfRows = minNumberOfRows;
  theMaxNumberOfRows = maxNumberOfRows;
  theMinNumberOfCols = minNumberOfCols;
  theMaxNumberOfCols = maxNumberOfCols;
}

rspf_uint32 rspfDoubleGridProperty::getNumberOfRows()const
{
  return ((int)theValues.size());
}

rspf_uint32 rspfDoubleGridProperty::getNumberOfCols()const
{
  if(getNumberOfRows())
    {
      return (rspf_uint32)theValues[0].size();
    }
  return 0;
}

double rspfDoubleGridProperty::getValue(rspf_uint32 row, 
					 rspf_uint32 col)const
{
  if((row < getNumberOfRows())&&
     (col < getNumberOfCols()))
    {
      return theValues[(int)row][(int)col];
    }

  return 0.0;
}
