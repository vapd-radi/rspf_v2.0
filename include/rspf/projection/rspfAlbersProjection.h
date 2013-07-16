#ifndef rspfAlbersProjection_HEADER
#define rspfAlbersProjection_HEADER
#include <rspf/projection/rspfMapProjection.h>
/*!
 *  A basic conic projection developed by H.C. Albers.  Classified as a 
 *  conic and equal area.  A graphic can be found at:  
 *  http://everest.hunter.cuny.edu/mp/pbander/albers.gif
 */
class RSPFDLLEXPORT rspfAlbersProjection : public rspfMapProjection
{
public:
   rspfAlbersProjection(const rspfEllipsoid& ellipsoid = rspfEllipsoid(),
                         const rspfGpt& origin = rspfGpt());
   
   /**
   * @param ellipsoid
   * @param origin latitude longitude in decimal degrees.
   * @param stdParallel1 First parallel in DEGREES.
   * @param stdParallel2 Second parallel in DEGREES.
   * @param falseEasting False easting in meters.
   * @param falseNorthing False northing in meters.
   */
   rspfAlbersProjection(const rspfEllipsoid& ellipsoid,
                         const rspfGpt& origin,
                         double stdParallel1,
                         double stdParallel2,
                         double falseEasting,
                         double falseNorthing);
   
   rspfObject *dup()const;
   virtual rspfGpt inverse_do(const rspfDpt &eastingNorthing,const rspfGpt &latLon)const;  
   virtual rspfDpt forward(const rspfGpt &worldPoint)    const;
   virtual rspfGpt inverse(const rspfDpt &projectedPoint)const;
   virtual void update();
   /*!
    * The passed in Argument is in degrees and will convert it to radians
    * for Geotrans. Update is then called so we can pre-compute paramters.
    */
   void setStandardParallel1(double degree);
   /*!
    * The passed in Argument is in degrees and will convert it to radians
    * for Geotrans. Update is then called so we can pre-compute paramters
    */
   void setStandardParallel2(double degree);
   /*!
    * Will allow you to set both parallels. both are passed in degrees.
    */
   void setStandardParallels(double parallel1Degree,
                             double prallel2Degree);
   /*!
    * SetFalseEasting.  The value is in meters.
    * Update is then called so we can pre-compute paramters
    */
   void setFalseEasting(double falseEasting);
   /*!
    * SetFalseNorthing.  The value is in meters.
    * Update is then called so we can pre-compute paramters
    */
   void setFalseNorthing(double falseNorthing);
   /*!
    * Sets both false easting and northing values.  The values are
    * expected to be in meters.
    * Update is then called so we can pre-compute paramters
    */
   void setFalseEastingNorthing(double falseEasting, double falseNorthing);
   /*!
    * Allows one to set all parameters for this projections.  parallels are in
    * degrees and easting northings are in meters.
    * Update is then called so we can pre-compute paramters.
    */
   void setParameters(double parallel1,
                      double parallel2,
                      double falseEasting,
                      double falseNorthing);
   /** @returns the first standard parallel in decimal degrees. */
   virtual double getStandardParallel1() const;
   /** @returns the second standard parallel in decimal degrees. */
   virtual double getStandardParallel2() const;
   
   void setDefaults();
   /*!
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool operator==(const rspfProjection& projection) const;
protected:
   virtual ~rspfAlbersProjection();
   mutable double Albers_a;                   /* Semi-major axis of ellipsoid in meters */
   mutable double Albers_f;           /* Flattening of ellipsoid */
   mutable double es;            /* Eccentricity of ellipsoid */
   mutable double es2;            /* Eccentricity squared         */
   mutable double C;                     /* constant c   */
   mutable double rho0;                 /* height above ellipsoid		*/
   mutable double n;                    /* ratio between meridians		*/
   mutable double Albers_a_OVER_n;      /* Albers_a / n */
   mutable double one_MINUS_es2;        /* 1 - es2 */
   mutable double two_es;               /* 2 * es */
   
/* Albers Projection Parameters */
   mutable double Albers_Origin_Lat;    /* Latitude of origin in radians     */
   mutable double Albers_Origin_Long;               /* Longitude of origin in radians    */
   mutable double Albers_Std_Parallel_1;
   mutable double Albers_Std_Parallel_2;
   mutable double Albers_False_Easting;
   mutable double Albers_False_Northing;
   
