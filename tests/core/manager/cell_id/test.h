#ifdef USE_MPI

#include "parallel_tests.h"

#else

void registerCoreManagerCellIdParallelTests() {}

#endif // USE_MPI

#include "serial_tests.h"

void registerCoreManagerCellIdTests() {
  registerCoreManagerCellIdSerialTests();
  registerCoreManagerCellIdParallelTests();
}
