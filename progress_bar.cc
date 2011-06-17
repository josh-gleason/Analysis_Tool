#include "progress_bar.h"

ProgressBar::ProgressBar() :
  _out(&cout),
  _maxVal(1),
  _currentVal(-1),
  _width(10),
  _preMsg("")
{}

ProgressBar::ProgressBar( ostream& out, const string& preMsg, int maxVal,
  int width ) :
  _out(&out),
  _maxVal(maxVal),
  _currentVal(-1),
  _width(width),
  _preMsg(preMsg)
{}

void ProgressBar::update( const int& currentVal )
{
  if ( (int)(_width * ((float)currentVal / _maxVal)) == _currentVal
     && currentVal < _maxVal-1 )
    return;
  
  _currentVal = (int)(_width * ((float)currentVal / _maxVal));

  *_out << '\r' << _preMsg << '[';
  if ( currentVal >= _maxVal-1 && currentVal != -1 )
  {
    for ( int i = 0; i < _width; i++ )
      *_out << '*';
    *_out << "] Complete!" << endl;
  }
  else
  {
    for ( int i = 0; i < _width; i++ )
      *_out << ( i <= _currentVal && currentVal != -1 ? '*' : '-' );
    *_out << "] "
         << setw(5) << fixed << right << setprecision(0)
         << 100.0 * max(0.0,(double)currentVal) / (float)_maxVal << "%  "
         << '\r' << setprecision(-1) << flush;
  }
}

