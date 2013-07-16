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
// and has a return "RT".
//
// See link for more detail description:
// http://www.tutok.sk/fastgl/callback.html
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfCallback1wRet_HEADER
#define rspfCallback1wRet_HEADER 1

/**
 * @class rspfCallback1wRet
 *
 * Usage example, note template params can be anything:
 *
 * Caller interface to register callback:
 *
 * @code
 * void registerCallback(rspfCallback1wRet<const rspfFilename&, bool>* cb);
 * @endcode
 *
 * Callee creation of call back mechanism:
 *
 * @code
 * class ProcessFileCB: public rspfCallback1wRet<const rspfFilename&, bool>
 * {
 * public:
 *    ProcessFileCB(
 *       rspfImageElevationDatabase* obj,
 *       bool (rspfImageElevationDatabase::*func)(const rspfFilename&))
 *       :
 *       m_obj(obj),
 *       m_func(func)
 *    {}
 *       
 *    virtual bool operator()(const rspfFilename& file) const
 *    {
 *       return ( m_obj->*m_func)(file);
 *    }
 * 
 * private:
 *    rspfImageElevationDatabase* m_obj;
 *    bool (rspfImageElevationDatabase::*m_func)(const rspfFilename& file);
 * };
 * @endcode
 *
 * Making/registering a callback.
 *
 * @code
 * rspfFileWalker* fw = new rspfFileWalker();
 *
 * rspfCallback1wRet<const rspfFilename&, bool>* cb =
 *    new ProcessFileCB(this, &rspfImageElevationDatabase::processFile);
 *
 * fw->registerProcessFileCallback(cb);
 *
 * @endcode
 */
template <class P1, class RT> class rspfCallback1wRet
{
public:
   rspfCallback1wRet(){}
   virtual ~rspfCallback1wRet(){}
   virtual RT operator()(P1 p1) const = 0;
};

#endif /* #ifndef rspfCallback1wRet_HEADER */
