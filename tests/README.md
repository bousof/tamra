# Building and running the TAMRA tests

Run the commands in this guide from the **TAMRA repository root**, the directory
containing `CMakeLists.txt`, `includes/`, `src/`, and `tests/`. If the repository
is embedded in `TAMRA_framework/`, enter its `tamra/` directory first.

The [main README](../README.md#getting-started) lists prerequisites and dependency
options. Tests require doctest; version 2.4.12 is the tested baseline. MPI tests
also require MPI headers, libraries, and a matching launcher.

## Build and run the serial suite

```sh
cmake -S . -B build-serial \
  -DUSE_MPI=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-serial --parallel 2
(cd build-serial && ctest --output-on-failure)
```

This configuration does not require MPI. It builds `tamra_tests`, including
tests of the serial implementations of the communication API.

`--parallel 2` selects compilation jobs. It has no effect on MPI rank counts.
The CTest commands in this guide run from inside the build directory and are
compatible with the project's CMake 3.16 minimum. Build the executables before
running CTest; an ordinary CTest run does not compile them.

## Build and run the MPI suite

Use a separate build directory so the serial and MPI configurations remain
available independently:

```sh
cmake -S . -B build-mpi \
  -DUSE_MPI=ON -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-mpi --parallel 2
(cd build-mpi && ctest --output-on-failure)
```

The following CTest entries are registered when MPI and tests are enabled:

| CTest name | MPI ranks | Executable |
| --- | --- | --- |
| `tamra_serial_tests` | Not launched through MPI | `tamra_tests` |
| `tamra_parallel_1_rank` | 1 | `tamra_mpi_tests` |
| `tamra_parallel_2_rank` | 2 | `tamra_mpi_tests` |
| `tamra_parallel_3_rank` | 3 | `tamra_mpi_tests` |
| `tamra_parallel_tests` | 4 | `tamra_mpi_tests` |
| `tamra_parallel_5_rank` | 5 | `tamra_mpi_tests` |

Every MPI entry runs the complete MPI executable's test suite. The four-rank
entry retains its original name, `tamra_parallel_tests`.

The `tamra_tests` executable in an MPI build does not initialize MPI and omits
the shared communication tests. Those tests run in `tamra_mpi_tests` instead.
Consequently, running `tamra_serial_tests` in an MPI build does **not** replace
testing a separate `USE_MPI=OFF` build. A one-rank MPI run also exercises MPI,
rather than the serial fallback implementation.

MPI entries have a 120-second timeout and a CTest `PROCESSORS` value equal to
their rank count. `PROCESSORS` accounts for scheduling resources; the rank count
is set by the MPI launch command registered in `tests/CMakeLists.txt`.

## Select a registered MPI rank count

List the available CTest entries:

```sh
(cd build-mpi && ctest -N)
```

Run the complete suite with three ranks:

```sh
(cd build-mpi && ctest --output-on-failure -R '^tamra_parallel_3_rank$')
```

Run the four-rank entry:

```sh
(cd build-mpi && ctest --output-on-failure -R '^tamra_parallel_tests$')
```

Run all registered MPI entries:

```sh
(cd build-mpi && ctest --output-on-failure -R '^tamra_parallel')
```

The `-R` expression matches **CTest entry names**, not individual doctest case
names. `ctest -j N` controls concurrent CTest execution; it does not change the
number of MPI ranks inside an entry. Likewise, `MPIEXEC_MAX_NUMPROCS` does not
override the explicit rank counts in the current TAMRA test registrations.

## Run another rank count or filter test cases

For a rank count not registered with CTest, launch the MPI executable directly.
For example, with six ranks:

```sh
mpiexec -n 6 ./build-mpi/tests/tamra_mpi_tests
```

Use the launcher belonging to the MPI installation used at configuration time.
On a cluster, run within an allocation that permits the requested rank count.
For a persistent CTest entry, add the desired count to the registration loop in
`tests/CMakeLists.txt`; four ranks already have a separate entry.

To run only test cases whose names begin with `[parallel]`:

```sh
./build-serial/tests/tamra_tests '--test-case=[parallel]*'
mpiexec -n 3 ./build-mpi/tests/tamra_mpi_tests '--test-case=[parallel]*'
```

Keep the filter quoted so the shell does not expand its wildcard. Use identical
filters and test ordering on every MPI rank so collective operations match.

List the serial executable's individual test cases:

```sh
./build-serial/tests/tamra_tests --list-test-cases
```

These executable paths apply to standalone builds with a single-configuration
generator such as Make or Ninja. With Xcode or another multi-configuration
generator, use `cmake --build <build-directory> --config Release` and
`ctest -C Release` in that directory. Executables may be in an additional
`Release/` subdirectory; CTest resolves their paths automatically.

## Select the MPI installation and launcher options

If CMake does not select the intended MPI installation, provide its compiler
wrapper and launcher explicitly when creating the build directory:

```sh
cmake -S . -B build-mpi \
  -DUSE_MPI=ON -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release \
  -DMPI_CXX_COMPILER="/path/to/mpi/bin/mpicxx" \
  -DMPIEXEC_EXECUTABLE="/path/to/mpi/bin/mpiexec"
```

Replace the example paths with the actual installation paths. If changing an
already configured compiler or MPI installation, use a fresh build directory.

CTest's MPI commands use `MPIEXEC_EXECUTABLE`, `MPIEXEC_NUMPROC_FLAG`,
`MPIEXEC_PREFLAGS`, and `MPIEXEC_POSTFLAGS` from CMake's
[FindMPI module](https://cmake.org/cmake/help/v3.16/module/FindMPI.html).
Launcher flag lists are semicolon-separated CMake lists. Configure them according
to your MPI implementation and cluster environment. Manual `mpiexec` commands
do not automatically inherit these cached CMake settings.

The full registered suite needs permission to launch up to five ranks. If your
machine or allocation provides fewer slots, select an appropriate entry with
`ctest -R`, or request a larger allocation. For local oversubscription, consult
your MPI implementation's launcher options.

## Build through an existing parent workspace

A parent project may build TAMRA through `add_subdirectory(tamra)`. It should
call `include(CTest)` after its own `project()` declaration so CTest discovers
the tests from the parent build directory.

Configure, build, and run CTest from that workspace using the same CMake options.
When the parent uses `add_subdirectory(tamra)` without a custom binary directory,
direct executable paths include the extra `tamra/` component:

- `build-serial/tamra/tests/tamra_tests`
- `build-mpi/tamra/tests/tamra_mpi_tests`

CTest entry names remain the same. The commands earlier in this guide use the
standalone repository layout.

## Diagnose failures

Show the launcher command and full output for a particular entry:

```sh
(cd build-mpi && ctest -V -R '^tamra_parallel_3_rank$')
```

CTest also records output in `Testing/Temporary/LastTest.log` inside the build
directory. After correcting a failure and rebuilding, rerun failed entries with:

```sh
(cd build-mpi && ctest --rerun-failed --output-on-failure)
```

If CTest reports no tests, check that you are in the correct build directory,
configure with `BUILD_TESTING=ON`, and check the parent CTest setup when building
inside another project. A missing executable requires a build before testing.

The MPI test main keeps failure diagnostics from all ranks and combines normal
test failure results before returning. A crash or mismatched collective can
instead cause an MPI abort or timeout.

### Enable standard-library bounds assertions

With GCC/libstdc++, a separate debug build can enable additional container
checks:

```sh
cmake -S . -B build-debug \
  -DUSE_MPI=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS=-D_GLIBCXX_ASSERTIONS
cmake --build build-debug --parallel 2
(cd build-debug && ctest --output-on-failure)
```

For MPI, use a separate `build-mpi-debug` directory with `USE_MPI=ON` and the
same assertion flag. Preserve any other `CMAKE_CXX_FLAGS` you need when setting
this option. `_GLIBCXX_ASSERTIONS` is specific to libstdc++; it does not enable
equivalent checks for libc++, commonly used by Apple Clang.

## Test coverage

| Area | Examples of behavior checked |
| --- | --- |
| [Mesh primitives](mesh/) | Cells, sibling groups, neighbors, owned/remote/boundary flags, and owned/ghost leaf counts |
| [Adaptation](mesh/adaptation/) | Refinement, coarsening, and minimum-level meshing |
| [Traversal](mesh/traversal/) | Morton/Hilbert traversal, cell IDs, uneven partition cuts, and coverage in 1D, 2D, and 3D |
| [Distributed meshes](mesh/distributed/) | Partition coverage, empty ranks, ghost exchanges, load-balance decisions, and migration |
| [Communication](parallel/) | Serial/MPI wrappers, reductions, broadcasts, gathers, all-to-all exchanges, and serialized payloads |
| [Snapshots](io/) | Serialization round trips, partition restoration, merging, and redistribution |
| [Linear algebra](linear_algebra/) | Serial and MPI Jacobi solves |
| [Utilities](utils/) | Prefix sums, vector splitting, and `linspace` edge cases |

Tests use numerical tolerances where appropriate. The suite covers selected
framework behavior; application-specific numerical validation belongs in the
application's own tests.
