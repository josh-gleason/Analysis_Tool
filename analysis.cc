/******************************************************************************\
|  Analysis Tool                                                               |
|                                                                              |
|  Author: Joshua Gleason                                                      |
|  Date: June 16, 2011                                                         |
|                                                                              |
|    This tool takes two files, one containing a list of images with labeled   |
|  ground truth and another with a list of computed regions.  The tool then    |
|  compares how well the computed regions match the true regions.              |
|                                                                              |
\******************************************************************************/

#include <assert.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <highgui.h>
#include "analysis_tools.h"
#include "options.h"
#include "image_region_list.h"
#include "io.h"
#include "progress_bar.h"

namespace fs = boost::filesystem;
namespace at = analysis_tools;

// TODO add options for polygons, ellipses and circles for label types
// TODO implement labels (give them a use)
//   TODO add option to ignore labels
// TODO draw a line to the nearest match in DrawResults
// TODO add filtering by score and color score
//   TODO may want to change name of score as its same as color score
// TODO output results
// TODO Implement having a second score value

// simple structure containing an index and a score
struct IndexScore
{
  IndexScore() {}
  IndexScore(int i, double s) : index(i), score(s) {}
  IndexScore(const IndexScore& a) : index(a.index), score(a.score) {}
  
  int index;
  double score;
};

/******************************************************************************\
|                          FUNCTION PROTOTYPES                                 |
\******************************************************************************/

/**ComputeScore****************************************************************\
|   Description: Compute score between true_roi and computed_roi               |
|    Input:                                                                    |
|      true_roi/computed_roi: two rectangles to compare                        |
|    Output: Return the "closeness" score.                                     |
\******************************************************************************/
double ComputeScore(
  const cv::Rect& true_roi,
  const cv::Rect& computed_roi
);

/**DetermineMatches************************************************************\
|   Description: Find the top matching regions for each ROI in true_roi_list   |
|                computared to all the ROI in the corresponding index of       |
|                computed_roi. Then place them all of the top matches in       |
|                ascending order in top_matches[image_index][roi_index].       |
|                Note: If no regions return non-zero score, list may be empty. |
|   Input:                                                                     |
|     true_roi_list: Ground truth data                                         |
|     computed_roi_list: Computed Regions to compare to                        |
|   Output:                                                                    |
|     top_match: lists of top matches for each ROI in true_roi_list            |
\******************************************************************************/
void DetermineMatches(
  const std::vector<ImageRegionList>&                     true_roi_list,
  const std::vector<ImageRegionList>&                     computed_roi_list,
  std::vector< std::vector< std::vector<IndexScore> > >&  top_matches
);

/**PrintResults****************************************************************\
|   Description: Determine results from computed list of sorted regions.       |
|   Input:                                                                     |
|     true_roi_list: Ground truth data                                         |
|     computed_roi_list: Computed Regions of interest                          |
|     top_match: output from DetermineMatches()                                |
|     program_settings: settings                                               |
|   Output: Write results to output stream determined by program_settings      |
\******************************************************************************/
void PrintResults(
  const std::vector<ImageRegionList>&                        true_roi_list,
  const std::vector<ImageRegionList>&                        computed_roi_list,
  const std::vector<std::vector<std::vector<IndexScore> > >& top_matches,
  const Settings&                                            program_settings
);

/**DrawResults*****************************************************************\
|   Description: Draws both true and computed regions to image                 |
|   Input:                                                                     |
|     true/computed_roi_list: true and computed regions of interest            |
|     program_settings: settings                                               |
|   Output: Writes all the images to a folder.                                 |
\******************************************************************************/
void DrawResults(
  std::vector<ImageRegionList>& true_roi_list,
  std::vector<ImageRegionList>& computed_roi_list,
  const Settings&               program_settings
);

/**DescendingSort**************************************************************\
|   Description: used by sort() to sort in descending order                    |
\******************************************************************************/
bool DescendingSortFunc(IndexScore lhs, IndexScore rhs)
{ return lhs.score > rhs.score; }

