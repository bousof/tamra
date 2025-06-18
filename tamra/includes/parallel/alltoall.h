/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI Alltoall operations to hanlde STL vectors.
 */

#pragma once

#include <mpi.h>

#include <vector>

template<typename T>
void scalarAlltoall(const std::vector<int> &send_buffer, std::vector<T> &recv_buffer, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, int>::value,
    "scalarAlltoall only supports T = int"
  );

	// Sharing of values between all processors
  MPI_Alltoall(send_buffer.data(), 1, data_type, recv_buffer.data(), 1, data_type, MPI_COMM_WORLD);
}

void intAlltoall(const std::vector<int>& send_buffer, std::vector<int>& recv_buffer, const int size);
