#ifndef FILE_SETTINGS_PROGRAM_OPTIONS
#define FILE_SETTINGS_PROGRAM_OPTIONS

#include <iostream>
#include <fstream>
#include <string>
#include <boost/program_options.hpp>

struct Settings
{
  std::string computed_roi_filename;
  std::string true_roi_filename;
  std::string output_results_filename;
  std::string draw_results_folder;
  bool draw_results;
};

void LoadSettings(const std::string& filename, int argc, char *argv[], Settings &settings);

#endif // FILE_SETTINGS_PROGRAM_OPTIONS

