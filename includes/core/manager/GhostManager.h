/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class that handles ghost cells.
 */

#pragma once

#include <memory>
#include <vector>

#include "../../parallel/allgather.h"
#include "../../parallel/alltoallv.h"

template<typename CellTypeT, typename TreeIteratorType> class GhostManager;

#include "GhostManagerTask.h"

template<typename CellTypeT, typename TreeIteratorTypeT>
class GhostManager {
 public:
  using CellType = CellTypeT;
  using ExtrapolationFunctionType = std::function<void(const std::shared_ptr<CellType>&)>;
  using TaskExtrapolationFunctionType = std::function<bool(const std::shared_ptr<CellType>&)>;
  using TreeIteratorType = TreeIteratorTypeT;
  using GhostManagerTaskType = GhostManagerTask<GhostManager<CellTypeT, TreeIteratorType>>;

  //***********************************************************//
  //  VARIABLES                                                //
  //***********************************************************//
 private:
  // Minimum mesh level
  const unsigned min_level;
  // Maximum mesh level
  const unsigned max_level;
  // Process rank
  const unsigned rank;
  // Number of process
  const unsigned size;
  // Function on how to interpolate owned cell values to children
  TaskExtrapolationFunctionType default_owned_extrapolation_function;
  // Function on how to interpolate ghost cell values to children
  TaskExtrapolationFunctionType default_ghost_extrapolation_function;
  // Default strategy on how to handle conflicts for owned cells
  std::vector<OwnedConflictResolutionStrategy> default_owned_strategies;
  // Default strategy on how to handle conflicts for ghost cells
  std::vector<GhostConflictResolutionStrategy> default_ghost_strategies;
  // Resend owned cells after solving conflicts by default
  bool default_resend_owned;

  //***********************************************************//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //***********************************************************//
 public :
  // Constructor
  GhostManager(const unsigned min_level, const unsigned max_level, const unsigned rank, const unsigned size);
  // Destructor
  ~GhostManager();

  //***********************************************************//
	//  MUTATORS                                                 //
	//***********************************************************//
 public:
  // Set the default extrapolation function for owned cells
  void setDefaultOwnedExtrapolationFunction(TaskExtrapolationFunctionType default_extrapolation_function);
  // Set the default extrapolation function for ghost cells
  void setDefaultGhostExtrapolationFunction(TaskExtrapolationFunctionType default_extrapolation_function);
  // Set the default strategy on how to handle conflicts for owned cells
  void setDefaultOwnedConflictResolutionStrategy(const std::vector<OwnedConflictResolutionStrategy> &default_strategies, const bool default_resend = false);
  // Set the default strategy on how to handle conflicts for ghost cells
  void setDefaultGhostConflictResolutionStrategy(const std::vector<GhostConflictResolutionStrategy> &default_strategies);

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Creation of ghost cells and exchange of ghost values
	GhostManagerTaskType buildGhostLayer(std::vector<std::shared_ptr<CellType>> &root_cells, TreeIteratorType &iterator, const std::vector<int> &directions, ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<CellType> &cell) { (void)cell; }) const;
  // Update ghost cells and exchange values for solving conflicts
	void updateGhostLayer(GhostManagerTaskType &task, TreeIteratorType &iterator) const;
  // Exchange ghost cell values
	void exchangeGhostValues(GhostManagerTaskType &task, TreeIteratorType &iterator, ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<CellType> &cell) { (void)cell; }) const;
  // Share the partitions start and end cells
  void sharePartitions(std::vector<std::vector<unsigned>> &begin_ids, std::vector<std::vector<unsigned>> &end_ids, TreeIteratorType &iterator) const;
 private:
  // Loop on owned cells and check if neighbors belong to another process
  void findCellsToSend(const std::vector<std::vector<unsigned>> &begin_ids, const std::vector<std::vector<unsigned>> &end_ids, std::vector<std::vector<std::shared_ptr<CellType>>> &cells_to_send, TreeIteratorType &iterator, const std::vector<int> &directions) const;
  // Set all ghost cells to coarse
  void setGhostToCoarseRecurs(const std::shared_ptr<CellType> &cell) const;
};

#include "GhostManager.tpp"
