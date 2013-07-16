//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfDoubleGridProperty.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfDoubleGridProperty_HEADER
#define rspfDoubleGridProperty_HEADER
#include <rspf/base/rspfProperty.h>

class RSPFDLLEXPORT rspfDoubleGridProperty : public rspfProperty
{
public:
  rspfDoubleGridProperty(const rspfString& name = rspfString(""),
			  int numberOfRows = 0,
			  int numberOfCols = 0,
			  const std::vector<double>& values = std::vector<double>());
  rspfDoubleGridProperty(const rspfDoubleGridProperty& rhs);
  
  virtual ~rspfDoubleGridProperty();

  rspfObject* dup()const;

  virtual const rspfProperty& assign(const rspfProperty& rhs);

  virtual bool setValue(const rspfString& value);
  virtual void valueToString(rspfString& valueResult)const;

  void clearConstraints();
  /*!
   * Setting both values to -1 will say no constraints.
   * if min is constrained and max not then it will not allow
   * the cols to grow any larger than the passed in max.
   *
   * If the min is -1 and max not then it will not exceed the 
   * max number of cols
   */
  void setColConstraints(int minNumberOfCols,
			 int maxNumberOfCols);

  void setRowConstraints(int minNumberOfRows,
			 int maxNumberOfRows);
  
  void setContraints(int minNumberOfRows,
		     int maxNumberOfRows,
		     int minNumberOfCols,
		     int maxNumberOfCols);

  rspf_uint32 getNumberOfRows()const;
  rspf_uint32 getNumberOfCols()const;

  double getValue(rspf_uint32 row, rspf_uint32 col)const;
protected:
  int theMinNumberOfCols;
  int theMaxNumberOfCols;
  int theMinNumberOfRows;
  int theMaxNumberOfRows;
  std::vector< std::vector<double> > theValues;
    

TYPE_DATA
};
#endif 
