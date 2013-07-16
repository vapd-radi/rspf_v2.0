//*****************************************************************************
//                                   O S S I M
//
// FILE: rspfDblGrid.h
//
// License:  See top level LICENSE.txt file.
//
// AUTHOR:  Oscar Kramer
//
// DESCRIPTION:
//   Contains declaration of class rspfDblGrid. This object class maintains
//   a regular grid of floating point (double) values. Access methods to the
//   grid include interpolation between grid nodes. Capability is included
//   to address the grid in an arbitrary, floating-point u, v coordinate
//   system which is related to the integer grid x, y system by scale and
//   offset:
//
//          u = Sx*(x + Xo),   v = Sy*(y + Y0)
//
//   where Xo, Yo is the origin of the U/V cood system. Sx, Sy are the scale
//   relations. For example, if the grid is being used to hold a grid of
//   elevation posts, then the origin will be the SW corner post ground point,
//   and the scale will be derived from the post spacing. Both origin and
//   spacing are specified to the constructor.
//
//*****************************************************************************
//  $Id: rspfDblGrid.h 20044 2011-09-06 15:02:43Z oscarkramer $

#ifndef rspfDblGrid_HEADER
#define rspfDblGrid_HEADER

#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfCommon.h>

class rspfDrect;

/*!****************************************************************************
 *
 * CLASS:  rspfDblGrid
 *
 *****************************************************************************/
class RSPFDLLEXPORT rspfDblGrid 
{
public:
   /*!
    * Default Constructor.
    */
   rspfDblGrid ();

   /*!
    * Copy Constructor.
    */
   rspfDblGrid (const rspfDblGrid& source_to_copy);

   /*!
    * Constructs given a U/V origin and spacing, and a grid size.
    */
   rspfDblGrid (const rspfIpt&  size, 
                 const rspfDpt&  origin,
                 const rspfDpt&  spacing,
                 double null_value = RSPF_DEFAULT_NULL_PIX_DOUBLE);

   /*!
    * Constructs given a rectangle in U/V space and grid spacing. The origin is
    * taken as the upper-left corner of the rectangle. The lower-rightmost grid
    * point may not coincide with the rectangles LR corner if the side is not
    * evenly divisible by the corresponding spacing.
    */
   rspfDblGrid (const rspfDrect&  uv_rect, 
                 const rspfDpt&    spacing,
                 double null_value = RSPF_DEFAULT_NULL_PIX_DOUBLE);

   ~rspfDblGrid ();

   /*!
    * When constructed with default, permits initialization afterward.
    */
   void         initialize(const rspfIpt&  size, 
                           const rspfDpt&  origin,
                           const rspfDpt&  spacing,
                           double null_value = RSPF_DEFAULT_NULL_PIX_DOUBLE);
   void         initialize(const rspfDrect&  uv_rect, 
                           const rspfDpt&    spacing,
                           double null_value = RSPF_DEFAULT_NULL_PIX_DOUBLE);
   /*!
   * Enables/disables extrapolation functionality. If extrapolation is enabled, then calls to 
   * operator() will handle points outside of the grid. 
   */
   void enableExtrapolation(bool arg=true) { theExtrapIsEnabled = arg;}

   void deallocate();
   /*!
    * Fills the current grid with the value specified.
    */
   void         fill(double fill_value);

   /*!
    * Fills the current grid with null values.
    */
   void         clear() { fill(theNullValue); }
   
   /*!
    * Access individual node value by node index in the X/Y (grid) coord system.
    */
   void         setNode (const rspfIpt& p,
                         const double& value)   { setNode(p.x, p.y, value); }
   void         setNode (int x, int y, const double&  value);

   double       getNode (const rspfIpt& p) const { return getNode(p.x, p.y); }
   double       getNode (int x, int y) const;

   /*!
    * Sets the node nearest the U,V point specified to the value given. This is
    * different from setNode() in that the UV coordinate system is used to
    * address a node instead of an XY grid point.
    */
   void         setNearestNode(const rspfDpt& uv_point,
                               const double&   value);
   
