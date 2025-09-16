/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Jacobi method implementation for parallel sparse matrix solving using MPI.
 */

#pragma once

#include <Eigen/Sparse>
#include <functional>
#include <iostream>
#include <set>
#include <vector>
#include <unordered_map>
#include "../utils/array_utils.h"

std::vector<double> jacobiIteration(const Eigen::SparseMatrix<double, Eigen::RowMajor> &A_local,
                                    const std::vector<double> &b_local,
                                    const std::vector<double> &x_local = {},
                                    const std::unordered_map<int, double> &x_lacking = {},
                                    const int rank = 0,
                                    const int col_offset = 0);

std::vector<double> sparseJacobi(const Eigen::SparseMatrix<double, Eigen::RowMajor> &A,
                                 const std::vector<double> &b,
                                 const std::vector<double> &x = {},
                                 const int max_iterations = 100);

//#define USE_MPI
#ifdef USE_MPI
#include <mpi.h>
#include "../parallel/alltoallv.h"

std::vector<double> parallelSparseJacobi(const Eigen::SparseMatrix<double, Eigen::RowMajor> &A_local,
                                         const std::vector<double> &b_local,
                                         const std::vector<double> &x_local = {},
                                         const int max_iterations = 100,
                                         const int rank = 0, const int size = 1);

#endif
