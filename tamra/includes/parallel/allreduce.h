/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI Allreduce operations (general case).
 */

#pragma once

#include <mpi.h>

#include <numeric>

template<typename T>
void scalarAllReduce(const T value, T &reduction, const MPI_Datatype data_type, const MPI_Op op_type) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, unsigned>::value,
    "scalarAllReduce only supports T = bool, or unsigned"
  );

	// Reduction of values between all processors
  MPI_Allreduce(&value, &reduction, 1, data_type, op_type, MPI_COMM_WORLD);
}

void boolAndAllReduce(const bool value, bool& reduction);

void unsignedSumAllReduce(const unsigned value, unsigned& reduction);
