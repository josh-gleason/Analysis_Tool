#ifndef ANALYSIS_IMAGE_REGION_LIST
#define ANALYSIS_IMAGE_REGION_LIST

#include <vector>
#include <cv.h>

struct ImageRegionList
{
  std::string image_filename;
  std::vector<cv::Rect> regions;
};

#endif // ANALYSIS_IMAGE_REGION_LIST

