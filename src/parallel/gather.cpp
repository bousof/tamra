#include "../../includes/parallel/gather.h"

void doubleGather(double &value, std::vector<double> &buffer, const unsigned root, const unsigned rank, const unsigned size) {
#ifdef USE_MPI
  gather(value, buffer, root, rank, size, MPI_DOUBLE);
#else
  gather(value, buffer, root, rank, size);
#endif // USE_MPI
}
