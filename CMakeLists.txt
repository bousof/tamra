# All source files for the tamra core library
file(GLOB_RECURSE TAMRA_SRC CONFIGURE_DEPENDS
    src/*.cpp
)

# Template headers for the tamra core library
file(GLOB_RECURSE TAMRA_HEADERS CONFIGURE_DEPENDS
  includes/*.h
  includes/*.tpp
)

add_library(tamra_core STATIC
  ${TAMRA_SRC}
  ${TAMRA_HEADERS}   # headers are *listed*, but not compiled
)

# Public includes for everyone using tamra
target_include_directories(tamra_core
  PUBLIC
    ${CMAKE_SOURCE_DIR}/tamra/includes
  SYSTEM PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/eigen-3.4.0
)

if(USE_MPI)
  target_compile_definitions(tamra_core PUBLIC USE_MPI)
  target_link_libraries(tamra_core PUBLIC MPI::MPI_CXX)
endif()

# Tests
add_subdirectory(tests)
