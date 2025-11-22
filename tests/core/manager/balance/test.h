#ifdef USE_MPI

#include "parallel_tests.h"

#else

void registerCoreManagerBalanceParallelTests() {}

#endif // USE_MPI

void registerCoreManagerBalanceTests() {
  registerCoreManagerBalanceParallelTests();
}
