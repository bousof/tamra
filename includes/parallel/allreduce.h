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
#include "mpitypes.h"

#endif // USE_MPI

#include <numeric>

//-----------------------------------------------------------//
//  PROTOTYPES                                               //
//-----------------------------------------------------------//

void boolAndAllreduce(const bool value, bool &reduction);

template<typename T>
void scalarSumAllreduce(const T value, T &reduction);

template<typename T>
void scalarMinAllreduce(const T value, T &reduction);

//-----------------------------------------------------------//
//  LOWER LEVEL METHODS                                      //
//-----------------------------------------------------------//

namespace allreduce::detail {

#ifdef USE_MPI

template<typename T>
void scalarAllreduceT(const T value, T &reduction, const MPI_Datatype data_type, const MPI_Op op_type) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "scalarAllreduceT only supports T = bool, double, or unsigned"
  );

	// Reduction of values between all processors
  MPI_Allreduce(&value, &reduction, 1, data_type, op_type, MPI_COMM_WORLD);
}

#else

template<typename T>
void scalarAllreduceT(const T value, T &reduction) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, double>::value || std::is_same<T, unsigned>::value,
    "scalarAllreduceT only supports T = bool, double, or unsigned"
  );

	// No MPI, so one proc then reduction is value
  reduction = value;
}

#endif // USE_MPI

} // namespace allreduce::detail

//-----------------------------------------------------------//
//  IMPLEMENTATIONS                                          //
//-----------------------------------------------------------//

template<typename T>
void scalarSumAllreduce(const T value, T &reduction) {
  static_assert(
    std::is_same<T, unsigned>::value,
    "scalarSumAllreduce only supports T = unsigned"
  );

#ifdef USE_MPI
  allreduce::detail::scalarAllreduceT(value, reduction, mpi_type<T>(), MPI_SUM);
#else
  allreduce::detail::scalarAllreduceT(value, reduction);
#endif // USE_MPI
}

template<typename T>
void scalarMinAllreduce(const T value, T &reduction) {
  static_assert(
    std::is_same<T, double>::value,
    "scalarMinAllreduce only supports T = double"
  );

#ifdef USE_MPI
  allreduce::detail::scalarAllreduceT(value, reduction, mpi_type<T>(), MPI_MIN);
#else
  allreduce::detail::scalarAllreduceT(value, reduction);
#endif // USE_MPI
}
