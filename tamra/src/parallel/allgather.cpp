#include "../../includes/parallel/allgather.h"

void intAllgather(const int &value, std::vector<int> &recv_buffer, const int size) {
  scalarAllgather(value, recv_buffer, size, MPI_INT);
}

void matrixUnsignedAllgather(const std::vector<std::vector<unsigned>> &send_buffers, std::vector<std::vector<unsigned>> &recv_buffers, const int size) {
  matrixAllgather(send_buffers, recv_buffers, size, MPI_UNSIGNED);
}