/******************************************************************************\
|                              MAIN FUNCTION                                   |
\******************************************************************************/
int main(int argc, char *argv[])
{
  /****************************************************************************\
  |                          DECLARE VARIABLES                                 |
  \****************************************************************************/
  Settings program_settings;

  // each ImageRegionList contains all the regions for one image
  std::vector<ImageRegionList> true_roi_list;
  std::vector<ImageRegionList> computed_roi_list;
  
  // a list of matches for each region in each image, i.e., 
  // top_matches[image][roi_index] would correspond to the list of top matches
  // of the <roi_index> region of interest in <image>
  std::vector< std::vector< std::vector<IndexScore> > > top_matches;

  /****************************************************************************\
  |                          INTIALIZE VARIABLES                               |
  \****************************************************************************/
  LoadSettings(argc, argv, program_settings);

  // loads the files into vectors of ImageRegionList objects
  LoadTrueROI(program_settings.true_roi_path, true_roi_list);
  LoadComputedROI(program_settings.computed_roi_path, computed_roi_list);
  
  /****************************************************************************\
  |                              RUN PROGRAM                                   |
  \****************************************************************************/

  // build list of top matching computed regions for each roi in ground truth
  DetermineMatches(true_roi_list, computed_roi_list, top_matches);

  // print results
  PrintResults(true_roi_list, computed_roi_list, top_matches, program_settings);

  // draw results on images and save
  DrawResults(true_roi_list, computed_roi_list, program_settings);
  
  // print settings XXX Remove Me
  PrintSettings(program_settings, std::cout);

  return 0;
}

/******************************************************************************\
|                           FUNCTION IMPLEMENTATIONS                           |
\******************************************************************************/
void DetermineMatches(const std::vector<ImageRegionList>& true_roi_list,
  const std::vector<ImageRegionList>& computed_roi_list,
  std::vector< std::vector< std::vector<IndexScore> > >& top_matches)
{
  // iterator typedefs
  typedef std::vector<ImageRegionList>::const_iterator
          ConstRegionIterator;

  typedef std::vector< std::vector< std::vector<IndexScore> > >::iterator
          Vector3DIterator;

  typedef std::vector< std::vector<IndexScore> >::iterator
          Vector2DIterator;

  typedef std::vector<cv::Rect>::const_iterator
          ConstRectIterator;

  // test for valid inputs
  {
    assert(top_matches.empty());
    assert(true_roi_list.size()==computed_roi_list.size());

    // makes sure image paths coincide
    ConstRegionIterator true_roi_it = true_roi_list.begin();
    ConstRegionIterator computed_roi_it = computed_roi_list.begin();
    for ( ; computed_roi_it != computed_roi_list.end();
            ++true_roi_it, ++computed_roi_it )
      assert( true_roi_it->image_path == computed_roi_it->image_path );
  }
   
  // initialize 1st dimension of top_matches
  top_matches.resize(true_roi_list.size());

  // initialize an empty list for each ROI (2nd dimension of top_matches)
  {
    // loop iterates through both true_roi_list and top_matches
    ConstRegionIterator true_roi_it = true_roi_list.begin();
    Vector3DIterator top_matches_it = top_matches.begin();
    for ( ; top_matches_it != top_matches.end();
            ++top_matches_it, ++true_roi_it)
      top_matches_it->resize(true_roi_it->regions.size());
  }

  // calculate unsorted top matches and store (3d dimension of top_matches)
  {
    // iterate through all three
    Vector3DIterator top_matches_it = top_matches.begin();
    ConstRegionIterator true_roi_it = true_roi_list.begin();
    ConstRegionIterator computed_roi_it = computed_roi_list.begin();
    for ( ; true_roi_it != true_roi_list.end();
            ++top_matches_it, ++true_roi_it, ++computed_roi_it )
    {
      // for each region in current index of true_roi_list
      ConstRectIterator true_regions_it = true_roi_it->regions.begin();
      ConstRectIterator true_regions_end = true_roi_it->regions.end();
      Vector2DIterator top_regions_it = top_matches_it->begin();
      Vector2DIterator top_regions_end = top_matches_it->end();
      for ( ; true_regions_it != true_regions_end;
              ++true_regions_it, ++top_regions_it )
      {
        // compare against every region in current index of computed_roi
        ConstRectIterator computed_regions_it =
          computed_roi_it->regions.begin();
        ConstRectIterator computed_regions_end =
          computed_roi_it->regions.end();
        int index = 0;
        for ( ; computed_regions_it != computed_regions_end;
                ++computed_regions_it, ++index )
        {
          static double score;
          score = ComputeScore(*true_regions_it, *computed_regions_it);
          if ( score > 0 )  // only save if score is greater than zero
            top_regions_it->push_back(IndexScore(index, score));
        }
      }
    }
  }

  // sort lists of top matches
  {
    // iterate through all three
    Vector3DIterator top_matches_it = top_matches.begin();
    for ( ; top_matches_it != top_matches.end(); ++top_matches_it )
    {
      // for each region in current index of true_roi_list
      Vector2DIterator top_regions_it = top_matches_it->begin();
      Vector2DIterator top_regions_end = top_matches_it->end();
      for ( ; top_regions_it != top_regions_end; ++top_regions_it )
        sort(top_regions_it->begin(),top_regions_it->end(),DescendingSortFunc);
    }
  }

}

