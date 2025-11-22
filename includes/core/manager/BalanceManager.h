/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class that handles tree load balancing.
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "../../parallel/alltoallv.h"
#include "../../parallel/bcast.h"
#include "../../parallel/gather.h"
#include "../../utils/array_utils.h"

template<typename CellType, typename TreeIteratorType>
class BalanceManager {
  using ExtrapolationFunctionType = std::function<void(const std::shared_ptr<CellType>&)>;

  //***********************************************************//
  //  VARIABLES                                                //
  //***********************************************************//
  // Minimum mesh level
  const unsigned min_level;
  // Maximum mesh level
  const unsigned max_level;
  // Process rank
  const unsigned rank;
  // Number of process
  const unsigned size;

  //***********************************************************//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //***********************************************************//
 public :
  // Constructor
  BalanceManager(const unsigned min_level, const unsigned max_level, const unsigned rank, const unsigned size);
  // Destructor
  ~BalanceManager();

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Determine if load balancing is needed
  std::pair<bool, std::vector<double>> isLoadBalancingNeeded(const std::vector<std::shared_ptr<CellType>> &root_cells, const double max_pct_unbalance) const;
  // Performs load balancing between processes
	void loadBalance(const std::vector<std::shared_ptr<CellType>> &root_cells, TreeIteratorType &iterator, const double max_pct_unbalance = 0., ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<CellType> &cell) { (void)cell; }) const;
 private:
  // Determine the local load for this process
  double computeLoad(const std::shared_ptr<CellType> &cell) const;
  // Determine the cells to send to each process
  std::vector<std::vector<std::shared_ptr<CellType>>> cellsToExchange(const std::vector<double> &cumulative_loads, const std::vector<double> &target_cumulative_loads, TreeIteratorType &iterator) const;
  // Exchange cells structure and data
  void exchangeAndCreateCells(const std::vector<std::vector<std::shared_ptr<CellType>>> &cells_to_send, TreeIteratorType &iterator, ExtrapolationFunctionType extrapolation_function) const;
  // Compress the structure of cells (1 cell ID + other cell levels)
  void compressCellStructure(const std::vector<unsigned> &cell_id, const std::vector<unsigned> &cell_levels, std::vector<unsigned> &cell_structure) const;
  // Uncompress the structure of cells (1 cell ID + other cell levels)
  void uncompressCellStructure(const std::vector<unsigned> &cell_structure, std::vector<unsigned> &first_cell_id, std::vector<unsigned> &cell_levels, const unsigned cell_id_size) const;
  // Set a parent to belong to this proc if any of its child do else set to other proc
  bool backPropagateFlags(const std::shared_ptr<CellType> &cell) const;
};

#include "./BalanceManager.tpp"
