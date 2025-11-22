#ifdef USE_MPI

#include "parallel_tests.h"

#else

void registerCoreManagerMinLevelParallelTests() {}

#endif // USE_MPI

#include "serial_tests.h"

void registerCoreManagerMinLevelTests() {
  registerCoreManagerMinLevelSerialTests();
  registerCoreManagerMinLevelParallelTests();
}
