#include "../../includes/parallel/allreduce.h"

//-----------------------------------------------------------//
//  IMPLEMENTATIONS                                          //
//-----------------------------------------------------------//

void boolAndAllreduce(const bool value, bool &reduction) {
#ifdef USE_MPI
  allreduce::detail::scalarAllreduceT(value, reduction, MPI_C_BOOL, MPI_LAND);
#else
  allreduce::detail::scalarAllreduceT(value, reduction);
#endif // USE_MPI
}
