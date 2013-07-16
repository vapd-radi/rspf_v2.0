//----------------------------------------------------------------------------
//
// File: rspfCallback2wRet.h
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Simple templated callback class that takes a two parameters(arguments) "P1"
// and has a return "RT".
//
// See link for more detail description:
// http://www.tutok.sk/fastgl/callback.html
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfCallback2wRet_HEADER
#define rspfCallback2wRet_HEADER 1

/**
 * @class rspfCallback2wRet
 *
 * Usage example, note template params can be anything:
 *
 * Caller interface to register callback:
 *
 * @code
 * void registerCallback(rspfCallback2wRet<const rspfFilename&, bool&, bool>* cb);
 * @endcode
 *
 * Callee creation of call back mechanism:
 *
 * @code
 * class ProcessFileCB: public rspfCallback2wRet<const rspfFilename&, bool&, bool>
 * {
 * public:
 *    ProcessFileCB(
 *       rspfImageElevationDatabase* obj,
 *       bool (rspfImageElevationDatabase::*func)(const rspfFilename&, bool&))
 *       :
 *       m_obj(obj),
 *       m_func(func)
 *    {}
 *       
 *    virtual bool operator()(const rspfFilename& file, bool& flag) const
 *    {
 *       return ( m_obj->*m_func)(file, flag);
 *    }
 * 
 * private:
 *    rspfImageElevationDatabase* m_obj;
 *    bool (rspfImageElevationDatabase::*m_func)(const rspfFilename& file, bool& flag);
 * };
 * @endcode
 *
 * Making/registering a callback.
 *
 * @code
 * rspfFileWalker* fw = new rspfFileWalker();
 *
 * rspfCallback2wRet<const rspfFilename&, bool&, bool>* cb =
 *    new ProcessFileCB(this, &rspfImageElevationDatabase::processFile);
 *
 * fw->registerProcessFileCallback(cb);
 *
 * @endcode
 */

template <class P1, class P2, class RT> class rspfCallback2wRet
{
public:
   rspfCallback2wRet(){}
   virtual ~rspfCallback2wRet(){}
   virtual RT operator()(P1 p1, P2 p2) const = 0;
};

#endif /* #ifndef rspfCallback2wRet_HEADER */
