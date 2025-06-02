#include "cell_oct/test.h"
#include "manager/test.h"
#include "tree/test.h"
#include "tree_iterator/test.h"

void registerCoreTests() {
  registerCoreCellOctTests();
  registerCoreManagerTests();
  registerCoreTreeTests();
  registerCoreTreeIteratorTests();
}