   mutable double Albers_Delta_Northing;
   mutable double Albers_Delta_Easting;
   
/*!
 * The function Set_Albers_Parameters receives the ellipsoid parameters and
 * projection parameters as inputs, and sets the corresponding state
 * variables.  If any errors occur, the error code(s) are returned by the function, 
 * otherwise ALBERS_NO_ERROR is returned.
 *
 *    a                 : Semi-major axis of ellipsoid, in meters   (input)
 *    f                 : Flattening of ellipsoid						        (input)
 *    Origin_Latitude   : Latitude in radians at which the          (input)
 *                          point scale factor is 1.0
 *    Central_Meridian  : Longitude in radians at the center of     (input)
 *                          the projection (central meridian)
 *    Std_Parallel_1    : First standard parallel                   (input)
 *    Std_Parallel_2    : Second standard parallel                  (input)
 *    False_Easting     : A coordinate value in meters assigned to the
 *                          central meridian of the projection.     (input)
 *    False_Northing    : A coordinate value in meters assigned to the
 *                          origin latitude of the projection       (input)
 */
   long Set_Albers_Parameters(double a,
                             double f,
                             double Origin_Latitude,
                             double Central_Meridian,
                             double Std_Parallel_1,
                             double Std_Parallel_2,
                             double False_Easting,
                             double False_Northing);
/*!
 * The function Get_Albers_Parameters returns the current ellipsoid
 * parameters, and Albers projection parameters.
 *
 *    a                 : Semi-major axis of ellipsoid, in meters   (output)
 *    f                 : Flattening of ellipsoid						        (output)
 *    Origin_Latitude   : Latitude in radians at which the          (output)
 *                          point scale factor is 1.0
 *    Central_Meridian  : Longitude in radians at the center of     (output)
 *                          the projection
 *    Std_Parallel_1    : First standard parallel                   (output)
 *    Std_Parallel_2    : Second standard parallel                  (output)
 *    False_Easting     : A coordinate value in meters assigned to the
 *                          central meridian of the projection.     (output)
 *    False_Northing    : A coordinate value in meters assigned to the
 *                          origin latitude of the projection       (output)
 */
  void Get_Albers_Parameters(double *a,
                             double *f,
                             double *Origin_Latitude,
                             double *Central_Meridian,
                             double *Std_Parallel_1,
                             double *Std_Parallel_2,
                             double *False_Easting,
                             double *False_Northing)const;
/*!
 * The function Convert_Geodetic_To_Albers converts geodetic (latitude and
 * longitude) coordinates to Albers projection (easting and northing)
 * coordinates, according to the current ellipsoid and Albers projection
 * parameters.  If any errors occur, the error code(s) are returned by the
 * function, otherwise ALBERS_NO_ERROR is returned.
 *
 *    Latitude          : Latitude (phi) in radians           (input)
 *    Longitude         : Longitude (lambda) in radians       (input)
 *    Easting           : Easting (X) in meters               (output)
 *    Northing          : Northing (Y) in meters              (output)
 */
  long Convert_Geodetic_To_Albers (double Latitude,
                                   double Longitude,
                                   double *Easting,
                                   double *Northing)const;
/*!
 * The function Convert_Albers_To_Geodetic converts Albers projection
 * (easting and northing) coordinates to geodetic (latitude and longitude)
 * coordinates, according to the current ellipsoid and Albers projection
 * coordinates.  If any errors occur, the error code(s) are returned by the
 * function, otherwise ALBERS_NO_ERROR is returned.
 *
 *    Easting           : Easting (X) in meters                  (input)
 *    Northing          : Northing (Y) in meters                 (input)
 *    Latitude          : Latitude (phi) in radians              (output)
 *    Longitude         : Longitude (lambda) in radians          (output)
 */
  long Convert_Albers_To_Geodetic(double Easting,
                                  double Northing,
                                  double *Latitude,
                                  double *Longitude)const;
   
/*
 * RTTI information defined in data_types/rtti.h
 */
TYPE_DATA
};
#endif
