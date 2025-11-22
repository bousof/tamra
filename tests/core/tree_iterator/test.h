#ifdef USE_MPI

#include "parallel_tests.h"

#else

void registerCoreTreeIteratorParallelTests() {}

#endif // USE_MPI

#include "serial_tests.h"

void registerCoreTreeIteratorTests() {
  registerCoreTreeIteratorSerialTests();
  registerCoreTreeIteratorParallelTests();
}
