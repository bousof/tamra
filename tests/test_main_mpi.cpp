// test_main_mpi.cpp
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
#include <parallel/wrapper.h>  // your mpi_init / mpi_finalize

int main(int argc, char** argv) {
  mpi_init(&argc, &argv);   // sets rank, size, etc.

  doctest::Context ctx;
  ctx.applyCommandLine(argc, argv);

  // You can constrain which tests run, e.g. only [mpi] tags:
  // ctx.setOption("test-case-exclude", "[serial]");

  // Mute doctest on non-root ranks
  if (mpi_rank() != 0) {
    ctx.setOption("quiet", true);    // no console output for ranks != 0
  } else {
    ctx.setOption("quiet", false);
  }
  ctx.setOption("success", false);   // show OK tests
  ctx.setOption("duration", true);   // show timings
  ctx.setOption("order-by", "name"); // optional

  int res = ctx.run();

  mpi_finalize();
  return res;
}
