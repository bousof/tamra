#include "../../includes/parallel/allreduce.h"

void boolAndAllReduce(const bool value, bool &reduction) {
#ifdef USE_MPI
  scalarAllReduce(value, reduction, MPI_C_BOOL, MPI_LAND);
#else
  scalarAllReduce(value, reduction);
#endif // USE_MPI
}

void unsignedSumAllReduce(const unsigned value, unsigned &reduction) {
#ifdef USE_MPI
  scalarAllReduce(value, reduction, MPI_UNSIGNED, MPI_SUM);
#else
  scalarAllReduce(value, reduction);
#endif // USE_MPI
}

void doubleMinAllReduce(const double value, double &reduction) {
#ifdef USE_MPI
  scalarAllReduce(value, reduction, MPI_DOUBLE, MPI_MIN);
#else
  scalarAllReduce(value, reduction);
#endif // USE_MPI
}
