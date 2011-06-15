//
// Description : Functions for computing the overlap score of two
//               contiguous polygons or rectangles.
//
// computeScore() computes overlap score using Intersection/Union
//
// Author : Joshua Gleason
// Date   : June 2, 2011
//

#ifndef ANALYSIS_TOOLS
#define ANALYSIS_TOOLS

#include <vector>
#include "clipper.h"

using namespace std;

namespace analysis_tools
{
  struct Point
  {
    Point() : x(0.0), y(0.0) {}
    Point(float _x, float _y) : x(_x), y(_y) {}
    float x, y;
  };
  
  struct Rect
  {
    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(float _x, float _y, float _w, float _h) : x(_x), y(_y), width(_w), height(_h) {}
    float x, y, width, height;
  };

  // calculate the intersection rectangle
  void intersectRect(Rect& intersect, const Rect& r1, const Rect& r2);

  // test if polygon is valid against a list of polygons (has score above threshold)
  // returns the first valid polygon index or -1 if none are found
  int checkValid(const Rect&, const vector<Rect>&,double=0.0);
  int checkValid(const vector<Point>&, const vector<vector<Point> >&,double=0.0);

  // computes overlap score of two polygons (or rectangles)
  // this score is calculated using Intersect/Union
  double computeScore(const vector<Point>&, const vector<Point>&);
  double computeScore(const Rect&, const Rect&);
  double computeScore(const clipper::Polygon&, const clipper::Polygon&);

  // compute the bounding box for a polygon
  Rect boundingBox(const Point*,int);
}

#endif // ANALYSIS_TOOLS

