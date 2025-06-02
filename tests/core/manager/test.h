#include "cell_id/test.h"
#include "min_level/test.h"
#include "refine/test.h"
#include "coarse/test.h"

void registerCoreManagerTests() {
  registerCoreManagerCellIdTests();
  registerCoreManagerMinLevelTests();
  registerCoreManagerRefineTests();
  registerCoreManagerCoarseTests();
}
