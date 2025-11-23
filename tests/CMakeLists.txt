# Shared test sources (used in both serial + MPI test executables)
set(TAMRA_TESTS_SRC
  # add test files here
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
