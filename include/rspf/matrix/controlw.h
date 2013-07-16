#ifndef CONTROL_WORD_LIB
#define CONTROL_WORD_LIB 0
class ControlWord
{
protected:
   int cw;                                      // the control word
public:
   ControlWord() : cw(0) {}                     // do nothing
   ControlWord(int i) : cw(i) {}                // load an integer
   ControlWord operator*(ControlWord i) const
      { return ControlWord(cw & i.cw); }
   void operator*=(ControlWord i)  { cw &= i.cw; }
   ControlWord operator+(ControlWord i) const
      { return ControlWord(cw | i.cw); }
   void operator+=(ControlWord i)  { cw |= i.cw; }
   ControlWord operator-(ControlWord i) const
      { return ControlWord(cw - (cw & i.cw)); }
   void operator-=(ControlWord i) { cw -= (cw & i.cw); }
   bool operator>=(ControlWord i) const { return (cw & i.cw) == i.cw; }
   bool operator<=(ControlWord i) const { return (cw & i.cw) == cw; }
   ControlWord operator^(ControlWord i) const
      { return ControlWord(cw ^ i.cw); }
   ControlWord operator~() const { return ControlWord(~cw); }
   int operator+() const { return cw; }
   int operator!() const { return cw==0; }
   FREE_CHECK(ControlWord)
};
#endif
