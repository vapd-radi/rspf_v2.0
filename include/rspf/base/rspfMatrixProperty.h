//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfMatrixProperty.h 17074 2010-04-14 18:49:11Z dburken $
#ifndef rspfMatrixProperty_HEADER
#define rspfMatrixProperty_HEADER
#include <vector>
#include <rspf/base/rspfProperty.h>


class RSPFDLLEXPORT rspfMatrixProperty : public rspfProperty
{
public:
   rspfMatrixProperty(const rspfString& name=rspfString(""),
                       const std::vector<double>& values = std::vector<double>(),
                       int numberOfRows    = 0,
                       int numberOfColumns = 0);
   rspfMatrixProperty(const rspfMatrixProperty& rhs);

   virtual ~rspfMatrixProperty();

   virtual rspfObject* dup()const;
   virtual const rspfProperty& assign(const rspfProperty& rhs);
  virtual bool setValue(const rspfString& value);
   virtual void valueToString(rspfString& valueResult)const;
   
   void resize(int numberOfRows,
               int numberOfColumns);

   double& operator ()(int rowIdx,
                       int colIdx);
   
   const double& operator()(int rowIdx,
                            int colIdx)const;

   int getNumberOfRows()const;
   int getNumberOfCols()const;

   void clearConstraints();
   
   /*!
    * If any of the constraints are set to something < 0 then it means there
    * isn't a constraint for that value.  So if you give a min value of -1
    * and a max value of 10 then the column can grow from 10 downto 0.
    */
   void setColConstraints(int minNumberOfColumns,
                          int maxNumberOfColumns);

   /*!
    * If any of the contraints are set to something < 0 then it means there
    * isn't a constraint for that value.  So if you give a min value of -1
    * and a max value of 10 then the row can grow from 10 downto 0.
    */
   void setRowConstraints(int minNumberOfRows,
                          int maxNumberOfRows);

   void getColConstraints(int& minNumberOfColumns,
                          int& maxNumberOfColumns) const;
   
   void getRowConstraints(int& minNumberOfRows,
                          int& maxNumberOfRows) const;

   void normalize();
   void zero();
   
protected:
   std::vector< std::vector<double> > theValueArray;
   int theMinNumberOfCols;
   int theMaxNumberOfCols;
   int theMinNumberOfRows;
   int theMaxNumberOfRows;

   double density()const;
TYPE_DATA
};

#endif
