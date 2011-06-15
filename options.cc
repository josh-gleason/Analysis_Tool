#include "options.h"

// namespace alias
namespace po = boost::program_options;

// replace all occurance of <repl> in <input> with <replValue>
void FindReplace( const std::string& input, const std::string& place_holder,
  const std::string& replace_value, std::string& output )
{
  output = input;
  
  size_t index, place_holder_length;
  place_holder_length = place_holder.size();
  
  // search for place holders and replace until none are found
  index = output.find(place_holder);
  while ( index != std::string::npos )
  {
    output = output.replace(index,place_holder_length,replace_value);
    index = output.find(place_holder);
  }
}

void LoadSettings(const std::string& config_filename, int argc, char *argv[],
  Settings& settings)
{
  // The replace string is the string that will replace any occurance
  // of %s in any of the filenames
  std::string replace_string;

  // Declare options description class
  po::options_description description("Allowed options");

  /****************************************************************************\
  |                           GENERIC DESCRIPTIONS                             |
  \****************************************************************************/
  description.add_options()
    
    // add the default help message
    ("help", "View help message");

  // command line only options
  po::options_description command_line_options("Command Line Options");
  
  command_line_options.add_options()
    ("input_value,i", po::value<std::string>
        (&replace_string)->default_value("file"),
        "The value that replaces any occurance of \%s in any filename")
    ;

  // config file or command line options
  po::options_description config_options("Config File Options");

  config_options.add_options()
    
    // input files
    ("computed_roi_filename", po::value<std::string>
        (&settings.computed_roi_filename),
        "Filename for the computed Regions of interest")
    ("true_roi_filename", po::value<std::string>
        (&settings.true_roi_filename),
        "Filename containing ground truth")
    
    // output files
    ("output_results_filename", po::value<std::string>
        (&settings.output_results_filename),
        "Results output file")
    ("draw_results_folder", po::value<std::string>
        (&settings.draw_results_folder),
        "File location to draw results")
    
    // flags
    ("draw_results,D", po::value<bool>
        (&settings.draw_results)->default_value(false),
        "Option to draw results and save images")
  ;

  // add all to file descriptions
  description.add(config_options).add(command_line_options);
  
  // declare the variable map for reading in values
  po::variables_map vm;

  /****************************************************************************\
  |                       READ IN COMMAND LINE ARGUMENTS                       |
  \****************************************************************************/

  store(po::parse_command_line(argc, argv, description), vm);
  notify(vm);

  // output help
  if ( vm.count("help") )
  {
    std::cout << description << "\n";
    exit(0);
  }

  /****************************************************************************\
  |                       READ IN CONFIG FILE OPTIONS                          |
  \****************************************************************************/
  std::ifstream fin(config_filename.c_str());
  if ( !fin )
  {
    std::cout << "Error: Could not open config file \"" << config_filename 
              << '\"' << std::endl;
    exit(0);
  }
  else
  {
    store(parse_config_file(fin, config_options), vm);
    notify(vm);
    fin.close();
  }

  /****************************************************************************\
  |                              POST PROCESSING                               |
  \****************************************************************************/

  // textural replacement of %s in all file path strings
  FindReplace(settings.computed_roi_filename,
              "%s",
              replace_string,
              settings.computed_roi_filename);
  
  FindReplace(settings.true_roi_filename,
              "%s",
              replace_string,
              settings.true_roi_filename);
  
  FindReplace(settings.output_results_filename,
              "%s",
              replace_string,
              settings.output_results_filename);
  
  FindReplace(settings.draw_results_folder,
              "%s",
              replace_string,
              settings.draw_results_folder);
}

