#include "../../includes/parallel/alltoall.h"

void intAlltoall(const std::vector<int>& send_buffer, std::vector<int>& recv_buffer, const int size) {
  scalarAlltoall(send_buffer, recv_buffer, MPI_INT);
}
