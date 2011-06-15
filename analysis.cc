#include <iostream>
#include <fstream>
#include "analysis_tools.h"
#include "options.h"
#include "image_region_list.h"
#include "io.h"

#define CONFIG_FILE "settings.cfg"

// TODO : Obviously a lot, need to add boost filesystem to enable generic
//        file handling, then clone Ara's analysis tool

int main(int argc, char *argv[])
{
  // variable declaration
  Settings program_settings;

  std::vector<ImageRegionList> computed_roi_list;
  std::vector<ImageRegionList> true_roi_list;

  // initialization
  LoadSettings(CONFIG_FILE, argc, argv, program_settings);

  LoadComputedROI(
    program_settings.computed_roi_filename,
    computed_roi_list
  );
  
  LoadTrueROI(
    program_settings.true_roi_filename,
    true_roi_list
  );

  //std::cout << program_settings.computed_roi_filename << std::endl
  //          << program_settings.true_roi_filename << std::endl
  //          << program_settings.output_results_filename << std::endl
  //          << program_settings.draw_results_folder << std::endl
  //          << program_settings.draw_results << std::endl;

  return 0;
}



