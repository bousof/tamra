cmake_minimum_required(VERSION 3.16)
project(TAMRA LANGUAGES CXX)

option(USE_MPI "Build with MPI support" ON)
include(CTest)

# Keep the same language and warning settings when built without the workspace.
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR
   AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  add_compile_options(
    -Wall -Wextra -Wpedantic -fdiagnostics-color=always
    -Wno-unused-but-set-variable
  )
endif()

if(USE_MPI)
  find_package(MPI REQUIRED COMPONENTS CXX)
endif()

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

# Template headers require C++17 in applications linking TAMRA as well.
target_compile_features(tamra_core PUBLIC cxx_std_17)

# TAMRA headers: available to the library and its users.
target_include_directories(tamra_core
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/includes"
)

# An explicit header directory supports offline builds and unpacked Eigen copies.
# Retain the existing workspace's bundled dependency when it is available.
set(TAMRA_EIGEN_INCLUDE_DIR "" CACHE PATH
  "Directory containing Eigen/ (Eigen 3.4.0 or newer)")
if(NOT TAMRA_EIGEN_INCLUDE_DIR AND NOT TARGET Eigen3::Eigen
   AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/eigen-3.4.0/Eigen/Core")
  set(TAMRA_EIGEN_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/eigen-3.4.0"
    CACHE PATH "Directory containing Eigen/ (Eigen 3.4.0 or newer)" FORCE)
endif()

if(TAMRA_EIGEN_INCLUDE_DIR)
  if(NOT EXISTS "${TAMRA_EIGEN_INCLUDE_DIR}/Eigen/Core")
    message(FATAL_ERROR
      "TAMRA_EIGEN_INCLUDE_DIR must point to a directory containing Eigen/Core.")
  endif()
  target_include_directories(tamra_core SYSTEM PUBLIC "${TAMRA_EIGEN_INCLUDE_DIR}")
else()
  if(NOT TARGET Eigen3::Eigen)
    find_package(Eigen3 3.4 REQUIRED NO_MODULE)
  endif()
  target_link_libraries(tamra_core PUBLIC Eigen3::Eigen)
endif()

if(USE_MPI)
  target_compile_definitions(tamra_core PUBLIC USE_MPI)
  target_link_libraries(tamra_core PUBLIC MPI::MPI_CXX)
endif()

# Tests
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
