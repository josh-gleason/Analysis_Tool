#ifndef PROGRESS_BAR
#define PROGRESS_BAR

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

class ProgressBar
{
  public:
    ProgressBar();
    ProgressBar(
      ostream& out,
      const string& preMsg = "",
      int maxVaL = 1,
      int width = 10
    );

    void update( const int& currentVal );
  protected:
    ostream* _out;
    int _maxVal;
    int _currentVal;
    int _width;
    string _preMsg;
};

#endif

