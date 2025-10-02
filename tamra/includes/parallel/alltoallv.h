/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI Alltoallv operations to hanlde STL vectors.
 */

#pragma once

#include <mpi.h>

#include <functional>
#include <numeric>
#include <vector>
#include"ParallelData.h"
#include"./alltoall.h"
#include"../utils/array_utils.h"

std::vector<int> vectorUnsignedAlltoallv(const std::vector<std::vector<unsigned>> &send_buffers, std::vector<unsigned> &recv_buffer);

std::vector<int> vectorIntAlltoallv(const std::vector<std::vector<int>> &send_buffers, std::vector<int> &recv_buffer);

std::vector<int> vectorDoubleAlltoallv(const std::vector<std::vector<double>> &send_buffers, std::vector<double> &recv_buffer);

void vectorUnsignedAlltoallv(const std::vector<std::vector<unsigned>> &send_buffers, std::vector<std::vector<unsigned>> &recv_buffer);

void vectorIntAlltoallv(const std::vector<std::vector<int>> &send_buffers, std::vector<std::vector<int>> &recv_buffer);

void vectorDoubleAlltoallv(const std::vector<std::vector<double>> &send_buffers, std::vector<std::vector<double>> &recv_buffer);

void matrixUnsignedAlltoallv(const std::vector<std::vector<std::vector<unsigned>>> &send_buffers, std::vector<std::vector<unsigned>> &recv_buffer, const unsigned colCount);

void matrixUnsignedAlltoallv(const std::vector<std::vector<std::vector<unsigned>>> &send_buffers, std::vector<std::vector<std::vector<unsigned>>> &recv_buffer, const unsigned colCount);

using ParallelDataFactory = std::function<std::unique_ptr<ParallelData>()>;
void vectorDataAlltoallv(const std::vector<std::vector<std::unique_ptr<ParallelData>>> &send_buffers, std::vector<std::unique_ptr<ParallelData>> &recv_buffer, const ParallelDataFactory createData);

template<typename T>
std::vector<int> vectorAlltoallv(const std::vector<std::vector<T>> &send_buffers, std::vector<T> &recv_buffer, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, unsigned>::value || std::is_same<T, int>::value || std::is_same<T, double>::value,
    "vectorAllToAll only supports T = unsigned, int, or double"
  );

  int size = send_buffers.size();

	// Number of elements to send to each process
	std::vector<int> send_counts(send_buffers.size());
	for (int p{0}; p<size; ++p)
		send_counts[p] = send_buffers[p].size();

	// Number of values to receive from the other processes
	std::vector<int> recv_counts(size);
	intAlltoall(send_counts, recv_counts);

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
void vectorAlltoallv(const std::vector<std::vector<T>> &send_buffers, std::vector<std::vector<T>> &recv_buffers, const MPI_Datatype data_type) {
  // Call the merging all to all function
	std::vector<T> recv_buffer;
  std::vector<int> recv_displacements = vectorAlltoallv(send_buffers, recv_buffer, data_type);

  // Split received data from each processor
  split(recv_buffer, recv_buffers, recv_displacements);
}

template<typename T>
std::vector<int> matrixAlltoallv(const std::vector<std::vector<std::vector<T>>> &send_buffers, std::vector<std::vector<T>> &recv_buffer, const MPI_Datatype data_type, const unsigned colCount) {
  int size = send_buffers.size();

  // Transform vector of matrix to vector of vectors
	std::vector<std::vector<T>> vector_send_buffers(size);
  for (int p{0}, i; p<size; ++p) {
    vector_send_buffers[p].reserve(send_buffers[p].size() * colCount);
    for (i=0; i<send_buffers[p].size(); ++i)
      vector_send_buffers[p].insert(vector_send_buffers[p].end(), send_buffers[p][i].begin(), send_buffers[p][i].end());
  }

  // Call the vector all to all function
	std::vector<T> vector_recv_buffer;
  std::vector<int> recv_displacements = vectorAlltoallv(vector_send_buffers, vector_recv_buffer, data_type);

  // Transform back to a matrix (known colCounts)
  recv_buffer.resize(vector_recv_buffer.size() / colCount);
  for (int i{0}; i<recv_buffer.size(); ++i)
    recv_buffer[i].assign(vector_recv_buffer.begin() + i*colCount, vector_recv_buffer.begin() + (i+1)*colCount);

  return recv_displacements;
}

template<typename T>
void matrixAlltoallv(const std::vector<std::vector<std::vector<T>>> &send_buffers, std::vector<std::vector<std::vector<T>>> &recv_buffers, const MPI_Datatype data_type, const unsigned colCount) {
  // Call the vector all to all function
	std::vector<std::vector<T>> recv_buffer;
  std::vector<int> recv_displacements = matrixAlltoallv(send_buffers, recv_buffer, data_type, colCount);

  // Split received data from each processor
  split(recv_buffer, recv_buffers, recv_displacements);
}
