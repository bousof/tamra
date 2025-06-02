//#define USE_MPI

#ifndef USE_MPI

void registerCommunicationsAllToAllTests() {}
void registerCommunicationsBCastTests() {}

#else

#include "alltoall/tests.h"
#include "bcast/tests.h"

#endif

void registerCommunicationsTests() {
  registerCommunicationsAllToAllTests();
  registerCommunicationsBCastTests();
}
