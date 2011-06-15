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
|     filename: The filename of the file containined the computed ROIs         |
|   Output:                                                                    |
|     computed_regions: list of computed regions                               |
\******************************************************************************/
bool LoadComputedROI( 
  const std::string&             filename,
  std::vector<ImageRegionList>&  computed_regions 
);

/**LoadTrueROI*****************************************************************\
|   Description: Load the file contiaining the ground truth regions of         |
|                interest(ROIs)                                                |
|   Input:                                                                     |
|     filename: The filename of the file containined the computed ROIs         |
|   Output:                                                                    |
|     true_regions: list of true regions                                       |
\******************************************************************************/
bool LoadTrueROI(
  const std::string&             filename,
  std::vector<ImageRegionList>&  true_regions
);

#endif // ANALYSIS_IO

