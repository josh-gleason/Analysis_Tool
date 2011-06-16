
#include "io.h"

namespace fs = boost::filesystem;

bool LoadComputedROI( const fs::path& file_path,
  std::vector<ImageRegionList>& computed_regions )
{
  // open file
  std::ifstream fin(file_path.native().c_str());
  
  std::string line;

  // check for bad file
  if ( !fin.good() )
    return false;

  // itterate through each line of the file
  {
    std::string image_path;
    char garbage;
    size_t region_count;

    size_t index = computed_regions.size();
    getline(fin, line);
    while ( fin.good() )
    {
      // line should have following format
      // <image> <#roi> : <label> <score> <ULx> <ULy> <LRx> <LRy> : <label> ...
      
      // load string stream to read from
      std::istringstream sin(line);

      sin >> image_path >> region_count;
      
      // increase size of vector
      computed_regions.push_back(ImageRegionList());

      // initialize arrays in the ImageRegionList
      computed_regions[index].regions.resize(region_count);
      computed_regions[index].labels.resize(region_count);
      computed_regions[index].scores.resize(region_count);

      // implicit convertion from string to fs::path
      computed_regions[index].image_path = image_path;

      // read every region
      cv::Point upper_left, lower_right;
      for ( size_t i = 0; i < region_count; ++i )
      {
        sin >> garbage >> computed_regions[index].labels[i]
            >> computed_regions[index].scores[i]
            >> upper_left.x  >> upper_left.y
            >> lower_right.x >> lower_right.y;
        
        // store rectangles in arrays
        computed_regions[index].regions[i].x = upper_left.x;
        computed_regions[index].regions[i].y = upper_left.y;
        computed_regions[index].regions[i].width =
            lower_right.x - upper_left.x;
        computed_regions[index].regions[i].height =
            lower_right.y - upper_left.y;
      }

      ++index;
      getline(fin, line);
    }
  }

  // close file
  fin.close();

  return true;
}

bool LoadTrueROI( const fs::path& file_path,
  std::vector<ImageRegionList>& true_regions )
{
  // open file
  std::ifstream fin(file_path.native().c_str());
  
  std::string line;

  // check for bad file
  if ( !fin.good() )
    return false;

  // itterate through each line of the file
  {
    std::string image_path;
    char garbage;
    size_t region_count;

    size_t index = true_regions.size();
    getline(fin, line);
    while ( fin.good() )
    {
      // line should have following format
      // <image> <#roi> : <label> <ULx> <ULy> <width> <height> : <label> ...
      
      // load string stream to read from
      std::istringstream sin(line);

      sin >> image_path >> region_count;
      
      // increase size of vector
      true_regions.push_back(ImageRegionList());

      // initialize arrays in the ImageRegionList
      true_regions[index].regions.resize(region_count);
      true_regions[index].labels.resize(region_count);
      true_regions[index].scores.resize(0);

      // implicit convertion from string to fs::path
      true_regions[index].image_path = image_path;

      // read every region
      for ( size_t i = 0; i < region_count; ++i )
        sin >> garbage >> true_regions[index].labels[i]
            >> true_regions[index].regions[i].x
            >> true_regions[index].regions[i].y
            >> true_regions[index].regions[i].width
            >> true_regions[index].regions[i].height;

      ++index;
      getline(fin, line);
    }
  }

  // close file
  fin.close();

  return true;
}

