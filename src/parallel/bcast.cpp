#include "../../includes/parallel/bcast.h"

void boolBcast(bool &value, const unsigned root) {
#ifdef USE_MPI
  scalarBcast(value, root, MPI_C_BOOL);
#else
  scalarBcast(value, root);
#endif // USE_MPI
}

void unsignedBcast(unsigned &value, const unsigned root) {
#ifdef USE_MPI
  scalarBcast(value, root, MPI_UNSIGNED);
#else
  scalarBcast(value, root);
#endif // USE_MPI
}

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

void vectorUnsignedBcast(std::vector<unsigned> &buffer, const unsigned root, const unsigned rank, unsigned count) {
#ifdef USE_MPI
  vectorBcast(buffer, root, rank, MPI_UNSIGNED, count);
#else
  vectorBcast(buffer, root, rank, count);
#endif // USE_MPI
}

void vectorDoubleBcast(std::vector<double> &buffer, const unsigned root, const unsigned rank, unsigned count) {
#ifdef USE_MPI
  vectorBcast(buffer, root, rank, MPI_DOUBLE, count);
#else
  vectorBcast(buffer, root, rank, count);
#endif // USE_MPI
}

void matrixUnsignedBcast(std::vector<std::vector<unsigned>> &buffer, const unsigned root, const unsigned rank, unsigned rowCount, unsigned colCount) {
#ifdef USE_MPI
  matrixBcast(buffer, root, rank, MPI_UNSIGNED, rowCount, colCount);
#else
  matrixBcast(buffer, root, rank, rowCount, colCount);
#endif // USE_MPI
}
