//----------------------------------------------------------------------------
//
// File: rspfCallback1wRet.h
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Simple templated callback class that takes a single parameter(argument) "P1"
// with void return.
//
// See link for more detail description:
// http://www.tutok.sk/fastgl/callback.html
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfCallback1_HEADER
#define rspfCallback1_HEADER 1

/**
 * @class rspfCallback1
 *
 * Usage example, note template params can be anything:
 *
 * Caller interface to register callback:
 *
 * @code
 * void registerCallback(rspfCallback1<const rspfFilename&>* cb);
 * @endcode
 *
 * Callee creation of call back mechanism:
 *
 * @code
 * class ProcessFileCB: public rspfCallback1<const rspfFilename&>
 * {
 * public:
 *    ProcessFileCB(
 *       rspfImageElevationDatabase* obj,
 *       void (rspfImageElevationDatabase::*func)(const rspfFilename&))
 *       :
 *       m_obj(obj),
 *       m_func(func)
 *    {}
 *       
 *    virtual void operator()(const rspfFilename& file) const
 *    {
 *       ( m_obj->*m_func)(file);
 *    }
 * 
 * private:
 *    rspfImageElevationDatabase* m_obj;
 *    void (rspfImageElevationDatabase::*m_func)(const rspfFilename& file);
 * };
 * @endcode
 *
 * Making/registering a callback.
 *
 * @code
 * rspfFileWalker* fw = new rspfFileWalker();
 *
 * rspfCallback1wRet<const rspfFilename&>* cb =
 *    new ProcessFileCB(this, &rspfImageElevationDatabase::processFile);
 *
 * fw->registerProcessFileCallback(cb);
 *
 * @endcode
 */
template <class P1> class rspfCallback1
{
public:
   rspfCallback1(){}
   virtual ~rspfCallback1(){}
   virtual void operator()(P1 p1) const = 0;
};

#endif /* #ifndef rspfCallback1_HEADER */
