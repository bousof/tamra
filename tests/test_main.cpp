// test_main_mpi.cpp
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
#include <parallel/wrapper.h>  // your mpi_init / mpi_finalize

int main(int argc, char** argv) {
  doctest::Context ctx;
  ctx.applyCommandLine(argc, argv);

  // You can constrain which tests run, e.g. only [mpi] tags:
  // ctx.setOption("test-case-exclude", "[serial]");

  ctx.setOption("quiet", false);
  ctx.setOption("success", false);   // show OK tests
  ctx.setOption("duration", true);  // show timings
  ctx.setOption("order_by", "name"); // optional

  int res = ctx.run();

  return res;
}
