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
  mesh/traversal/test_tree_iterator.cpp
  utils/test_array_utils.cpp
)

# Run the same public communication API tests against serial fallbacks and MPI.
# The MPI-enabled tamra_tests executable has no MPI_Init, so these sources belong
# only in tamra_mpi_tests when USE_MPI is enabled.
set(TAMRA_COLLECTIVE_TESTS_SRC
  parallel/test_allreduce.cpp
  parallel/test_bcast.cpp
  parallel/test_gather.cpp
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
  ${CMAKE_SOURCE_DIR}/third_party/doctest
  ${CMAKE_CURRENT_SOURCE_DIR}/utils/includes
)

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
    parallel/test_alltoall_mpi.cpp
    parallel/test_bcast_mpi.cpp
  )

  add_executable(tamra_mpi_tests
    test_main_mpi.cpp        # MPI doctest main
    ${TAMRA_SERIAL_TESTS_SRC}
    ${TAMRA_MPI_TESTS_SRC}
  )

  target_include_directories(tamra_mpi_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/doctest
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/includes
  )

  target_link_libraries(tamra_mpi_tests PRIVATE tamra_core)

  add_test(NAME tamra_parallel_tests
    COMMAND ${MPIEXEC_EXECUTABLE}
            ${MPIEXEC_NUMPROC_FLAG} 4
            $<TARGET_FILE:tamra_mpi_tests>
            ${MPIEXEC_POSTFLAGS}
  )
  set_tests_properties(tamra_parallel_tests PROPERTIES PROCESSORS 4 TIMEOUT 120)

  # Single-rank MPI exercises MPI paths separately from serial fallbacks; two
  # ranks cover nonzero roots and asymmetric exchanges without mesh-test rank
  # assumptions. The complete suite above runs with four ranks.
  foreach(rank_count IN ITEMS 1 2)
    add_test(NAME tamra_collectives_${rank_count}_rank
      COMMAND ${MPIEXEC_EXECUTABLE}
              ${MPIEXEC_NUMPROC_FLAG} ${rank_count}
              ${MPIEXEC_PREFLAGS}
              $<TARGET_FILE:tamra_mpi_tests>
              ${MPIEXEC_POSTFLAGS}
              "--test-case=[parallel]*"
    )
    set_tests_properties(tamra_collectives_${rank_count}_rank
      PROPERTIES PROCESSORS ${rank_count} TIMEOUT 60)
  endforeach()
endif()
