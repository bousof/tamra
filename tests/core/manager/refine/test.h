#ifdef USE_MPI

#include "parallel_tests.h"

#else

void registerCoreManagerRefineParallelTests() {}

#endif // USE_MPI

#include "serial_tests.h"

void registerCoreManagerRefineTests() {
  registerCoreManagerRefineSerialTests();
  registerCoreManagerRefineParallelTests();
}
