/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI Bcast operations to hanlde STL vectors.
 */

#pragma once

#ifdef USE_MPI

#include <mpi.h>
#include <tamra/parallel/mpitypes.h>

#endif // USE_MPI

#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include <tamra/parallel/ParallelData.h>


//-----------------------------------------------------------//
//  PROTOTYPES                                               //
//-----------------------------------------------------------//

template<typename T>
void scalar_bcast(T &value, const unsigned root);

void string_bcast(std::string &value, const unsigned root);

template<typename T>
void vector_bcast(std::vector<T> &buffer, const unsigned root, const unsigned rank, unsigned count = 0);

template<typename T>
void matrix_bcast(std::vector<std::vector<T>> &buffer, const unsigned root, const unsigned rank, unsigned row_count = 0, unsigned col_count = 0);

//-----------------------------------------------------------//
//  LOWER LEVEL METHODS                                      //
//-----------------------------------------------------------//

namespace bcast::detail {

#ifdef USE_MPI

template<typename T>
void scalar_bcast_impl(T &value, const int root, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, unsigned>::value,
    "scalar_bcast_impl only supports T = bool, and unsigned"
  );

	// Communication of cells IDs between all processors
	MPI_Bcast(&value, 1, data_type, root, MPI_COMM_WORLD);
}

#else

template<typename T>
void scalar_bcast_impl(T &value, const int root) {
  (void)value; (void)root; // Unused

  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, unsigned>::value,
    "scalar_bcast_impl only supports T = bool, and unsigned"
  );

	// No MPI, so one proc then nothing to do
}

#endif // USE_MPI

#ifdef USE_MPI

template<typename T>
void vector_bcast_impl(std::vector<T> &buffer, const unsigned root, const unsigned rank, const MPI_Datatype data_type, unsigned count = 0) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "vector_bcast_impl only supports T = double, and unsigned"
  );

	// Number of elements to broadcast to all processes
  if (count == 0) {
    if (root == rank)
      count = buffer.size();
    scalar_bcast<unsigned>(count, root);
  }

	// Communication of cells IDs between all processors
	buffer.resize(count);
	MPI_Bcast(buffer.data(), count, data_type, root, MPI_COMM_WORLD);
}

template<typename T>
void matrix_bcast_impl(std::vector<std::vector<T>> &buffer, const unsigned root, const unsigned rank, const MPI_Datatype data_type, unsigned row_count = 0, unsigned col_count = 0) {
	// Number of elements to broadcast to all processes
  if (col_count == 0) {
    if (rank == root)
      col_count = buffer[0].size();
    scalar_bcast<unsigned>(col_count, root);
  }

  // Call the merging all to all function
	std::vector<T> vector_buffer;
  unsigned count = row_count * col_count;
  if (rank == root) {
    row_count = buffer.size();
    vector_buffer.resize(row_count * col_count);
    for (unsigned i{0}, j; i<row_count; ++i)
      for (j=0; j<col_count; ++j)
        vector_buffer[i * col_count + j] = buffer[i][j];
  }
  vector_bcast_impl(vector_buffer, root, rank, data_type, count);

  // Split received data from each processor
  if (rank != root) {
    count = vector_buffer.size();
    row_count = count / col_count;
    buffer.resize(row_count);
    for (unsigned i{0}; i<row_count; ++i)
      buffer[i].assign(vector_buffer.begin() + i * col_count, vector_buffer.begin() + (i+1) * col_count);
  }
}

#else

template<typename T>
void vector_bcast_impl(std::vector<T> &, const unsigned, const unsigned, unsigned = 0) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "vector_bcast_impl only supports T = double, and unsigned"
  );

	// No MPI, so one proc then nothing to do
}

template<typename T>
void matrix_bcast_impl(std::vector<std::vector<T>> &, const unsigned, const unsigned, unsigned = 0, unsigned = 0) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "matrix_bcast_impl only supports T = double, and unsigned"
  );

	// No MPI, so one proc then nothing to do
}

#endif // USE_MPI

} // namespace bcast::detail

//-----------------------------------------------------------//
//  IMPLEMENTATIONS                                          //
//-----------------------------------------------------------//

template<typename T>
void scalar_bcast(T &value, const unsigned root) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, unsigned>::value,
    "scalar_bcast only supports T = bool, and unsigned"
  );

#ifdef USE_MPI
  bcast::detail::scalar_bcast_impl(value, root, mpi_type<T>());
#else
  bcast::detail::scalar_bcast_impl(value, root);
#endif // USE_MPI
}

template<typename T>
void vector_bcast(std::vector<T> &buffer, const unsigned root, const unsigned rank, unsigned count) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "vector_bcast only supports T = double, and unsigned"
  );

#ifdef USE_MPI
  bcast::detail::vector_bcast_impl(buffer, root, rank, mpi_type<T>(), count);
#else
  bcast::detail::vector_bcast_impl(buffer, root, rank, count);
#endif // USE_MPI
}

template<typename T>
void matrix_bcast(std::vector<std::vector<T>> &buffer, const unsigned root, const unsigned rank, unsigned row_count, unsigned col_count) {
  static_assert(
    std::is_same<T, unsigned>::value,
    "matrix_bcast only supports T = unsigned"
  );

#ifdef USE_MPI
  bcast::detail::matrix_bcast_impl(buffer, root, rank, mpi_type<T>(), row_count, col_count);
#else
  bcast::detail::matrix_bcast_impl(buffer, root, rank, row_count, col_count);
#endif // USE_MPI
}
