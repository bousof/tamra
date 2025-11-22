#ifdef USE_MPI

#include "parallel_tests.h"

#else

void registerCoreCellOctParallelTests() {}

#endif // USE_MPI

#include "serial_tests.h"

void registerCoreCellOctTests() {
  registerCoreCellOctSerialTests();
  registerCoreCellOctParallelTests();
}
