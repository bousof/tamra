/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI Alltoall operations to hanlde STL vectors.
 */

#pragma once

#ifdef USE_MPI

#include <mpi.h>
#include "mpitypes.h"

#endif // USE_MPI

#include <vector>

//-----------------------------------------------------------//
//  PROTOTYPES                                               //
//-----------------------------------------------------------//

template<typename T>
void scalarAlltoall(const std::vector<T> &send_buffer, std::vector<T> &recv_buffer);

//-----------------------------------------------------------//
//  LOWER LEVEL METHODS                                      //
//-----------------------------------------------------------//

namespace alltoall::detail {

#ifdef USE_MPI

template<typename T>
void scalarAlltoallT(const std::vector<int> &send_buffer, std::vector<T> &recv_buffer, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, int>::value,
    "scalarAlltoallT only supports T = int"
  );

	// Sharing of values between all processors
  MPI_Alltoall(send_buffer.data(), 1, data_type, recv_buffer.data(), 1, data_type, MPI_COMM_WORLD);
}

#else

template<typename T>
void scalarAlltoallT(const std::vector<int> &send_buffer, std::vector<T> &recv_buffer) {
  static_assert(
    std::is_same<T, int>::value,
    "scalarAlltoallT only supports T = int"
  );

	// No MPI, so one proc then only receive from itself
  recv_buffer = send_buffer;
}

#endif // USE_MPI

} // namespace alltoall::detail

//-----------------------------------------------------------//
//  IMPLEMENTATIONS                                          //
//-----------------------------------------------------------//

template<typename T>
void scalarAlltoall(const std::vector<T> &send_buffer, std::vector<T> &recv_buffer) {
  static_assert(
    std::is_same<T, int>::value,
    "scalarAlltoall only supports T = int"
  );

#ifdef USE_MPI
  alltoall::detail::scalarAlltoallT(send_buffer, recv_buffer, mpi_type<T>());
#else
  alltoall::detail::scalarAlltoallT(send_buffer, recv_buffer);
#endif // USE_MPI
}
