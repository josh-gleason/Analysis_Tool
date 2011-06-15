#ifndef ANALYSIS_IO
#define ANALYSIS_IO

#include <fstream>
#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include "image_region_list.h"

/**LoadComputedROI*************************************************************\
|   Description: Load the computed regions of interest(ROIs) in a file         |
|   Input:                                                                     |
|     filename: Path to the file containined the computed ROIs                 |
|   Output:                                                                    |
|     computed_regions: List of computed regions                               |
\******************************************************************************/
bool LoadComputedROI( 
  const boost::filesystem::path&  filename,
  std::vector<ImageRegionList>&   computed_regions 
);

/**LoadTrueROI*****************************************************************\
|   Description: Load the file contiaining the ground truth regions of         |
|                interest(ROIs)                                                |
|   Input:                                                                     |
|     filename: Path to the file containined the computed ROIs                 |
|   Output:                                                                    |
|     true_regions: List of true regions                                       |
\******************************************************************************/
bool LoadTrueROI(
  const boost::filesystem::path&  filename,
  std::vector<ImageRegionList>&   true_regions
);

#endif // ANALYSIS_IO

