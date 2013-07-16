#pragma ident "$Id: ComputeIonoModel.hpp 2496 2011-01-14 16:18:08Z susancummins $"

/**
 * @file ComputeIonoModel.hpp
 * This is a class to compute the main values related to a given
 * GNSS ionospheric model.
 */

#ifndef GPSTK_COMPUTE_IONO_MODEL_HPP
#define GPSTK_COMPUTE_IONO_MODEL_HPP

//============================================================================
//
//  This file is part of GPSTk, the GPS Toolkit.
//
//  The GPSTk is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 2.1 of the License, or
//  any later version.
//
//  The GPSTk is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with GPSTk; if not, write to the Free Software Foundation,
//  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Wei Yan - Chinese Academy of Sciences . 2009, 2010, 2011
//
//============================================================================



#include "ProcessingClass.hpp"
#include "IonexStore.hpp"
#include "IonoModelStore.hpp"


namespace gpstk
{

      /** @addtogroup GPSsolutions */
      //@{

      /** This is a class to compute the main values related to a given
       *  GNSS ionospheric model.
       *
       * This class is intended to be used with GNSS Data Structures (GDS).
       * It is a more modular alternative to classes such as ModelObs
       * and ModelObsFixedStation.
       *
       * A typical way to use this class follows:
       *
       * @code
       *      // Input observation file stream
       *   RinexObsStream rin("ebre0300.02o");
       *
       *      // Now, create the ComputeTropModel object
       *   ComputeIonoModel computeIono(nominalPosition);
       *   computeTropo.setIonosphereMap(ionexFile);
       *
       *   gnssRinex gRin;  // GNSS data structure for fixed station data
       *
       *   while(rin >> gRin)
       *   {
       *         // Apply the ionoospheric model on the GDS
       *      gRin >> computeIono;
       *   }
       *
       * @endcode
       *
       * The "ComputeIonoModel" object will visit every satellite in
       * the GNSS data structure that is "gRin" and will try to compute
       * the main values of the corresponding ionospheric model.
       *
       * When used with the ">>" operator, this class returns the same
       * incoming data structure with the extra data inserted along their
       * corresponding satellites.
       *
       * Be warned that if a given satellite does not have the information
       * needed (mainly elevation), it will be summarily deleted from the data
       * structure. 
       *
       */
   class ComputeIonoModel : public ProcessingClass
   {
   public:
      enum IonoModelType
      {
         Zero = 0,     ///< Don't do ionospheric delay correction
         Klobuchar,    ///< Klobuchar
         GridTec      ///< Ionospheric maps
      };

   public:

         /// Default constructor.
      ComputeIonoModel()
         : ionoType(Zero), nominalPos(0.0,0.0,0.0)
      { setIndex(); };


         /** Common constructor
          *
          * @param stapos    Nominal position of receiver station.
          */
      ComputeIonoModel(const Position& stapos) 
         : ionoType(Zero), nominalPos(stapos)
      { setIndex(); };


         /** Returns a satTypeValueMap object, adding the new data generated
          *  when calling a modeling object.
          *
          * @param time      Epoch.
          * @param gData     Data object holding the data.
          */
      virtual satTypeValueMap& Process( const DayTime& time,
                                        satTypeValueMap& gData )
         throw(ProcessingException);


         /** Returns a gnnsSatTypeValue object, adding the new data generated
          *  when calling a modeling object.
          *
          * @param gData    Data object holding the data.
          */
      virtual gnssSatTypeValue& Process(gnssSatTypeValue& gData)
         throw(ProcessingException)
      { Process(gData.header.epoch, gData.body); return gData; };


         /** Returns a gnnsRinex object, adding the new data generated when
          *  calling a modeling object.
          *
          * @param gData    Data object holding the data.
          */
      virtual gnssRinex& Process(gnssRinex& gData)
         throw(ProcessingException)
      { Process(gData.header.epoch, gData.body); return gData; };


      virtual ComputeIonoModel& setKlobucharModel(const double a[4], 
                                                  const double b[4]);

      virtual ComputeIonoModel& setIonosphereMap(const std::string& ionexFile);


         /// Returns nominal position of receiver station.
      virtual Position getNominalPosition(void) const
      { return nominalPos; };


         /** Sets nominal position of receiver station.
          * @param stapos    Nominal position of receiver station.
          */
      virtual ComputeIonoModel& setNominalPosition(const Position& stapos)
        { nominalPos = stapos; return (*this); };

         /// Returns an index identifying this object.
      virtual int getIndex(void) const;


         /// Returns a string identifying this object.
      virtual std::string getClassName(void) const;


         /// Destructor.
      virtual ~ComputeIonoModel() {};


   private:

      IonoModelType  ionoType;

         /// Object to calculate ionospheric delay with klobuchar model
      IonoModelStore klbStore;

         /// Object to calculate ionospheric delay with ionospheric map model
      IonexStore gridStore;

         /// Receiver position
      Position nominalPos;


         /// Initial index assigned to this class.
      static int classIndex;


         /// Index belonging to this object.
      int index;


         /// Sets the index and increment classIndex.
      void setIndex(void)
      { index = classIndex++; };


   }; // End of class 'ComputeIonoModel'

      //@}

} // End of namespace gpstk

#endif   // GPSTK_COMPUTE_IONO_MODEL_HPP
