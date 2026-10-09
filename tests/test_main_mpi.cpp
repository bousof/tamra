// test_main_mpi.cpp
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
#include <iostream>
#include <tamra/parallel/wrapper.h>


int main(int argc, char** argv) {
  mpi_init(&argc, &argv);   // sets rank, size, etc.

  doctest::Context ctx;
  ctx.applyCommandLine(argc, argv);

  // You can constrain which tests run, e.g. only [mpi] tags:
  // ctx.setOption("test-case-exclude", "[serial]");

  const unsigned rank = mpi_rank();
  // Keep diagnostics from failing non-root ranks, suppress only their success output.
  ctx.setOption("quiet", false);
  ctx.setOption("minimal", rank != 0);
  ctx.setOption("success", false);   // show OK tests
  ctx.setOption("duration", rank == 0);
  ctx.setOption("order-by", "name"); // optional

  int res = ctx.run();
  if (res != 0)
    std::cerr << "TAMRA tests failed on MPI rank " << rank << '\n';

  // Keep test-runner status independent of the reduction wrappers under test.
  const int failed = res != 0;
  int any_failed = 0;
  MPI_Allreduce(&failed, &any_failed, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  mpi_finalize();
  return any_failed;
}