   /*!
    * This method performs a resampling of the defined grid nodes in order to
    * compute interpolated values for those uninitialized nodes. This is
    * necessary when only a subset of nodes are available for initializing the
    * grid.
    */
   void interpolateNullValuedNodes(const double& decay_rate=10.0);

   /*!
    * Passes the grid data through a convolution filter given the kernel array.
    * The grid must not contain any NULL nodes as these are treated as valid
    * quantities. The kernel sizes should be odd numbers. To avoid shrinking
    * the grid by the kernel radius, the edge nodes outside of the filter's
    * reach are computed by extrapolation. X is the contiguous
    * axis in the kernel array, i.e., kernel(x,y) = kernel[y*size_x + x].
    */
   void filter(int size_x, int size_y, double* kernel);
   
   /*!
    * Interpolate between grid nodes given arbitrary u/v coord system:
    */
   enum InterpType
   {
      BILINEAR
   };
   void  setInterpolationType (InterpType interp);

   enum DomainType
   {
      CONTINUOUS = 0, // Default continuous grid with no limits on values
      SAWTOOTH_90= 1, // For angles between -90 and 90 deg such as latitude
      WRAP_180   = 2, // For angles between -180 and 180 with discontinuity at limits (typically used for longitude)
      WRAP_360   = 3  // For angles between 0 and 360 with discontinuity at limits (typically used for rotations)
   };
   void setDomainType(DomainType dt) { theDomainType = dt; }

   /*!
    * Access an interpolated value in the U/V (output) coordinate system.
    */
   double operator() (const rspfDpt& uv_point) const {return (*this)(uv_point.u, uv_point.v);}
   double value      (const rspfDpt& uv_point) const {return (*this)(uv_point.u, uv_point.v);}

   double operator() (const double& u, const double& v) const;
   double value (const double& u, const double& v) const {return (*this)(u,v);}

   /*!
    * operator for initializing this grid with another.
    */
   const rspfDblGrid& operator = (const rspfDblGrid& grid);
   
   /*!
    *Statistics methods:
    */
   double minValue()   const { return theMinValue; }
   double maxValue()   const { return theMaxValue; }
   double nullValue()  const { return theNullValue; }
   double meanValue();
   double meanStdDev();

   void setMinValue(double value) {theMinValue = value;}
   void setMaxValue(double value) {theMaxValue = value;}
   void setNullValue(double value) {theNullValue = value;}
   /*!
    * Other member access methods:
    */
   const rspfIpt& size()    const { return theSize; }
   const rspfDpt& origin()  const { return theOrigin; }
   const rspfDpt& spacing() const { return theSpacing; }
   unsigned long   getSizeInBytes() const { return theSize.x*theSize.y*sizeof(double); }
   
   /*!
    * Returns true if double point lies within world space coverage:
    */
   bool  isInside(const rspfDpt& p) const {return isInside(p.u, p.v);}
   bool  isInside(const double& u, const double& v) const;

   /*!
    * Loads and saves the grid from/to a stream. Returns TRUE if successful.
    */
   bool  save(std::ostream& os, const char* descr) const;
   bool  load(std::istream& is);
   
   friend std::ostream& operator << (std::ostream& os, const rspfDblGrid& grid);
   
private:
   
   void  computeMean();
   double interpolate(double x, double y) const;
   double extrapolate(double x, double y) const;

   //! Constrains the value to the numerical domain specified in theDomainType.
   void constrain(double& value) const;

   rspf_uint32   index(int x, int y) const { return y*theSize.x + x; }
   
   double*      theGridData;
   rspfIpt     theSize;
   rspfDpt     theOrigin;
   rspfDpt     theSpacing;
   double       theMinValue;
   double       theMaxValue;
   double       theNullValue;
   double       theMeanValue;
   double       theDeviation;
   bool         theMeanIsComputed;
   bool         theExtrapIsEnabled;
   DomainType   theDomainType;
   
};

#endif

