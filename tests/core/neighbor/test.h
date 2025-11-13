//#define USE_MPI

#ifndef USE_MPI

void registerCoreNeighborTests() {}

#endif

#include "serial_tests.h"

void registerCoreNeighborTests() {
  registerCoreNeighborSerialTests();
}
