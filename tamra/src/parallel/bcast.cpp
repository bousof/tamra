#include "../../includes/parallel/bcast.h"

void boolBcast(bool &value, const int root) {
  scalarBcast(value, root, MPI_C_BOOL);
}

void unsignedBcast(unsigned &value, const int root) {
  scalarBcast(value, root, MPI_UNSIGNED);
}

void vectorUnsignedBcast(std::vector<unsigned> &buffer, const int root, const int rank, unsigned count) {
  vectorBcast(buffer, root, rank, MPI_UNSIGNED, count);
}

void vectorDoubleBcast(std::vector<double> &buffer, const int root, const int rank, unsigned count) {
  vectorBcast(buffer, root, rank, MPI_DOUBLE, count);
}

void matrixUnsignedBcast(std::vector< std::vector<unsigned> > &buffer, const int root, const int rank, unsigned rowCount, unsigned colCount) {
  matrixBcast(buffer, root, rank, MPI_UNSIGNED, rowCount, colCount);
}
