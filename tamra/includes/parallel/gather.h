/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI Allgather operations to hanlde STL vectors.
 */

#pragma once

#include <mpi.h>

#include <functional>
#include <iostream>
#include <numeric>
#include <vector>
#include"ParallelData.h"

template<typename T>
void gather(const T value, std::vector<T> &buffer, const int root, const int rank, const int size, const MPI_Datatype data_type) {
  static_assert(
    std::is_same<T, double>::value,
    "gather only supports T = double"
  );

	// Communication of cells IDs between all processors
  if (rank == root)
	  buffer.resize(size);
	MPI_Gather(&value, 1, data_type, buffer.data(), 1, data_type, root, MPI_COMM_WORLD);
}

void doubleGather(double &value, std::vector<double> &buffer, const int root, const int rank, const int size);
