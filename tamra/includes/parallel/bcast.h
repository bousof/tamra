/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI Bcast operations to hanlde STL vectors.
 */

#pragma once

#include <mpi.h>

#include <functional>
#include <iostream>
#include <numeric>
#include <vector>
#include"parallel_data.h"

template<typename T>
void vectorBCast(std::vector<T> &buffer, const int root, const int rank, const MPI_Datatype data_type, int count = 0) {
  static_assert(
    std::is_same<T, unsigned>::value,
    "vectorBCast only supports T = unsigned"
  );

	// Number of elements to broadcast to all processes
  if (count==0) {
    if (root==rank) {
      count = buffer.size();
    }
    MPI_Bcast(&count, 1, MPI_INT, root, MPI_COMM_WORLD);
  }

	// Communication of cells IDs between all processors
	buffer.resize(count);
	MPI_Bcast(buffer.data(), count, data_type, root, MPI_COMM_WORLD);
}

template<typename T>
void matrixBCast(std::vector< std::vector<T> > &buffer, const int root, const int rank, const MPI_Datatype data_type, int rowCount = 0, int colCount = 0) {
	// Number of elements to broadcast to all processes
  if (colCount==0) {
    if (rank == root)
      colCount = buffer[0].size();
    MPI_Bcast(&colCount, 1, MPI_INT, root, MPI_COMM_WORLD);
  }

  // Call the merging all to all function
	std::vector<T> vector_buffer;
  int count = rowCount * colCount;
  if (rank == root) {
    rowCount = buffer.size();
    vector_buffer.resize(rowCount * colCount);
    for (int i{0}, j; i<rowCount; ++i)
      for (j=0; j<colCount; ++j)
        vector_buffer[i * colCount + j] = buffer[i][j];
  }
  vectorBCast(vector_buffer, root, rank, data_type, count);

  // Split received data from each processor
  if (rank != root) {
    count = vector_buffer.size();
    rowCount = count / colCount;
    buffer.resize(rowCount);
    for (int i{0}; i<rowCount; ++i)
      buffer[i].assign(vector_buffer.begin() + i * colCount, vector_buffer.begin() + (i+1) * colCount);
  }
}

void vectorUnsignedBCast(std::vector<unsigned> &buffer, const int root, const int rank, int count = 0);

void matrixUnsignedBCast(std::vector< std::vector<unsigned> > &buffer, const int root, const int rank, int rowCount = 0, int colCount = 0);
