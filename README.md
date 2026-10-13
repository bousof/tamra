<p align="center">
  <picture>
    <img src="assets/TAMRA_logo.png"
         alt="TAMRA — Toolkit for Adaptive Mesh Refinement Applications"
         width="600">
  </picture>
</p>

# TAMRA

**TAMRA** (**T**oolkit for **A**daptive **M**esh **R**efinement **A**pplications) is a modular **C++ HPC framework** designed to accelerate the development of **scalable adaptive numerical simulation codes**.

It provides the building blocks needed to add **adaptive mesh refinement (AMR)**, **distributed-memory parallelism**, and a flexible solver architecture to scientific applications, while keeping the numerical formulation of each solver as independent as possible.

TAMRA is designed for teams who want to:

- improve the scalability of existing simulation codes,
- reduce computational cost by refining only where needed,
- support both **Eulerian** and **Lagrangian** methods,
- build reusable multiphysics software instead of solver-specific prototypes.

---

## Why TAMRA

Large-scale simulations often face two major limitations:

1. **Computational cost grows too quickly** when the full domain is resolved uniformly.
2. **Codebases become difficult to maintain** when refinement, parallelization, and solver logic are too tightly coupled.

TAMRA addresses these issues by providing a framework where:

- **AMR reduces unnecessary computation** by concentrating resolution only in the important parts of the domain,
- **MPI-based domain decomposition improves scalability** on distributed-memory systems,
- solver-independent abstractions support **1D/2D/3D** meshes and provide a basis for multiple discretizations and multiphysics extensions.

The goal is not only to run simulations faster, but also to make advanced HPC simulation software easier to design, extend, and maintain.

---

## Main benefits

- **Computing acceleration through AMR**  
  Refine only where the physics requires it instead of using a uniformly fine mesh everywhere.

- **Improved scalability**  
  Built for distributed-memory architectures with MPI-based parallel execution.

- **Reusable software architecture**  
  Decouples mesh adaptation, parallel partitioning, and solver logic to simplify development of new solvers.

- **Support for multiple numerical paradigms**  
  Suitable for both **Eulerian grid-based methods** and **Lagrangian particle methods**.

- **Designed for research and industrial code evolution**  
  TAMRA can serve both as a basis for new simulation software and as a way to accelerate/refactor existing in-house codes.

---

## Target applications

TAMRA is intended for advanced scientific and engineering simulations, including for example:

- compressible and incompressible flow solvers,
- particle-based methods such as **SPH**,
- wave propagation solvers,
- multiphase and multiphysics simulations,
- research codes that need better scalability and adaptive resolution.

Examples of numerical approaches that can be integrated include:

- **Eulerian methods**: Roe-type finite-volume solvers, LBM, DG, finite differences,
- **Lagrangian methods**: SPH and related particle methods.

---

## Core ideas

TAMRA is being developed around a few core principles:

- **performance-oriented**
- **dimension-independent (1D / 2D / 3D)**
- **solver-agnostic**
- **modular and extensible**
- **adapted to large-scale distributed simulations**

---

## Current status

TAMRA is an active research and development project.

The repository contains the framework and automated serial and MPI tests.
Numerical solvers such as SPH and finite-volume applications are developed in
separate application projects. Small starter applications and tutorials will be
added progressively as the library develops.

Further material is planned, including:

- improved user documentation,
- architecture explanations,
- tutorials and examples,
- benchmark cases,
- guidance for integrating TAMRA into existing solvers.

**Documentation is still under active development and will be expanded soon.**

---

## Getting started

### Prerequisites

| Dependency | Requirement |
| --- | --- |
| C++ compiler | C++17 support |
| CMake | Version 3.16 or newer |
| Build tool | A tool supported by the selected CMake generator, such as Make or Ninja |
| Eigen | Version 3.4.0 or a compatible newer release; tested with 3.4.0 |
| doctest | Required when building tests; tested with version 2.4.12 |
| MPI | Headers, libraries, and an MPI launcher when `USE_MPI=ON` |

MPI support and tests are enabled by default. A serial build configured with
`USE_MPI=OFF` does not require MPI. A library-only build with
`BUILD_TESTING=OFF` does not require doctest.

Install the dependencies through your environment's package manager or provide
local copies as described below. CMake does not download dependencies.

### Build and test without MPI

Run these commands from the **repository root**: the directory containing
`CMakeLists.txt`, `includes/`, `src/`, and `tests/`. In a workspace containing the
library as a subdirectory, this is the `tamra/` directory.

```sh
cmake -S . -B build-serial \
  -DUSE_MPI=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-serial --parallel 2
(cd build-serial && ctest --output-on-failure)
```

The build produces the `tamra_core` static library and the `tamra_tests`
executable. CTest runs the tests and prints diagnostics if a test fails.
Adjust `--parallel 2` to select the number of concurrent compilation jobs.

### Build and test with MPI

Use a separate build directory for MPI:

