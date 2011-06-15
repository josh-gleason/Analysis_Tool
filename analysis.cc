#include <iostream>
#include <fstream>
#include "analysis_tools.h"
#include "options.h"
#include "image_region_list.h"
#include "io.h"

namespace fs = boost::filesystem;

int main(int argc, char *argv[])
{
  // variable declaration
  Settings program_settings;

  std::vector<ImageRegionList> computed_roi_list;
  std::vector<ImageRegionList> true_roi_list;

  // initialization
  LoadSettings(argc, argv, program_settings);

  LoadComputedROI(
    program_settings.computed_roi_path,
    computed_roi_list
  );
  
  LoadTrueROI(
    program_settings.true_roi_path,
    true_roi_list
  );

  // print settings
  OutputSettings(program_settings, std::cout);

  return 0;
}

