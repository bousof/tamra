#ifdef USE_MPI

#include "alltoall/tests.h"
#include "bcast/tests.h"

#else

void registerCommunicationsAllToAllTests() {}
void registerCommunicationsBcastTests() {}

#endif // USE_MPI

void registerCommunicationsTests() {
  registerCommunicationsAllToAllTests();
  registerCommunicationsBcastTests();
}
