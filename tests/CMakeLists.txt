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
  mesh/test_tree.cpp
  mesh/traversal/test_cell_id.cpp
  mesh/traversal/test_tree_iterator.cpp
)

add_executable(tamra_tests
  test_main.cpp              # serial doctest main
  ${TAMRA_SERIAL_TESTS_SRC}
)

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
endif()
