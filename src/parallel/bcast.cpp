#include "../../includes/parallel/bcast.h"

//-----------------------------------------------------------//
//  IMPLEMENTATIONS                                          //
//-----------------------------------------------------------//

void stringBcast(std::string &value, const unsigned root) {
#ifdef USE_MPI
  int length = static_cast<int>(value.size());
  MPI_Bcast(&length, 1, MPI_INT, root, MPI_COMM_WORLD);
  value.resize(length);
  if (length > 0)
    MPI_Bcast(value.data(), length, MPI_CHAR, root, MPI_COMM_WORLD);
#else
  (void) value; (void) root; // Unsued
  // No MPI, so one proc then nothing to do
#endif // USE_MPI
}
