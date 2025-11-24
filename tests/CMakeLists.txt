# Shared test sources (used in both serial + MPI test executables)
set(TAMRA_TESTS_SRC
  # add test files here
  core/tests/manager/serial_test_core_manager_cell_id.cpp
  core/tests/manager/serial_test_core_manager_coarse.cpp
  core/tests/manager/serial_test_core_manager_min_level.cpp
  core/tests/manager/serial_test_core_manager_refine.cpp
  core/tests/serial_test_core_cell_oct.cpp
  core/tests/serial_test_core_neighbor.cpp
  core/tests/serial_test_core_tree.cpp
  core/tests/serial_test_core_tree_iterator.cpp
  linear_algebra/tests/serial_test_jacobi.cpp
)

add_executable(tamra_tests
  test_main.cpp              # serial doctest main
  ${TAMRA_TESTS_SRC}         # shared tests
)

target_include_directories(tamra_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/third_party/doctest
  ${CMAKE_CURRENT_SOURCE_DIR}/utils/includes
)

target_link_libraries(tamra_tests PRIVATE tamra_core)

add_test(NAME tamra_serial_tests COMMAND tamra_tests)

if(USE_MPI)
  add_executable(tamra_mpi_tests
    test_main_mpi.cpp        # serial doctest main
    ${TAMRA_TESTS_SRC}       # shared tests
    # add MPI test files here
    communications/tests/mpi_test_communications_alltoall.cpp
    communications/tests/mpi_test_communications_bcast.cpp
    core/tests/manager/mpi_test_core_manager_balance.cpp
    core/tests/manager/mpi_test_core_manager_ghost.cpp
    core/tests/manager/mpi_test_core_manager_min_level.cpp
    linear_algebra/tests/mpi_test_jacobi.cpp
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
