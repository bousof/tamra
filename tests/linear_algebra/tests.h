#ifdef USE_MPI

#include "parallel_tests.h"

#else

void registerLinalgParallelTests() {}

#endif // USE_MPI

#include "serial_tests.h"

void registerLinalgTests() {
  registerLinalgSerialTests();
  registerLinalgParallelTests();
}
