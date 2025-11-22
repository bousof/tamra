#include "../../includes/parallel/alltoall.h"

void intAlltoall(const std::vector<int> &send_buffer, std::vector<int> &recv_buffer) {
#ifdef USE_MPI
  scalarAlltoall(send_buffer, recv_buffer, MPI_INT);
#else
  scalarAlltoall(send_buffer, recv_buffer);
#endif // USE_MPI
}
