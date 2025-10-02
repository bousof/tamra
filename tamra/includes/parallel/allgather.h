/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI Allgather operations to hanlde STL vectors.
 */

#pragma once

#include <mpi.h>

#include <functional>
#include <iostream>
#include <numeric>
#include <vector>
#include"ParallelData.h"

template<typename T>
void scalarAllgather(const T &value, std::vector<T> &recv_buffer, const int size, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, int>::value,
    "scalarAllgather only supports T = int"
  );

	// Communication of between all processors
  recv_buffer.resize(size);
	MPI_Allgather(&value, 1, data_type, recv_buffer.data(), 1, data_type, MPI_COMM_WORLD);
}

void intAllgather(const int &value, std::vector<int> &recv_buffer, const int size);

template<typename T>
void vectorAllgather(std::vector<T> &send_buffer, std::vector<T> &recv_buffer, const int size, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "vectorAllgather only supports T = bool, double, or unsigned"
  );

  // Number of elements to gather to all processes
  int count = send_buffer.size();

	// Communication between all processors
  recv_buffer.resize(size * count);
	MPI_Allgather(send_buffer.data(), count, data_type, recv_buffer.data(), count, data_type, MPI_COMM_WORLD);
}

template<typename T>
void matrixAllgather(const std::vector<std::vector<T>> &send_buffers, std::vector<std::vector<T>> &recv_buffers, const int size, const MPI_Datatype data_type) {
  unsigned rowCount = send_buffers.size(),
           colCount = send_buffers[0].size();
  // Call the merging all to all function
	std::vector<T> send_buffer, recv_buffer;
  send_buffer.reserve(rowCount * colCount);
  for (int i{0}; i<rowCount; ++i)
    send_buffer.insert(send_buffer.end(), send_buffers[i].begin(), send_buffers[i].end());
  vectorAllgather(send_buffer, recv_buffer, size, data_type);

  // Split received data from each processor
  recv_buffers.resize(size * rowCount);
  for (int p{0}, i, j; p<size; ++p)
    for (i=0; i<rowCount; ++i) {
      j = p*rowCount+i;
      recv_buffers[j].assign(recv_buffer.begin() + j*colCount, recv_buffer.begin() + (j+1)*colCount);
    }
}

void matrixUnsignedAllgather(const std::vector<std::vector<unsigned>> &send_buffers, std::vector<std::vector<unsigned>> &recv_buffers, const int size);
