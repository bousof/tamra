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
#include <tamra/parallel/mpitypes.h>

#endif // USE_MPI

#include <algorithm>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <type_traits>


//-----------------------------------------------------------//
//  PROTOTYPES                                               //
//-----------------------------------------------------------//

void bool_and_allreduce(const bool value, bool &reduction);

template<typename T>
void scalar_sum_allreduce(const T value, T &reduction);

template<typename T>
void scalar_min_allreduce(const T value, T &reduction);

template<typename T>
void scalar_max_allreduce(const T value, T &reduction);

// Sum-reduce a contiguous array on all ranks. In a sequential build, copy the
// input into the output, matching the scalar allreduce behavior.
template<typename T>
void array_sum_allreduce(const T *values, T *reduction, const std::size_t count) {
  static_assert(
    std::is_same<T, unsigned>::value || std::is_same<T, float>::value || std::is_same<T, double>::value,
    "array_sum_allreduce only supports T = unsigned, float, or double"
  );

  if (count == 0)
    return;

#ifdef USE_MPI
  if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    throw std::length_error("array_sum_allreduce: element count exceeds the MPI int count limit");

  MPI_Allreduce(values, reduction, static_cast<int>(count), mpi_type<T>(),
                MPI_SUM, MPI_COMM_WORLD);
#else
  std::copy_n(values, count, reduction);
#endif // USE_MPI
}

//-----------------------------------------------------------//
//  LOWER LEVEL METHODS                                      //
//-----------------------------------------------------------//

namespace allreduce::detail {

#ifdef USE_MPI

template<typename T>
void scalar_allreduce_impl(const T value, T &reduction, const MPI_Datatype data_type, const MPI_Op op_type) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, double>::value || std::is_same<T, float>::value || std::is_same<T, unsigned>::value,
    "scalar_allreduce_impl only supports T = bool, double, float, or unsigned"
  );

	// Reduction of values between all processors
  MPI_Allreduce(&value, &reduction, 1, data_type, op_type, MPI_COMM_WORLD);
}

#else

template<typename T>
void scalar_allreduce_impl(const T value, T &reduction) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, double>::value || std::is_same<T, float>::value || std::is_same<T, unsigned>::value,
    "scalar_allreduce_impl only supports T = bool, double, float, or unsigned"
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
void scalar_sum_allreduce(const T value, T &reduction) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, float>::value || std::is_same<T, unsigned>::value,
    "scalar_sum_allreduce only supports T = double, float, or unsigned"
  );

#ifdef USE_MPI
  allreduce::detail::scalar_allreduce_impl(value, reduction, mpi_type<T>(), MPI_SUM);
#else
  allreduce::detail::scalar_allreduce_impl(value, reduction);
#endif // USE_MPI
}

template<typename T>
void scalar_min_allreduce(const T value, T &reduction) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, float>::value,
    "scalar_min_allreduce only supports T = double or float"
  );

#ifdef USE_MPI
  allreduce::detail::scalar_allreduce_impl(value, reduction, mpi_type<T>(), MPI_MIN);
#else
  allreduce::detail::scalar_allreduce_impl(value, reduction);
#endif // USE_MPI
}

template<typename T>
void scalar_max_allreduce(const T value, T &reduction) {
  static_assert(
    std::is_same<T, double>::value || std::is_same<T, float>::value,
    "scalar_max_allreduce only supports T = double or float"
  );

#ifdef USE_MPI
  allreduce::detail::scalar_allreduce_impl(value, reduction, mpi_type<T>(), MPI_MAX);
#else
  allreduce::detail::scalar_allreduce_impl(value, reduction);
#endif // USE_MPI
}


