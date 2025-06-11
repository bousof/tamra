//#define USE_MPI

#ifndef USE_MPI

void registerCoreManagerBalanceParallelTests() {}

#else

#include "parallel_tests.h"

#endif

void registerCoreManagerBalanceTests() {
  registerCoreManagerBalanceParallelTests();
}
