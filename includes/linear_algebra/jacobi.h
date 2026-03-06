/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Jacobi method implementation for parallel sparse matrix solving using MPI.
 */

#pragma once

#ifdef USE_MPI
  #include <mpi.h>
#endif // USE_MPI

#include <Eigen/Sparse>
#include <functional>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>

#include "../parallel/alltoallv.h"
#include "../utils/array_utils.h"

template <typename RealT = double, typename AccT = double>
std::vector<RealT> jacobiIteration(const Eigen::SparseMatrix<RealT, Eigen::RowMajor> &A_local,
                                    const std::vector<RealT> &b_local,
                                    const std::vector<RealT> &x_local = {},
                                    const std::unordered_map<int, RealT> &x_lacking = {},
                                    const unsigned rank = 0,
                                    const unsigned col_offset = 0);

template <typename RealT = double, typename AccT = double>
std::vector<RealT> sparseJacobi(const Eigen::SparseMatrix<RealT, Eigen::RowMajor> &A,
                                 const std::vector<RealT> &b,
                                 const std::vector<RealT> &x = {},
                                 const unsigned max_iterations = 100);

template <typename RealT = double, typename AccT = double>
std::vector<RealT> parallelSparseJacobi(const Eigen::SparseMatrix<RealT, Eigen::RowMajor> &A_local,
                                         const std::vector<RealT> &b_local,
                                         const std::vector<RealT> &x_local = {},
                                         const unsigned max_iterations = 100,
                                         const unsigned rank = 0, const unsigned size = 1);

#include "jacobi.tpp"
