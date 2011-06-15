#include "analysis_tools.h"

namespace analysis_tools
{

void cvt(const Point& p1, clipper::IntPoint& p2)
{
  p2.X = (clipper::long64)p1.x;
  p2.Y = (clipper::long64)p1.y;
}

void intersectRect(Rect& intersect, const Rect& r1, const Rect& r2)
{
  float lRx1 = r1.x+r1.width,
        lRy1 = r1.y+r1.height,
        lRx2 = r2.x+r2.width,
        lRy2 = r2.y+r2.height;
  
  intersect.x = max(r1.x,r2.x);
  intersect.y = max(r1.y,r2.y);
  
  float lRx3 = min(lRx1,lRx2),
        lRy3 = min(lRy1,lRy2);

  intersect.width = lRx3-intersect.x;
  intersect.height = lRy3-intersect.y;

  if ( intersect.width <= 0 || intersect.height <= 0 )
    intersect.width = intersect.height = intersect.x = intersect.y = 0;
}

double computeScore(const Rect& r1, const Rect& r2)
{
  // calculate rectangle intersection
  Rect intersect;
  intersectRect(intersect,r1,r2);
  double intersectArea = intersect.width*intersect.height;
  double unionArea = (r1.width*r1.height + r2.width*r2.height) - intersectArea;
  return intersectArea/unionArea;
}

double computeScore(const vector<Point>& polygon1, const vector<Point>& polygon2)
{
  // convert the vectors to arrays of Points
  int p1_size = polygon1.size(),
      p2_size = polygon2.size();

  clipper::Polygon poly1(polygon1.size()), poly2(polygon2.size());
  
  for( int i = 0; i < p1_size; i++ )
    cvt(polygon1[i],poly1[i]);
  for( int i = 0; i < p2_size; i++ )
    cvt(polygon2[i],poly2[i]);

  // call the other compute score function
  return computeScore(poly1,poly2);
}

// pts is the number of contours in each polygon respectively (i.e., length of array)
double computeScore(const clipper::Polygon& polygon1, const clipper::Polygon& polygon2)
{
  static clipper::Clipper c;
  c.Clear();
  
  clipper::Polygons solution;
  
  c.AddPolygon(polygon1,clipper::ptSubject);
  c.AddPolygon(polygon2,clipper::ptClip);
  if ( !c.Execute(clipper::ctIntersection,solution) )
    return 0;

  double polyIntersect = 0.0;

  for ( clipper::Polygons::iterator i = solution.begin(); i != solution.end(); ++i )
    polyIntersect += clipper::Area(*i,false);

  double polyUnion = clipper::Area(polygon1,false) + clipper::Area(polygon2,false) -
    polyIntersect;

  return polyIntersect / polyUnion;
}

int checkValid(const Rect& testRect,
  const vector<Rect>& validRects,double threshold)
{
  double scr;
  int index = -1;
  for ( vector<Rect>::const_iterator i = validRects.begin();
        i < validRects.end(); ++i )
  {
    scr = computeScore(testRect,*i);
    if ( scr > threshold )
    {
      // set a new threshold
      threshold = scr;

      // set index to most overlapping thus far
      index = distance(validRects.begin(),i);
    }
  }
  return index;
}

int checkValid(const vector<Point>& testPoly,
  const vector<vector<Point> >& validPolys,double threshold)
{
  double scr;
  int index = -1;
  for ( vector<vector<Point> >::const_iterator i = validPolys.begin();
        i < validPolys.end(); ++i )
  {
    scr = computeScore(testPoly,*i);
    if ( scr > threshold )
    {
      // set a new threshold
      threshold = scr;

      // set index to best match thus far
      index = distance(validPolys.begin(),i);
    }
  }
  return index;
}

}

