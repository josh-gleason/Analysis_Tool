#include <iostream>
#include <fstream>
#include "analysis_tools.h"
#include "options.h"

#define CONFIG_FILE "settings.cfg"

// TODO : Obviously a lot, need to add boost filesystem to enable generic
//        file handling, then clone Ara's analysis tool

int main(int argc, char *argv[])
{
  // variable declaration/initialization
  Settings program_settings;

  LoadSettings(CONFIG_FILE, argc, argv, program_settings);

  //std::cout << program_settings.computed_roi_filename << std::endl
  //          << program_settings.true_roi_filename << std::endl
  //          << program_settings.output_results_filename << std::endl
  //          << program_settings.draw_results_folder << std::endl
  //          << program_settings.draw_results << std::endl;

  return 0;
}



