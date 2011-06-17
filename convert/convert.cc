// convert original label (txt file) to x y width height
// where x,y are upper left corner and origin is in the upper left

// originally stored as <y x height width> with x,y as upper right and origin
// in upper right

#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{
  if ( argc < 3 )
  {
    cout << "Usage : ./" << argv[0] << " <input> <output>" << endl;
    return -1;
  }

  // y x height width

  ifstream fin(argv[1]);
  ofstream fout(argv[2]);

  string filename, label;
  int roiCount;
  char garbage;
  int x, y, width, height;
  int count = 0;

  while ( fin.good() )
  {
    fin >> filename >> roiCount;

    if ( fin.good() )
    {
      fout << filename << ' ' << roiCount;
      if ( roiCount != 0 )
      {
        for ( int i = 0; i < roiCount; i++ )
        {
          fin >> garbage >> label >> y >> x >> height >> width;
          
          // correct x coordinate
          x = 4000-x-width;

          fout << " : " << label << ' ' << x << ' ' << y
               << ' ' << width << ' ' << height;
        }
      }
      else
      {
        fin >> garbage >> label >> x >> y >> width >> height;
        
        fout << " : " << label << ' ' << x << ' ' << y
             << ' ' << width << ' ' << height;
      }
      fout << endl;
    }
    count++;
    cout << '\r' << count << "      ";
  }
  cout << endl;
  fin.close();
  fout.close();

  return 0;
}
