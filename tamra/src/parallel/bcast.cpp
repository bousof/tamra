#include "../../includes/parallel/bcast.h"

void vectorUnsignedBCast(std::vector<unsigned> &buffer, const int root, const int rank, int count) {
  vectorBCast(buffer, root, rank, MPI_UNSIGNED, count);
}

void matrixUnsignedBCast(std::vector< std::vector<unsigned> > &buffer, const int root, const int rank, int rowCount, int colCount) {
  matrixBCast(buffer, root, rank, MPI_UNSIGNED, rowCount, colCount);
}
