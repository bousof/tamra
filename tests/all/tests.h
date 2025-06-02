#include "../communications/tests.h"
#include "../core/test.h"
#include "../linear_algebra/tests.h"

void registerAllTests() {
  registerCommunicationsTests();
  registerCoreTests();
  registerLinalgTests();
}
