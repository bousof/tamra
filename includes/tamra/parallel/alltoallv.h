/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI Alltoallv operations to hanlde STL vectors.
 */

#pragma once

#ifdef USE_MPI

#include <mpi.h>
#include <tamra/parallel/mpitypes.h>

#endif // USE_MPI

#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

#include <tamra/utils/array_utils.h>
#include <tamra/parallel/alltoall.h>
#include <tamra/parallel/ParallelData.h>


//-----------------------------------------------------------//
//  PROTOTYPES                                               //
//-----------------------------------------------------------//

template<typename T>
std::vector<int> vector_alltoallv(const std::vector<std::vector<T>> &send_buffers, std::vector<T> &recv_buffer, std::vector<int> recv_counts = {});

template<typename T>
void vector_alltoallv(const std::vector<std::vector<T>> &send_buffers, std::vector<std::vector<T>> &recv_buffers, std::vector<int> recv_counts = {});

template<typename T>
void matrix_alltoallv(const std::vector<std::vector<std::vector<T>>> &send_buffers, std::vector<std::vector<T>> &recv_buffer, const unsigned col_count);

using ParallelDataFactory = std::function<std::unique_ptr<ParallelData>()>;
void vector_data_alltoallv(const std::vector<std::vector<std::unique_ptr<ParallelData>>> &send_buffers, std::vector<std::unique_ptr<ParallelData>> &recv_buffer, const ParallelDataFactory create_data);

//-----------------------------------------------------------//
//  LOWER LEVEL METHODS                                      //
//-----------------------------------------------------------//

namespace alltoallv::detail {

#ifdef USE_MPI

template<typename T>
std::vector<int> vector_alltoallv_impl(const std::vector<std::vector<T>> &send_buffers, std::vector<T> &recv_buffer, const std::vector<int> &recv_counts_in, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, unsigned>::value || std::is_same<T, int>::value || std::is_same<T, double>::value,
    "vector_alltoallv_impl only supports T = float, unsigned, int, or double"
  );

  int size = send_buffers.size();

	// Number of elements to send to each process
	std::vector<int> send_counts(send_buffers.size());
	for (int p{0}; p<size; ++p)
		send_counts[p] = send_buffers[p].size();

	// Number of values to receive from the other processes
	std::vector<int> recv_counts;
  if (recv_counts_in.size() == static_cast<std::size_t>(size))
    recv_counts = recv_counts_in;
  else {
    recv_counts.resize(size);
    scalar_alltoall<int>(send_counts, recv_counts);
  }

	// Total number of cells to send and receive
  std::vector<int> send_displacements, recv_displacements;
  int tot_send = cumulative_sum(send_counts, send_displacements, true);
  int tot_receive = cumulative_sum(recv_counts, recv_displacements, true);

	// Creation of the sending buffer
	std::vector<T> send_buffer;
  send_buffer.reserve(tot_send);
	for (const auto &b : send_buffers)
		send_buffer.insert(send_buffer.end(), b.begin(), b.end());

	// Communication of cells IDs between all processors
	recv_buffer.resize(tot_receive);
	MPI_Alltoallv(send_buffer.data(), send_counts.data(), send_displacements.data(), data_type,
				        recv_buffer.data(), recv_counts.data(), recv_displacements.data(), data_type, MPI_COMM_WORLD);

  return recv_displacements;
}

template<typename T>
void vector_alltoallv_impl(const std::vector<std::vector<T>> &send_buffers, std::vector<std::vector<T>> &recv_buffers, const std::vector<int> &recv_counts, const MPI_Datatype data_type) {
  // Call the merging all to all function
	std::vector<T> recv_buffer;
  std::vector<int> recv_displacements = vector_alltoallv_impl(send_buffers, recv_buffer, recv_counts, data_type);

  // Split received data from each processor
  split(recv_buffer, recv_buffers, recv_displacements);
}

template<typename T>
std::vector<int> matrix_alltoallv_impl(const std::vector<std::vector<std::vector<T>>> &send_buffers, std::vector<std::vector<T>> &recv_buffer, const MPI_Datatype data_type, const unsigned col_count) {
  unsigned size = send_buffers.size();

  // Transform vector of matrix to vector of vectors
	std::vector<std::vector<T>> vector_send_buffers(size);
  for (unsigned p{0}; p<size; ++p) {
    vector_send_buffers[p].reserve(send_buffers[p].size() * col_count);
    for (size_t i{0}; i<send_buffers[p].size(); ++i)
      vector_send_buffers[p].insert(vector_send_buffers[p].end(), send_buffers[p][i].begin(), send_buffers[p][i].end());
  }

  // Call the vector all to all function
	std::vector<T> vector_recv_buffer;
  std::vector<int> recv_counts;
  std::vector<int> recv_displacements = vector_alltoallv_impl(vector_send_buffers, vector_recv_buffer, recv_counts, data_type);

  // Transform back to a matrix (known column counts)
  recv_buffer.resize(vector_recv_buffer.size() / col_count);
  for (size_t i{0}; i<recv_buffer.size(); ++i)
    recv_buffer[i].assign(vector_recv_buffer.begin() + i*col_count, vector_recv_buffer.begin() + (i+1)*col_count);

  return recv_displacements;
}

