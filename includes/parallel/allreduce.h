/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Simplifies MPI Allreduce operations (general case).
 */

#pragma once

#ifdef USE_MPI
  #include <mpi.h>
#endif // USE_MPI

#include <numeric>

#ifdef USE_MPI

template<typename T>
void scalarAllReduce(const T value, T &reduction, const MPI_Datatype data_type, const MPI_Op op_type) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "scalarAllReduce only supports T = bool, double, or unsigned"
  );

	// Reduction of values between all processors
  MPI_Allreduce(&value, &reduction, 1, data_type, op_type, MPI_COMM_WORLD);
}

#else

template<typename T>
void scalarAllReduce(const T value, T &reduction) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "scalarAllReduce only supports T = bool, double, or unsigned"
  );

	// No MPI, so one proc then reduction is value
  reduction = value;
}

#endif // USE_MPI

void boolAndAllReduce(const bool value, bool &reduction);

void unsignedSumAllReduce(const unsigned value, unsigned &reduction);

void doubleMinAllReduce(const double value, double &reduction);
