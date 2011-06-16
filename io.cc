
#include "io.h"

namespace fs = boost::filesystem;

bool LoadComputedROI( const fs::path& filename,
  std::vector<ImageRegionList>& computed_regions )
{
  // open file
  std::ifstream fin(filename.native().c_str());
  
  std::string line;

  // check for bad file
  if ( !fin.good() )
    return false;

  // iterate through each line of the file
  {
    std::string image_path;
    char garbage;
    size_t region_count;

    size_t index = computed_regions.size();
    getline(fin, line);
    while ( fin.good() )
    {
      // line should have following format
      // <image> <#roi> : <label> <score> <x> <y> <width> <height> : <label> ...
      
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
      for ( size_t i = 0; i < region_count; ++i )
        sin >> garbage >> computed_regions[index].labels[i]
            >> computed_regions[index].scores[i]
            >> computed_regions[index].regions[i].x
            >> computed_regions[index].regions[i].y
            >> computed_regions[index].regions[i].width
            >> computed_regions[index].regions[i].height;

      index++;
      getline(fin, line);
    }
  }

  // close file
  fin.close();

  return true; // stub
}

bool LoadTrueROI( const fs::path& filename,
  std::vector<ImageRegionList>& true_regions )
{
  return false; // stub
}

