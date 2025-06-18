//#define USE_MPI

#ifndef USE_MPI

void registerCoreManagerGhostParallelTests() {}

#else

#include "parallel_tests.h"

#endif

void registerCoreManagerGhostTests() {
  registerCoreManagerGhostParallelTests();
}
