#include "../../includes/parallel/allgather.h"

void matrixUnsignedAllgather(const std::vector<std::vector<unsigned>> &send_buffers, std::vector<std::vector<unsigned>> &recv_buffers, const int size) {
  matrixAllgather(send_buffers, recv_buffers, size, MPI_UNSIGNED);
}
