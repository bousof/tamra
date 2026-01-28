/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI Allgather operations to hanlde STL vectors.
 */

#pragma once

#ifdef USE_MPI

#include <mpi.h>
#include "mpitypes.h"

#endif // USE_MPI

#include <functional>
#include <iostream>
#include <numeric>
#include <vector>

//-----------------------------------------------------------//
//  PROTOTYPES                                               //
//-----------------------------------------------------------//

template<typename T>
void scalarAllgather(const T &value, std::vector<T> &recv_buffer, const unsigned size);

template<typename T>
void matrixAllgather(const std::vector<std::vector<T>> &send_buffers, std::vector<std::vector<T>> &recv_buffers, const unsigned size);

//-----------------------------------------------------------//
//  LOWER LEVEL METHODS                                      //
//-----------------------------------------------------------//

namespace allgather::detail {

#ifdef USE_MPI

template<typename T>
void scalarAllgatherT(const T &value, std::vector<T> &recv_buffer, const unsigned size, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, int>::value,
    "scalarAllgatherT only supports T = int"
  );

	// Communication of between all processors
  recv_buffer.resize(size);
	MPI_Allgather(&value, 1, data_type, recv_buffer.data(), 1, data_type, MPI_COMM_WORLD);
}

#else

template<typename T>
void scalarAllgatherT(const T &value, std::vector<T> &recv_buffer, const unsigned) {
  static_assert(
    std::is_same<T, int>::value,
    "scalarAllgatherT only supports T = int"
  );

	// No MPI, so one proc then only gather from itself
  recv_buffer = { value };
}

#endif // USE_MPI

#ifdef USE_MPI

template<typename T>
void vectorAllgatherT(std::vector<T> &send_buffer, std::vector<T> &recv_buffer, const unsigned size, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "vectorAllgatherT only supports T = bool, double, or unsigned"
  );

  // Number of elements to gather to all processes
  int count = send_buffer.size();

	// Communication between all processors
  recv_buffer.resize(size * count);
	MPI_Allgather(send_buffer.data(), count, data_type, recv_buffer.data(), count, data_type, MPI_COMM_WORLD);
}

template<typename T>
void matrixAllgatherT(const std::vector<std::vector<T>> &send_buffers, std::vector<std::vector<T>> &recv_buffers, const unsigned size, const MPI_Datatype data_type) {
  unsigned rowCount = send_buffers.size(),
           colCount = send_buffers[0].size();
  // Call the merging all to all function
	std::vector<T> send_buffer, recv_buffer;
  send_buffer.reserve(rowCount * colCount);
  for (unsigned i{0}; i<rowCount; ++i)
    send_buffer.insert(send_buffer.end(), send_buffers[i].begin(), send_buffers[i].end());
  vectorAllgatherT(send_buffer, recv_buffer, size, data_type);

  // Split received data from each processor
  recv_buffers.resize(size * rowCount);
  for (unsigned p{0}; p<size; ++p)
    for (unsigned i=0; i<rowCount; ++i) {
      unsigned j = p*rowCount+i;
      recv_buffers[j].assign(recv_buffer.begin() + j*colCount, recv_buffer.begin() + (j+1)*colCount);
    }
}

#else

template<typename T>
void vectorAllgatherT(std::vector<T> &send_buffer, std::vector<T> &recv_buffer, const unsigned) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "vectorAllgatherT only supports T = bool, double, or unsigned"
  );

  // No MPI, so one proc then only gather from itself
  recv_buffer = send_buffer;
}

template<typename T>
void matrixAllgatherT(const std::vector<std::vector<T>> &send_buffers, std::vector<std::vector<T>> &recv_buffers, const unsigned) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "vectorAllgatherT only supports T = bool, double, or unsigned"
  );

  // No MPI, so one proc then only gather from itself
  recv_buffers = send_buffers;
}

#endif // USE_MPI

} // namespace allgather::detail

//-----------------------------------------------------------//
//  IMPLEMENTATIONS                                          //
//-----------------------------------------------------------//

template<typename T>
void scalarAllgather(const T &value, std::vector<T> &recv_buffer, const unsigned size) {
  static_assert(
    std::is_same<T, int>::value,
    "scalarAllgather only supports T = int"
  );

#ifdef USE_MPI
  allgather::detail::scalarAllgatherT(value, recv_buffer, size, mpi_type<T>());
#else
  allgather::detail::scalarAllgatherT(value, recv_buffer, size);
#endif // USE_MPI
}

template<typename T>
void matrixAllgather(const std::vector<std::vector<T>> &send_buffers, std::vector<std::vector<T>> &recv_buffers, const unsigned size) {
  static_assert(
    std::is_same<T, unsigned>::value,
    "matrixAllgather only supports T = unsigned"
  );

#ifdef USE_MPI
  allgather::detail::matrixAllgatherT(send_buffers, recv_buffers, size, mpi_type<T>());
#else
  allgather::detail::matrixAllgatherT(send_buffers, recv_buffers, size);
#endif // USE_MPI
}
