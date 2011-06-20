#ifndef ANALYSIS_IMAGE_REGION_LIST
#define ANALYSIS_IMAGE_REGION_LIST

#include <vector>
#include <cv.h>
#include <boost/filesystem.hpp>

#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_FILESYSTEM_NO_DEPRECATED

struct ImageRegionList
{
  // holds the file path to the image being processed
  boost::filesystem::path   image_path;

  // list of regions with corresponding labels
  std::vector<cv::Rect>     regions;
  std::vector<std::string>  labels;
  std::vector<float>        scores;
};

#endif // ANALYSIS_IMAGE_REGION_LIST

