#include "balance/test.h"
#include "cell_id/test.h"
#include "coarse/test.h"
#include "ghost/test.h"
#include "min_level/test.h"
#include "refine/test.h"

void registerCoreManagerTests() {
  registerCoreManagerCellIdTests();
  registerCoreManagerMinLevelTests();
  registerCoreManagerRefineTests();
  registerCoreManagerCoarseTests();
  registerCoreManagerBalanceTests();
  registerCoreManagerGhostTests();
}
