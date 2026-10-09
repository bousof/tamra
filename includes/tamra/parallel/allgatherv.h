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
#include <tamra/parallel/mpitypes.h>

#endif // USE_MPI

#include <functional>
#include <memory>
#include <numeric>
#include <vector>

#include <tamra/utils/array_utils.h>
#include <tamra/parallel/allgather.h>
#include <tamra/parallel/ParallelData.h>


//-----------------------------------------------------------//
//  PROTOTYPES                                               //
//-----------------------------------------------------------//

template<typename T>
std::vector<int> vector_allgatherv(const std::vector<T> &send_buffer, std::vector<T> &recv_buffer, const unsigned size);

using ParallelDataFactory = std::function<std::unique_ptr<ParallelData>()>;
void vector_data_allgatherv(const std::vector<std::unique_ptr<ParallelData>> &send_buffer, std::vector<std::unique_ptr<ParallelData>> &recv_buffer, const unsigned size, const ParallelDataFactory create_data);

//-----------------------------------------------------------//
//  LOWER LEVEL METHODS                                      //
//-----------------------------------------------------------//

namespace allgatherv::detail {

#ifdef USE_MPI

template<typename T>
std::vector<int> vector_allgatherv_impl(const std::vector<T> &send_buffer, std::vector<T> &recv_buffer, const unsigned size, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, unsigned>::value || std::is_same<T, double>::value,
    "vector_allgatherv_impl only supports T = unsigned, or double"
  );

	// Number of elements to send to from this process
	int send_count = send_buffer.size();

	// Number of values to receive from the other processes
	std::vector<int> recv_counts(size);
	scalar_allgather<int>(send_count, recv_counts, size);

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
void vector_allgatherv_impl(const std::vector<T> &send_buffer, std::vector<std::vector<T>> &recv_buffers, const unsigned size, const MPI_Datatype data_type) {
  // Call the merging all to all function
	std::vector<T> recv_buffer;
  std::vector<int> recv_displacements = vector_allgatherv_impl(send_buffer, recv_buffer, size, data_type);

  // Split received data from each processor
  split(recv_buffer, recv_buffers, recv_displacements);
}

#else

template<typename T>
std::vector<int> vector_allgatherv_impl(const std::vector<T> &send_buffer, std::vector<T> &recv_buffer, const unsigned) {
  static_assert(
    std::is_same<T, unsigned>::value || std::is_same<T, double>::value,
    "vector_allgatherv_impl only supports T = unsigned, or double"
  );

  // No MPI, so one proc then only gather from itself
	recv_buffer = send_buffer;

  return { 0 };
}

template<typename T>
void vector_allgatherv_impl(const std::vector<T> &send_buffer, std::vector<std::vector<T>> &recv_buffers, const unsigned) {
  // Call the merging all to all function
	std::vector<T> recv_buffer;
  std::vector<int> recv_displacements = vector_allgatherv_impl(send_buffer, recv_buffer);

  // Split received data from each processor
  split(recv_buffer, recv_buffers, recv_displacements);
}

#endif // USE_MPI

} // namespace allgatherv::detail

//-----------------------------------------------------------//
//  IMPLEMENTATIONS                                          //
//-----------------------------------------------------------//

template<typename T>
std::vector<int> vector_allgatherv(const std::vector<T> &send_buffer, std::vector<T> &recv_buffer, const unsigned size) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "vector_allgatherv only supports T = double, and unsigned"
  );

#ifdef USE_MPI
  return allgatherv::detail::vector_allgatherv_impl(send_buffer, recv_buffer, size, mpi_type<T>());
#else
  return allgatherv::detail::vector_allgatherv_impl(send_buffer, recv_buffer, size);
#endif // USE_MPI
}
