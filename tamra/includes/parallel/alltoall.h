/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI Alltoall operations to hanlde STL vectors.
 */

#pragma once

#include <mpi.h>

#include <functional>
#include <iostream>
#include <numeric>
#include <vector>
#include"parallel_data.h"
#include"../utils/array_utils.h"

void intAllToAll(const std::vector<int>& send_buffer, std::vector<int>& recv_buffer, const int size);

template<typename T>
std::vector<int> vectorAllToAll(const std::vector< std::vector<T> > &send_buffers, std::vector<T> &recv_buffer, const int size, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, unsigned>::value || std::is_same<T, int>::value || std::is_same<T, double>::value,
    "vectorAllToAll only supports T = unsigned, int, or double"
  );

	// Number of elements to send to each process
	std::vector<int> send_counts(size);
	for (int p=0; p<size; ++p) {
		send_counts[p] = send_buffers[p].size();
	}

	// Number of cells to receive for the other process
	std::vector<int> recv_counts(size);
	intAllToAll(send_counts, recv_counts, size);

	// Total number of cells to send and receive
  std::vector<int> send_displacements, recv_displacements;
  int tot_send = cumulative_sum(send_counts, send_displacements, true);
  int tot_receive = cumulative_sum(recv_counts, recv_displacements, true);

	// Creation of the sending buffer
	std::vector<T> send_buffer;
  send_buffer.reserve(tot_send);
	for (const auto &b: send_buffers) {
		send_buffer.insert(send_buffer.end(), b.begin(), b.end());
	}

	// Communication of cells IDs between all processors
	recv_buffer.resize(tot_receive);
	MPI_Alltoallv(send_buffer.data(), send_counts.data(), send_displacements.data(), data_type,
				        recv_buffer.data(), recv_counts.data(), recv_displacements.data(), data_type, MPI_COMM_WORLD);

  return recv_displacements;
}

template<typename T>
void vectorAllToAll(const std::vector< std::vector<T> > &send_buffers, std::vector< std::vector<T> > &recv_buffers, const int size, const MPI_Datatype data_type) {
  // Call the merging all to all function
	std::vector<T> recv_buffer;
  std::vector<int> recv_displacements = vectorAllToAll(send_buffers, recv_buffer, size, data_type);

  // Split received data from each processor
  recv_buffers.resize(size);
  for (int p{0}; p<size; ++p) {
    int end_index = p<(size-1) ? recv_displacements[p+1] : recv_buffer.size();
    recv_buffers[p].assign(recv_buffer.begin()+recv_displacements[p], recv_buffer.begin()+end_index);
  }
}

std::vector<int> vectorUnsignedAllToAll(const std::vector< std::vector<unsigned> > &send_buffers, std::vector<unsigned> &recv_buffer, const int size);

std::vector<int> vectorIntAllToAll(const std::vector< std::vector<int> > &send_buffers, std::vector<int> &recv_buffer, const int size);

std::vector<int> vectorDoubleAllToAll(const std::vector< std::vector<double> > &send_buffers, std::vector<double> &recv_buffer, const int size);

void vectorUnsignedAllToAll(const std::vector< std::vector<unsigned> > &send_buffers, std::vector< std::vector<unsigned> > &recv_buffer, const int size);

void vectorIntAllToAll(const std::vector< std::vector<int> > &send_buffers, std::vector< std::vector<int> > &recv_buffer, const int size);

void vectorDoubleAllToAll(const std::vector< std::vector<double> > &send_buffers, std::vector< std::vector<double> > &recv_buffer, const int size);

using ParallelDataFactory = std::function<std::unique_ptr<ParallelData>()>;
void vectorDataAllToAll(const std::vector< std::vector< std::unique_ptr<ParallelData> > > &send_buffers, std::vector< std::unique_ptr<ParallelData> > &recv_buffer, const int size, const ParallelDataFactory createData);
