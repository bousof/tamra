/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI gather operations to hanlde STL vectors.
 */

#pragma once

#ifdef USE_MPI

#include <mpi.h>
#include <tamra/parallel/mpitypes.h>

#endif // USE_MPI

#include <functional>
#include <iostream>
#include <numeric>
#include <vector>

#include <tamra/parallel/ParallelData.h>


//-----------------------------------------------------------//
//  PROTOTYPES                                               //
//-----------------------------------------------------------//

template<typename T>
void scalar_gather(T &value, std::vector<T> &buffer, const unsigned root, const unsigned rank, const unsigned size);

//-----------------------------------------------------------//
//  LOWER LEVEL METHODS                                      //
//-----------------------------------------------------------//

namespace gather::detail {

#ifdef USE_MPI

template<typename T>
void scalar_gather_impl(const T value, std::vector<T> &buffer, const unsigned root, const unsigned rank, const unsigned size, const MPI_Datatype data_type) {
	// Communication of cells IDs between all processors
  if (rank == root)
	  buffer.resize(size);
	MPI_Gather(&value, 1, data_type, buffer.data(), 1, data_type, root, MPI_COMM_WORLD);
}

#else

template<typename T>
void scalar_gather_impl(const T value, std::vector<T> &buffer, const unsigned, const unsigned, const unsigned) {
  buffer = { value };
}

#endif // USE_MPI

} // namespace gather::detail

//-----------------------------------------------------------//
//  IMPLEMENTATIONS                                          //
//-----------------------------------------------------------//

template<typename T>
void scalar_gather(T &value, std::vector<T> &buffer, const unsigned root, const unsigned rank, const unsigned size) {
  static_assert(
    std::is_same<T, double>::value,
    "scalar_gather only supports T = double"
  );

#ifdef USE_MPI
  gather::detail::scalar_gather_impl(value, buffer, root, rank, size, mpi_type<T>());
#else
  gather::detail::scalar_gather_impl(value, buffer, root, rank, size);
#endif // USE_MPI
}
