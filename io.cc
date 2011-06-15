
#include "io.h"

namespace fs = boost::filesystem;

bool LoadComputedROI( const fs::path& filename,
  std::vector<ImageRegionList>&  computed_regions )
{
  return false; // stub
}

bool LoadTrueROI( const fs::path& filename,
  std::vector<ImageRegionList>& true_regions )
{
  return false; // stub
}

