# Tests include <doctest.h>; installed copies commonly put it under include/doctest.
# HINTS preserves the existing workspace layout, while the cache variable allows
# another header location to be supplied with -DTAMRA_DOCTEST_INCLUDE_DIR=...
find_path(TAMRA_DOCTEST_INCLUDE_DIR
  NAMES doctest.h
  HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/doctest"
  PATH_SUFFIXES doctest
)
if(NOT TAMRA_DOCTEST_INCLUDE_DIR
   OR NOT EXISTS "${TAMRA_DOCTEST_INCLUDE_DIR}/doctest.h")
  message(FATAL_ERROR
    "TAMRA tests require doctest. Install doctest, set TAMRA_DOCTEST_INCLUDE_DIR "
    "to the directory containing doctest.h, or configure with BUILD_TESTING=OFF.")
endif()

# Serial-only test sources
set(TAMRA_SERIAL_TESTS_SRC
  # add serial tests here
  io/test_snapshot.cpp
  linear_algebra/test_jacobi.cpp
  mesh/adaptation/test_coarse.cpp
  mesh/adaptation/test_min_level.cpp
  mesh/adaptation/test_refine.cpp
  mesh/test_cell_oct.cpp
  mesh/test_neighbor.cpp
  mesh/test_ownership.cpp
  mesh/test_tree.cpp
  mesh/traversal/test_cell_id.cpp
  mesh/traversal/test_iterator_coverage.cpp
  mesh/traversal/test_tree_iterator.cpp
  utils/test_array_utils.cpp
)

# Run the same public communication API tests against serial fallbacks and MPI.
# The MPI-enabled tamra_tests executable has no MPI_Init, so these sources belong
# only in tamra_mpi_tests when USE_MPI is enabled.
set(TAMRA_COLLECTIVE_TESTS_SRC
  mesh/distributed/test_load_balance_decision.cpp
  parallel/test_allreduce.cpp
  parallel/test_alltoall.cpp
  parallel/test_bcast.cpp
  parallel/test_gather.cpp
  parallel/test_parallel_data.cpp
  parallel/test_wrapper.cpp
)

add_executable(tamra_tests
  test_main.cpp              # serial doctest main
  ${TAMRA_SERIAL_TESTS_SRC}
)

if(NOT USE_MPI)
  target_sources(tamra_tests PRIVATE ${TAMRA_COLLECTIVE_TESTS_SRC})
endif()

target_include_directories(tamra_tests PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/utils/includes"
)
target_include_directories(tamra_tests SYSTEM PRIVATE "${TAMRA_DOCTEST_INCLUDE_DIR}")

target_link_libraries(tamra_tests PRIVATE tamra_core)

add_test(NAME tamra_serial_tests COMMAND tamra_tests)

if(USE_MPI)
  set(TAMRA_MPI_TESTS_SRC
    # add mpi tests here
    io/test_snapshot_mpi.cpp
    linear_algebra/test_jacobi_mpi.cpp
    mesh/adaptation/test_min_level_mpi.cpp
    mesh/distributed/test_balance_mpi.cpp
    mesh/distributed/test_ghost_mpi.cpp
    mesh/distributed/test_partition_mpi.cpp
    parallel/test_alltoall_mpi.cpp
    parallel/test_bcast_mpi.cpp
  )

  add_executable(tamra_mpi_tests
    test_main_mpi.cpp        # MPI doctest main
    ${TAMRA_SERIAL_TESTS_SRC}
    ${TAMRA_COLLECTIVE_TESTS_SRC}
    ${TAMRA_MPI_TESTS_SRC}
  )

  target_include_directories(tamra_mpi_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/includes"
  )
  target_include_directories(tamra_mpi_tests SYSTEM PRIVATE "${TAMRA_DOCTEST_INCLUDE_DIR}")

  target_link_libraries(tamra_mpi_tests PRIVATE tamra_core)

  add_test(NAME tamra_parallel_tests
    COMMAND ${MPIEXEC_EXECUTABLE}
            ${MPIEXEC_NUMPROC_FLAG} 4
            ${MPIEXEC_PREFLAGS}
            $<TARGET_FILE:tamra_mpi_tests>
            ${MPIEXEC_POSTFLAGS}
  )
  set_tests_properties(tamra_parallel_tests PROPERTIES PROCESSORS 4 TIMEOUT 120)

  # Run the complete suite with single-rank MPI and uneven partition cuts too.
  # The four-rank entry above is retained for compatibility.
  foreach(rank_count IN ITEMS 1 2 3 5)
    add_test(NAME tamra_parallel_${rank_count}_rank
      COMMAND ${MPIEXEC_EXECUTABLE}
              ${MPIEXEC_NUMPROC_FLAG} ${rank_count}
              ${MPIEXEC_PREFLAGS}
              $<TARGET_FILE:tamra_mpi_tests>
              ${MPIEXEC_POSTFLAGS}
    )
    set_tests_properties(tamra_parallel_${rank_count}_rank
      PROPERTIES PROCESSORS ${rank_count} TIMEOUT 120)
  endforeach()
endif()
