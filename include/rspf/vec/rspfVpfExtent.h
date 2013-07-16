/*
 * Copyright 1994, 1995 Vectaport Inc., Cartoactive Systems
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */
#ifndef rspfVpfExtent_HEADER
#define rspfVpfExtent_HEADER
#include <iostream>
using namespace std;
#include <rspf/vec/vpf.h>
class rspfVpfExtent
{
public:
   friend ostream& operator <<(ostream& out,
                               const rspfVpfExtent& data);
   
   rspfVpfExtent (double x1, double y1, double x2, double y2);
   rspfVpfExtent();
   rspfVpfExtent(const extent_type& etype);
   
   int contained (rspfVpfExtent& extent2);
   int completely_within (rspfVpfExtent& extent2);
   int within (double x, double y);
   rspfVpfExtent operator+ (const rspfVpfExtent& b)const;
protected:
   extent_type theExtentType;
};
#endif