void PrintResults( const std::vector<ImageRegionList>& true_roi_list,
  const std::vector<ImageRegionList>& computed_roi_list,
  const std::vector< std::vector< std::vector<IndexScore> > >& top_matches,
  const Settings& program_settings )
{
  // TODO: add return value, struct for holding results

  // calculates:  True detection rate (Detected/total)
  //              Number of false positives
  //              XXX(possibly) False positives per image

  // level 1: non-exclusive matching. i.e., 
  //          More than one computed region can overlap one ground truth and
  //          none will be counted as a false positive.
  //          Also if one computed region overlaps two (or more) ground truths,
  //          will count as a match to both ground truths.

  // level 2: semi-exclusive matching(1). i.e.,
  //          More than one computed region can overlap one ground truth and
  //          neither will be counted as a false positive.
  //          However, if one computed region overlaps two ground truths
  //          only one ground truth is considered matched/detected.
  
  // level 3: semi-exclusive matching(2). i.e.,
  //          If more than one computed region overlaps one ground truth,
  //          the lower matching regions will be counted as false positives.
  //          However, if one computed region overlaps two ground truths
  //          only one ground truth is considered matched/detected.
  
  // level 4: exclusive matching. i.e., 1-to-1 matching
  //          If more than one computed region overlaps one ground truth, the
  //          lower scoring computed regions will be counted as false positives.
  //          However if one computed region overlaps two (or more) ground
  //          truths, it will count as a match to both ground truths.


  // TODO: perhaps this should be sepearated into sub-functions
  // count false positives

  // calculate truth detection rate

  // print results
}

double ComputeScore(const cv::Rect& true_roi, const cv::Rect& computed_roi)
{
  at::Rect true_roi_at(true_roi.x,
                       true_roi.y,
                       true_roi.width,
                       true_roi.height);

  at::Rect computed_roi_at(computed_roi.x,
                           computed_roi.y,
                           computed_roi.width,
                           computed_roi.height);

  return at::computeScore(true_roi_at, computed_roi_at);
}

void DrawResults(std::vector<ImageRegionList>& true_roi_list,
                 std::vector<ImageRegionList>& computed_roi_list,
                 const Settings& program_settings)
{
  if ( !program_settings.draw_results )
    return;
  
  // if output folder does not exist, create it
  if ( !fs::exists(program_settings.draw_results_folder) )
  {
    std::cout << "Creating directory" << std::endl;
    if ( !fs::create_directories(program_settings.draw_results_folder) )
    {
      std::cout << "Could not create folder "
                << program_settings.draw_results_folder
                << std::endl;
      return;
    }
  }
  
  // iterator typedefs
  typedef std::vector<ImageRegionList>::const_iterator
          ConstRegionIterator;

  typedef std::vector<cv::Rect>::const_iterator
          ConstRectIterator;

  ProgressBar progress_bar(
    cout,
    "Drawing Results ",
    static_cast<int>(true_roi_list.size()),
    60
  );

  int progress = 0;

  // draw the progress bar
  progress_bar.update(progress);

  // draw rectangles on each image
  ConstRegionIterator true_roi_it = true_roi_list.begin();
  ConstRegionIterator computed_roi_it = computed_roi_list.begin();
  for ( ; true_roi_it != true_roi_list.end();
          ++true_roi_it, ++computed_roi_it )
  {
    cv::Mat img = cv::imread(true_roi_it->image_path.file_string());
    
    ConstRectIterator true_regions_it = true_roi_it->regions.begin();
    ConstRectIterator true_regions_end = true_roi_it->regions.end();
    for ( ; true_regions_it != true_regions_end; ++true_regions_it )
      cv::rectangle(img,
                    *true_regions_it,
                    cv::Scalar(0,255,0),  // color
                    2,    // thickness
                    8,    // line type
                    0);   // shift

    ConstRectIterator computed_regions_it = computed_roi_it->regions.begin();
    ConstRectIterator computed_regions_end = computed_roi_it->regions.end();
    for ( ; computed_regions_it != computed_regions_end; ++computed_regions_it )
      cv::rectangle(img,
                    *computed_regions_it,
                    cv::Scalar(0,0,255),
                    2,
                    8,
                    0);
  
    // build image path as ...
    // DRAW_RESULTS_FOLDER/ORIGINAL_BASENAME_analysis.ORIGINAL_EXTENSION
    std::string image_name =
      fs::basename(true_roi_it->image_path.filename())+"_analysis";
    
    fs::path image_path =
      program_settings.draw_results_folder /
      std::string(
        image_name +
        fs::extension(true_roi_it->image_path)
      );

    // write the image
    imwrite(image_path.file_string(), img);

    progress_bar.update(progress++);
  }
}

