/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI Allgatherv operations to hanlde STL vectors.
 */

#pragma once

#ifdef USE_MPI
  #include <mpi.h>
#endif // USE_MPI

#include <functional>
#include <numeric>
#include <vector>
#include"ParallelData.h"
#include"./allgather.h"
#include"../utils/array_utils.h"

std::vector<int> vectorDoubleAllgatherv(const std::vector<double> &send_buffer, std::vector<double> &recv_buffer, const unsigned size);

std::vector<int> vectorUnsignedAllgatherv(const std::vector<unsigned> &send_buffer, std::vector<unsigned> &recv_buffer, const unsigned size);

using ParallelDataFactory = std::function<std::unique_ptr<ParallelData>()>;
void vectorDataAllgatherv(const std::vector<std::unique_ptr<ParallelData>> &send_buffer, std::vector<std::unique_ptr<ParallelData>> &recv_buffer, const unsigned size, const ParallelDataFactory createData);

#ifdef USE_MPI

template<typename T>
std::vector<int> vectorAllgatherv(const std::vector<T> &send_buffer, std::vector<T> &recv_buffer, const unsigned size, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, unsigned>::value || std::is_same<T, double>::value,
    "vectorAllgatherv only supports T = unsigned, or double"
  );

	// Number of elements to send to from this process
	int send_count = send_buffer.size();

	// Number of values to receive from the other processes
	std::vector<int> recv_counts(size);
	intAllgather(send_count, recv_counts, size);

	// Total number of cells to send and receive
  std::vector<int> recv_displacements;
  int tot_receive = cumulative_sum(recv_counts, recv_displacements, true);

	// Communication of cells IDs between all processors
	recv_buffer.resize(tot_receive);
	MPI_Allgatherv(send_buffer.data(), send_count        , data_type,
				         recv_buffer.data(), recv_counts.data(), recv_displacements.data(), data_type, MPI_COMM_WORLD);

  return recv_displacements;
}

template<typename T>
void vectorAllgatherv(const std::vector<T> &send_buffer, std::vector<std::vector<T>> &recv_buffers, const unsigned size, const MPI_Datatype data_type) {
  // Call the merging all to all function
	std::vector<T> recv_buffer;
  std::vector<int> recv_displacements = vectorAllgatherv(send_buffer, recv_buffer, size, data_type);

  // Split received data from each processor
  split(recv_buffer, recv_buffers, recv_displacements);
}

#else

template<typename T>
std::vector<int> vectorAllgatherv(const std::vector<T> &send_buffer, std::vector<T> &recv_buffer, const unsigned) {
  static_assert(
    std::is_same<T, unsigned>::value || std::is_same<T, double>::value,
    "vectorAllgatherv only supports T = unsigned, or double"
  );

  // No MPI, so one proc then only gather from itself
	recv_buffer = send_buffer;

  return { 0 };
}

template<typename T>
void vectorAllgatherv(const std::vector<T> &send_buffer, std::vector<std::vector<T>> &recv_buffers, const unsigned) {
  // Call the merging all to all function
	std::vector<T> recv_buffer;
  std::vector<int> recv_displacements = vectorAllgatherv(send_buffer, recv_buffer);

  // Split received data from each processor
  split(recv_buffer, recv_buffers, recv_displacements);
}

#endif // USE_MPI
