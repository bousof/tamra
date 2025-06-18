#include "../../includes/parallel/bcast.h"

void boolBCast(bool &value, const int root) {
  scalarBCast(value, root, MPI_C_BOOL);
}

void intBCast(int &value, const int root) {
  scalarBCast(value, root, MPI_INT);
}

void vectorUnsignedBCast(std::vector<unsigned> &buffer, const int root, const int rank, int count) {
  vectorBCast(buffer, root, rank, MPI_UNSIGNED, count);
}

void vectorDoubleBCast(std::vector<double> &buffer, const int root, const int rank, int count) {
  vectorBCast(buffer, root, rank, MPI_DOUBLE, count);
}

void matrixUnsignedBCast(std::vector< std::vector<unsigned> > &buffer, const int root, const int rank, int rowCount, int colCount) {
  matrixBCast(buffer, root, rank, MPI_UNSIGNED, rowCount, colCount);
}
