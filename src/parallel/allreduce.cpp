#include <tamra/parallel/allreduce.h>

//-----------------------------------------------------------//
//  IMPLEMENTATIONS                                          //
//-----------------------------------------------------------//

void bool_and_allreduce(const bool value, bool &reduction) {
#ifdef USE_MPI
  allreduce::detail::scalar_allreduce_impl(value, reduction, MPI_C_BOOL, MPI_LAND);
#else
  allreduce::detail::scalar_allreduce_impl(value, reduction);
#endif // USE_MPI
}
