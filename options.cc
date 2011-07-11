#include "options.h"

// namespace aliasing
namespace po = boost::program_options;
namespace fs = boost::filesystem;

///////////////////////// LOCAL FUNCTIONS //////////////////////////////////////////

/******************************************************************************\
|                                                                              |
\******************************************************************************/
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

//////////////////////// GLOBAL FUNCTIONS //////////////////////////////////////

void LoadSettings(int argc, char *argv[],
  Settings& settings)
{
  // The replace string is the string that will replace any occurance
  // of %s in any of the filenames
  std::string replace_string;

  // strings to be processed before conversion to fs::path in settings
  std::string computed_roi_path;
  std::string true_roi_path;
  std::string output_results_path;
  std::string draw_results_folder;
  
  // path to the settings file (obtained from command line)
  std::string config_path;

  // Declare options description class
  po::options_description description("Allowed options");

  /****************************************************************************\
  |                           GENERIC DESCRIPTIONS                             |
  \****************************************************************************/
  description.add_options()
    
    // add the default help message
    ("help", "View this help message")
    ("help_match_level", "View help for match_threshold setting")
    ;

  // command line only options
  po::options_description command_line_options("Command Line Options");
  
  command_line_options.add_options()
    ("input_value,i", po::value<std::string>
        (&replace_string)->default_value("file"),
        "The value that replaces any occurance of \%s in any filename")
    ("config_file_path,c", po::value<std::string>
        (&config_path)->default_value("config.cfg"),
        "The configuration file name")
    ;

  // config file or command line options
  po::options_description config_options("Config File Options");

  config_options.add_options()
    
    // input files
    ("computed_roi_path", po::value<std::string>
        (&computed_roi_path),
        "Filename for the computed Regions of interest")
    ("true_roi_path", po::value<std::string>
        (&true_roi_path),
        "Filename containing ground truth")
    
    // output files
    ("output_results_path", po::value<std::string>
        (&output_results_path),
        "Results output file")
    ("draw_results_folder", po::value<std::string>
        (&draw_results_folder),
        "File location to draw results")
    
    // other settings
    ("match_level,M", po::value<int>
        (reinterpret_cast<int*>(&settings.match_level))->
          default_value(static_cast<int>(Settings::NON_EXCLUSIVE)),
        "Level of matching (--help_match_level for more information)")
    ("overlap_threshold,ot", po::value<double>
        (&settings.overlap_threshold)->default_value(0.0),
        "Minimum overlap score")
    ("draw_results,D", po::value<bool>
        (&settings.draw_results)->default_value(false),
        "Option to draw results and save images")
//    ("calculate_score_range", po::value<bool>
//        (&settings.calculate_score_range)->default_value(false),
//        "Causes associated score value to be used")
//    ("score_range", po::value<Range>
//        (&settings.score_range)->multitoken(),
//        "Range of scores to test")
    // XXX: Temporary
    ("score_threshold,S", po::value<double>
        (&settings.score_threshold)->default_value(0.0),
        "Minimum allowed score threshold")
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

  // output help for match_threshold
  if ( vm.count("help_match_level") )
  {
    std::cout << "\nmatch_level options" << std::endl
      << "\t1: non-exclusive matching. i.e.,\n"
      << "\tMore than one computed region can overlap one ground truth and\n"
      << "\tnone will be counted as a false positive.\n"
      << "\tAlso if one computed region overlaps two (or more) ground truths,\n"
      << "\twill count as a match to both ground truths.\n"
      << std::endl
      << "\t2: semi-exclusive matching(1). i.e.,\n"
      << "\tMore than one computed region can overlap one ground truth and\n"
      << "\tneither will be counted as a false positive.\n"
      << "\tHowever, if one computed region overlaps two ground truths\n"
      << "\tonly one ground truth is considered matched/detected.\n"
      << std::endl
      << "\t3: semi-exclusive matching(2). i.e.,\n"
      << "\tIf more than one computed region overlaps one ground truth,\n"
      << "\tthe lower matching regions will be counted as false positives.\n"
      << "\tHowever, if one computed region overlaps two ground truths\n"
      << "\tonly one ground truth is considered matched/detected.\n"
      << std::endl
      << "\t4: exclusive matching. i.e., 1-to-1 matching\n"
      << "\tIf more than one computed region overlaps one ground truth, the\n"
      << "\tlower scoring computed regions will be counted as false positives.\n"
      << "\tHowever if one computed region overlaps two (or more) ground\n"
      << "\ttruths, it will count as a match to both ground truths.\n"
      << std::endl;
              
    exit(0);
  }

  /****************************************************************************\
  |                       READ IN CONFIG FILE OPTIONS                          |
  \****************************************************************************/
  std::ifstream fin(config_path.c_str());
  if ( !fin )
  {
    std::cout << "Error: Could not open config file \"" << config_path
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
  FindReplace(computed_roi_path,
              "%s",
              replace_string,
              computed_roi_path);
  
  FindReplace(true_roi_path,
              "%s",
              replace_string,
              true_roi_path);
  
  FindReplace(output_results_path,
              "%s",
              replace_string,
              output_results_path);
  
  FindReplace(draw_results_folder,
              "%s",
              replace_string,
              draw_results_folder);

  // assign settings fs::path objects using string path values
  settings.computed_roi_path   = fs::path(computed_roi_path);
  settings.true_roi_path       = fs::path(true_roi_path);
  settings.output_results_path = fs::path(output_results_path);
  settings.draw_results_folder = fs::path(draw_results_folder);
}

void PrintSettings( const Settings& settings, std::ostream& out )
{
  // alias to shorten lines
  typedef Settings s;

  out << "# Program Options"      << std::endl
      << "computed_roi_path   = " << settings.computed_roi_path   << std::endl
      << "true_roi_path       = " << settings.true_roi_path       << std::endl
      << "output_results_path = " << settings.output_results_path << std::endl
      << "draw_results_folder = " << settings.draw_results_folder << std::endl
      << "draw_results        = " << settings.draw_results        << std::endl
      << "overlap_threshold   = " << settings.overlap_threshold   << std::endl
      << "match_level         = " << static_cast<int>(settings.match_level) <<
        (settings.match_level == s::NON_EXCLUSIVE    ?"\t\t# NON_EXCLUSIVE"   :
        (settings.match_level == s::SEMI_EXCLUSIVE_1 ?"\t\t# SEMI_EXCLUSIVE_1":
        (settings.match_level == s::SEMI_EXCLUSIVE_2 ?"\t\t# SEMI_EXCLUSIVE_2":
        (settings.match_level == s::EXCLUSIVE        ?"\t\t# EXCLUSIVE "      :
        "" )))) << std::endl
  ;
}

// overloaded extraction operator
std::istream& operator>> ( std::istream &in, Range& range )
{
  in >> range.start >> range.step >> range.end;
  return in;
}

