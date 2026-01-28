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
#include "mpitypes.h"

#endif // USE_MPI

#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include "ParallelData.h"

//-----------------------------------------------------------//
//  PROTOTYPES                                               //
//-----------------------------------------------------------//

template<typename T>
void scalarBcast(T &value, const unsigned root);

void stringBcast(std::string &value, const unsigned root);

template<typename T>
void vectorBcast(std::vector<T> &buffer, const unsigned root, const unsigned rank, unsigned count = 0);

template<typename T>
void matrixBcast(std::vector<std::vector<T>> &buffer, const unsigned root, const unsigned rank, unsigned rowCount = 0, unsigned colCount = 0);

//-----------------------------------------------------------//
//  LOWER LEVEL METHODS                                      //
//-----------------------------------------------------------//

namespace bcast::detail {

#ifdef USE_MPI

template<typename T>
void scalarBcastT(T &value, const int root, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, unsigned>::value,
    "scalarBcastT only supports T = bool, and unsigned"
  );

	// Communication of cells IDs between all processors
	MPI_Bcast(&value, 1, data_type, root, MPI_COMM_WORLD);
}

#else

template<typename T>
void scalarBcastT(T &value, const int root) {
  (void)value; (void)root; // Unused

  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, unsigned>::value,
    "scalarBcastT only supports T = bool, and unsigned"
  );

	// No MPI, so one proc then nothing to do
}

#endif // USE_MPI

#ifdef USE_MPI

template<typename T>
void vectorBcastT(std::vector<T> &buffer, const unsigned root, const unsigned rank, const MPI_Datatype data_type, unsigned count = 0) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "vectorBcastT only supports T = double, and unsigned"
  );

	// Number of elements to broadcast to all processes
  if (count == 0) {
    if (root == rank)
      count = buffer.size();
    scalarBcast<unsigned>(count, root);
  }

	// Communication of cells IDs between all processors
	buffer.resize(count);
	MPI_Bcast(buffer.data(), count, data_type, root, MPI_COMM_WORLD);
}

template<typename T>
void matrixBcastT(std::vector<std::vector<T>> &buffer, const unsigned root, const unsigned rank, const MPI_Datatype data_type, unsigned rowCount = 0, unsigned colCount = 0) {
	// Number of elements to broadcast to all processes
  if (colCount == 0) {
    if (rank == root)
      colCount = buffer[0].size();
    scalarBcast<unsigned>(colCount, root);
  }

  // Call the merging all to all function
	std::vector<T> vector_buffer;
  unsigned count = rowCount * colCount;
  if (rank == root) {
    rowCount = buffer.size();
    vector_buffer.resize(rowCount * colCount);
    for (unsigned i{0}, j; i<rowCount; ++i)
      for (j=0; j<colCount; ++j)
        vector_buffer[i * colCount + j] = buffer[i][j];
  }
  vectorBcastT(vector_buffer, root, rank, data_type, count);

  // Split received data from each processor
  if (rank != root) {
    count = vector_buffer.size();
    rowCount = count / colCount;
    buffer.resize(rowCount);
    for (unsigned i{0}; i<rowCount; ++i)
      buffer[i].assign(vector_buffer.begin() + i * colCount, vector_buffer.begin() + (i+1) * colCount);
  }
}

#else

template<typename T>
void vectorBcastT(std::vector<T> &, const unsigned, const unsigned, unsigned = 0) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "vectorBcastT only supports T = double, and unsigned"
  );

	// No MPI, so one proc then nothing to do
}

template<typename T>
void matrixBcastT(std::vector<std::vector<T>> &, const unsigned, const unsigned, unsigned = 0, unsigned = 0) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "matrixBcastT only supports T = double, and unsigned"
  );

	// No MPI, so one proc then nothing to do
}

#endif // USE_MPI

} // namespace bcast::detail

//-----------------------------------------------------------//
//  IMPLEMENTATIONS                                          //
//-----------------------------------------------------------//

template<typename T>
void scalarBcast(T &value, const unsigned root) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, unsigned>::value,
    "scalarBcast only supports T = bool, and unsigned"
  );

#ifdef USE_MPI
  bcast::detail::scalarBcastT(value, root, mpi_type<T>());
#else
  bcast::detail::scalarBcastT(value, root);
#endif // USE_MPI
}

template<typename T>
void vectorBcast(std::vector<T> &buffer, const unsigned root, const unsigned rank, unsigned count) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "vectorBcast only supports T = double, and unsigned"
  );

#ifdef USE_MPI
  bcast::detail::vectorBcastT(buffer, root, rank, mpi_type<T>(), count);
#else
  bcast::detail::vectorBcastT(buffer, root, rank, count);
#endif // USE_MPI
}

template<typename T>
void matrixBcast(std::vector<std::vector<T>> &buffer, const unsigned root, const unsigned rank, unsigned rowCount, unsigned colCount) {
  static_assert(
    std::is_same<T, unsigned>::value,
    "matrixBcast only supports T = unsigned"
  );

#ifdef USE_MPI
  bcast::detail::matrixBcastT(buffer, root, rank, mpi_type<T>(), rowCount, colCount);
#else
  bcast::detail::matrixBcastT(buffer, root, rank, rowCount, colCount);
#endif // USE_MPI
}
