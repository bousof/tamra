#include "../../includes/parallel/gather.h"

void doubleGather(double &value, std::vector<double> &buffer, const int root, const int rank, const int size) {
  gather(value, buffer, root, rank, size, MPI_DOUBLE);
}
