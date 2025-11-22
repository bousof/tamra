#include "../../includes/parallel/allgather.h"

void intAllgather(const int &value, std::vector<int> &recv_buffer, const unsigned size) {
#ifdef USE_MPI
  scalarAllgather(value, recv_buffer, size, MPI_INT);
#else
  scalarAllgather(value, recv_buffer, size);
#endif // USE_MPI
}

void matrixUnsignedAllgather(const std::vector<std::vector<unsigned>> &send_buffers, std::vector<std::vector<unsigned>> &recv_buffers, const unsigned size) {
#ifdef USE_MPI
  matrixAllgather(send_buffers, recv_buffers, size, MPI_UNSIGNED);
#else
  matrixAllgather(send_buffers, recv_buffers, size);
#endif // USE_MPI
}