template<typename T>
void matrix_alltoallv_impl(const std::vector<std::vector<std::vector<T>>> &send_buffers, std::vector<std::vector<std::vector<T>>> &recv_buffers, const MPI_Datatype data_type, const unsigned col_count) {
  // Call the vector all to all function
	std::vector<std::vector<T>> recv_buffer;
  std::vector<int> recv_displacements = matrix_alltoallv_impl(send_buffers, recv_buffer, data_type, col_count);

  // Split received data from each processor
  split(recv_buffer, recv_buffers, recv_displacements);
}

#else

template<typename T>
std::vector<int> vector_alltoallv_impl(const std::vector<std::vector<T>> &send_buffers, std::vector<T> &recv_buffer) {
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, unsigned>::value || std::is_same<T, int>::value || std::is_same<T, double>::value,
    "vector_alltoallv_impl only supports T = float, unsigned, int, or double"
  );

  // No MPI, so one proc then only receive from itself
  recv_buffer = send_buffers[0];

  return { 0 };
}

template<typename T>
void vector_alltoallv_impl(const std::vector<std::vector<T>> &send_buffers, std::vector<std::vector<T>> &recv_buffers) {
  // Call the merging all to all function
	std::vector<T> recv_buffer;
  std::vector<int> recv_displacements = vector_alltoallv_impl(send_buffers, recv_buffer);

  // Split received data from each processor
  split(recv_buffer, recv_buffers, recv_displacements);
}

template<typename T>
std::vector<int> matrix_alltoallv_impl(const std::vector<std::vector<std::vector<T>>> &send_buffers, std::vector<std::vector<T>> &recv_buffer, const unsigned col_count) {
  unsigned size = send_buffers.size();

  // Transform vector of matrix to vector of vectors
	std::vector<std::vector<T>> vector_send_buffers(size);
  for (unsigned p{0}; p<size; ++p) {
    vector_send_buffers[p].reserve(send_buffers[p].size() * col_count);
    for (size_t i=0; i<send_buffers[p].size(); ++i)
      vector_send_buffers[p].insert(vector_send_buffers[p].end(), send_buffers[p][i].begin(), send_buffers[p][i].end());
  }

  // Call the vector all to all function
	std::vector<T> vector_recv_buffer;
  std::vector<int> recv_displacements = vector_alltoallv_impl(vector_send_buffers, vector_recv_buffer);

  // Transform back to a matrix (known column counts)
  recv_buffer.resize(vector_recv_buffer.size() / col_count);
  for (size_t i{0}; i<recv_buffer.size(); ++i)
    recv_buffer[i].assign(vector_recv_buffer.begin() + i*col_count, vector_recv_buffer.begin() + (i+1)*col_count);

  return recv_displacements;
}

template<typename T>
void matrix_alltoallv_impl(const std::vector<std::vector<std::vector<T>>> &send_buffers, std::vector<std::vector<std::vector<T>>> &recv_buffers, const unsigned col_count) {
  // Call the vector all to all function
	std::vector<std::vector<T>> recv_buffer;
  std::vector<int> recv_displacements = matrix_alltoallv_impl(send_buffers, recv_buffer, col_count);

  // Split received data from each processor
  split(recv_buffer, recv_buffers, recv_displacements);
}

#endif // USE_MPI

} // namespace alltoallv::detail

//-----------------------------------------------------------//
//  IMPLEMENTATIONS                                          //
//-----------------------------------------------------------//

template<typename T>
std::vector<int> vector_alltoallv(const std::vector<std::vector<T>> &send_buffers, std::vector<T> &recv_buffer, std::vector<int> recv_counts) {
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value || std::is_same<T, int>::value || std::is_same<T, unsigned>::value,
    "vector_alltoallv only supports T = float, double, int, and unsigned"
  );

#ifdef USE_MPI
  return alltoallv::detail::vector_alltoallv_impl(send_buffers, recv_buffer, recv_counts, mpi_type<T>());
#else
  (void)recv_counts;
  return alltoallv::detail::vector_alltoallv_impl(send_buffers, recv_buffer);
#endif // USE_MPI
}

template<typename T>
void vector_alltoallv(const std::vector<std::vector<T>> &send_buffers, std::vector<std::vector<T>> &recv_buffers, std::vector<int> recv_counts) {
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value || std::is_same<T, int>::value || std::is_same<T, unsigned>::value,
    "vector_alltoallv only supports T = float, double, int, and unsigned"
  );

#ifdef USE_MPI
  alltoallv::detail::vector_alltoallv_impl(send_buffers, recv_buffers, recv_counts, mpi_type<T>());
#else
  (void)recv_counts;
  alltoallv::detail::vector_alltoallv_impl(send_buffers, recv_buffers);
#endif // USE_MPI
}

template<typename T>
void matrix_alltoallv(const std::vector<std::vector<std::vector<T>>> &send_buffers, std::vector<std::vector<T>> &recv_buffer, const unsigned col_count) {
  static_assert(
    std::is_same<T, unsigned>::value,
    "vector_alltoallv only supports T = unsigned"
  );

#ifdef USE_MPI
  alltoallv::detail::matrix_alltoallv_impl(send_buffers, recv_buffer, mpi_type<T>(), col_count);
#else
  alltoallv::detail::matrix_alltoallv_impl(send_buffers, recv_buffer, col_count);
#endif // USE_MPI
}
