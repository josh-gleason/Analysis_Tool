#ifndef FILE_SETTINGS_PROGRAM_OPTIONS
#define FILE_SETTINGS_PROGRAM_OPTIONS

#include <iostream>
#include <fstream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

struct Settings
{
  boost::filesystem::path computed_roi_path;
  boost::filesystem::path true_roi_path;
  boost::filesystem::path output_results_path;
  boost::filesystem::path draw_results_folder;
  bool draw_results;
};

/**LoadSettings****************************************************************\
|    Description: Load the settings from the settings file.  The settings file |
|                 is defined on the command line using the option              |
|                 -s (--settings-file).                                        |
|    Input:                                                                    |
|      argc/argc: command line arguments                                       |
|    Output:                                                                   |
|      settings: program settings                                              |
\******************************************************************************/
void LoadSettings(
  int        argc,
  char       *argv[],
  Settings&  settings
);

#endif // FILE_SETTINGS_PROGRAM_OPTIONS

