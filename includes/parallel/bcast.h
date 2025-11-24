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
#endif // USE_MPI

#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include "ParallelData.h"

#ifdef USE_MPI

template<typename T>
void scalarBcast(T &value, const int root, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, unsigned>::value,
    "scalarBcast only supports T = bool, and unsigned"
  );

	// Communication of cells IDs between all processors
	MPI_Bcast(&value, 1, data_type, root, MPI_COMM_WORLD);
}

#else

template<typename T>
void scalarBcast(T &value, const int root) {
  (void)value; (void)root; // Unused

  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, unsigned>::value,
    "scalarBcast only supports T = bool, and unsigned"
  );

	// No MPI, so one proc then nothing to do
}

#endif // USE_MPI

void boolBcast(bool &value, const unsigned root);

void unsignedBcast(unsigned &value, const unsigned root);

void stringBcast(std::string &value, const unsigned root);

#ifdef USE_MPI

template<typename T>
void vectorBcast(std::vector<T> &buffer, const unsigned root, const unsigned rank, const MPI_Datatype data_type, unsigned count = 0) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "vectorBcast only supports T = double, and unsigned"
  );

	// Number of elements to broadcast to all processes
  if (count == 0) {
    if (root == rank)
      count = buffer.size();
    unsignedBcast(count, root);
  }

	// Communication of cells IDs between all processors
	buffer.resize(count);
	MPI_Bcast(buffer.data(), count, data_type, root, MPI_COMM_WORLD);
}

template<typename T>
void matrixBcast(std::vector<std::vector<T>> &buffer, const unsigned root, const unsigned rank, const MPI_Datatype data_type, unsigned rowCount = 0, unsigned colCount = 0) {
	// Number of elements to broadcast to all processes
  if (colCount == 0) {
    if (rank == root)
      colCount = buffer[0].size();
    unsignedBcast(colCount, root);
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
  vectorBcast(vector_buffer, root, rank, data_type, count);

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
void vectorBcast(std::vector<T> &, const unsigned, const unsigned, unsigned = 0) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "vectorBcast only supports T = double, and unsigned"
  );

	// No MPI, so one proc then nothing to do
}

template<typename T>
void matrixBcast(std::vector<std::vector<T>> &, const unsigned, const unsigned, unsigned = 0, unsigned = 0) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "matrixBcast only supports T = double, and unsigned"
  );

	// No MPI, so one proc then nothing to do
}

#endif // USE_MPI

void vectorUnsignedBcast(std::vector<unsigned> &buffer, const unsigned root, const unsigned rank, unsigned count = 0);

void vectorDoubleBcast(std::vector<double> &buffer, const unsigned root, const unsigned rank, unsigned count = 0);

void matrixUnsignedBcast(std::vector<std::vector<unsigned>> &buffer, const unsigned root, const unsigned rank, unsigned rowCount = 0, unsigned colCount = 0);
