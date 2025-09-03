//#define USE_MPI

#ifndef USE_MPI

void registerCommunicationsAllToAllTests() {}
void registerCommunicationsBcastTests() {}

#else

#include "alltoall/tests.h"
#include "bcast/tests.h"

#endif

void registerCommunicationsTests() {
  registerCommunicationsAllToAllTests();
  registerCommunicationsBcastTests();
}
