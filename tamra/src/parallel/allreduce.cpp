#include "../../includes/parallel/allreduce.h"

void boolAndAllReduce(const bool value, bool& reduction) {
  scalarAllReduce(value, reduction, MPI_C_BOOL, MPI_LAND);
}

void unsignedSumAllReduce(const unsigned value, unsigned& reduction) {
  scalarAllReduce(value, reduction, MPI_UNSIGNED, MPI_SUM);
}

void doubleMinAllReduce(const double value, double &reduction) {
  scalarAllReduce(value, reduction, MPI_DOUBLE, MPI_MIN);
}
