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

#include "../utils/array_utils.h"
#include <Eigen/Sparse>
#include <functional>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>
#include"../parallel/alltoallv.h"

std::vector<double> jacobiIteration(const Eigen::SparseMatrix<double, Eigen::RowMajor> &A_local,
                                    const std::vector<double> &b_local,
                                    const std::vector<double> &x_local = {},
                                    const std::unordered_map<int, double> &x_lacking = {},
                                    const unsigned rank = 0,
                                    const unsigned col_offset = 0);

std::vector<double> sparseJacobi(const Eigen::SparseMatrix<double, Eigen::RowMajor> &A,
                                 const std::vector<double> &b,
                                 const std::vector<double> &x = {},
                                 const unsigned max_iterations = 100);

std::vector<double> parallelSparseJacobi(const Eigen::SparseMatrix<double, Eigen::RowMajor> &A_local,
                                         const std::vector<double> &b_local,
                                         const std::vector<double> &x_local = {},
                                         const unsigned max_iterations = 100,
                                         const unsigned rank = 0, const unsigned size = 1);
