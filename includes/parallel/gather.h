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
#endif // USE_MPI

#include <functional>
#include <iostream>
#include <numeric>
#include <vector>
#include "ParallelData.h"

#ifdef USE_MPI

template<typename T>
void gather(const T value, std::vector<T> &buffer, const unsigned root, const unsigned rank, const unsigned size, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, double>::value,
    "gather only supports T = double"
  );

	// Communication of cells IDs between all processors
  if (rank == root)
	  buffer.resize(size);
	MPI_Gather(&value, 1, data_type, buffer.data(), 1, data_type, root, MPI_COMM_WORLD);
}

#else

template<typename T>
void gather(const T value, std::vector<T> &buffer, const unsigned, const unsigned, const unsigned) {
  static_assert(
    std::is_same<T, double>::value,
    "gather only supports T = double"
  );

	buffer = { value };
}

#endif // USE_MPI

void doubleGather(double &value, std::vector<double> &buffer, const unsigned root, const unsigned rank, const unsigned size);