```sh
cmake -S . -B build-mpi \
  -DUSE_MPI=ON -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-mpi --parallel 2
(cd build-mpi && ctest --output-on-failure)
```

This build provides both `tamra_tests` and `tamra_mpi_tests`. CTest runs
`tamra_tests` directly and launches `tamra_mpi_tests` with **1, 2, 3, 4, and 5
ranks**. The MPI launcher must be allowed to start up to five ranks to run every
registered test.

Both build configurations are useful: the serial build checks communication
fallbacks, while even a one-rank MPI run exercises the MPI implementation.
See the [test guide](tests/README.md) for selecting rank counts, filtering test
cases, and configuring the launcher.

The examples use a single-configuration generator such as Make or Ninja.
For a multi-configuration generator such as Xcode, build with `--config Release`
and pass `-C Release` to CTest instead of selecting `CMAKE_BUILD_TYPE`.

### Dependencies in other locations

For installed dependencies outside the default search locations, pass their
installation prefixes to CMake:

```sh
cmake -S . -B build-serial \
  -DUSE_MPI=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="/path/to/eigen-prefix;/path/to/doctest-prefix"
```

Eigen is discovered through its `Eigen3` CMake package. The doctest header is
located directly, including the usual `include/doctest/` installation layout.
You may also set `Eigen3_DIR` to the directory containing `Eigen3Config.cmake`.
See [Eigen's CMake documentation](https://libeigen.gitlab.io/eigen/docs-3.4/TopicCMakeGuide.html)
for its package layout.

For unpacked header copies, use the explicit directory options instead:

```sh
cmake -S . -B build-serial \
  -DUSE_MPI=OFF -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release \
  -DTAMRA_EIGEN_INCLUDE_DIR="/path/to/eigen-3.4.0" \
  -DTAMRA_DOCTEST_INCLUDE_DIR="/path/to/doctest-header-directory"
```

`TAMRA_EIGEN_INCLUDE_DIR` must contain `Eigen/Core`;
`TAMRA_DOCTEST_INCLUDE_DIR` must contain `doctest.h`. Replace the example paths
with your local directories, then run the build and CTest commands above.

An existing workspace may keep these dependencies at
`../third_party/eigen-3.4.0/` and `../third_party/doctest/`, relative to the TAMRA
repository. The CMake files recognize that layout. Those sibling directories
are not required for a standalone clone. An explicit Eigen header directory
takes precedence over package discovery; the workspace Eigen copy is also used
by default unless an `Eigen3::Eigen` target is already available. Dependency
locations are cached per build directory.

### Build only the library

```sh
cmake -S . -B build-lib \
  -DUSE_MPI=OFF -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build-lib --parallel 2
```

Set `USE_MPI=ON` for an MPI-enabled library. Eigen is required in either mode.

### CMake options

| Option | Default | Purpose |
| --- | --- | --- |
| `USE_MPI` | `ON` | Build and link MPI support |
| `BUILD_TESTING` | `ON` | Build test executables and register CTest tests |
| `TAMRA_EIGEN_INCLUDE_DIR` | Automatic selection | Use a specific Eigen header directory |
| `TAMRA_DOCTEST_INCLUDE_DIR` | Automatic selection | Use a specific doctest header directory |

### Use TAMRA in an application workspace

An application can include the repository with `add_subdirectory(tamra)` and
link its existing target against `tamra_core`:

```cmake
add_subdirectory(tamra)
target_link_libraries(my_application PRIVATE tamra_core)
```

Here `my_application` is a target created by your application project. Linking
TAMRA propagates its public header paths, Eigen dependency, C++17 requirement,
and MPI settings when enabled. The application supplies its own numerical model.

To run tests from the parent build directory, include `CTest` in the parent
`CMakeLists.txt` after `project()`. The same `BUILD_TESTING` option controls
whether TAMRA adds its tests. Keep MPI discovery in the parent as well if other
application targets use `MPI::MPI_CXX` directly.

---

## Documentation

Build and test instructions are available above and in the
[test guide](tests/README.md).

Further documentation is planned, including:

- framework overview,
- API documentation,
- examples,
- integration notes for external solvers,
- performance/scalability discussions.

---

## Contributing

Contributions are welcome.

If you are interested in:

- contributing code,
- improving documentation,
- testing the framework,
- discussing solver integration,
- adding benchmark cases,

please open an issue or submit a pull request.

**Contributors are very welcome**, especially from the scientific computing, CFD, AMR, and HPC communities.

---

## Collaboration and industrial/research interest

TAMRA may also be of interest to:

- **research labs** developing in-house simulation codes,
- **companies** looking to improve the scalability of existing numerical software,
- teams interested in adding **AMR**, **MPI scalability**, or a cleaner solver architecture to their current codebase.

If your lab or company is interested in using TAMRA, collaborating, or exploring how it could help accelerate or modernize an existing simulation code, feel free to get in touch.

---

## License

This project is licensed under the MIT License.
