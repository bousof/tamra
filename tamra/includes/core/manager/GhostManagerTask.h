/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class to handle of incomplete ghost manager tasks.
 */

#pragma once

#include <parallel/allreduce.h>
#include <parallel/wrapper.h>
#include "../../utils/array_utils.h"
#include "./GhostManager.h"

// Enumeration for strategies on how to handle conflicts for owned cells.
// Possible values are:
// - `EXTRAPOLATE`: Extrapolates values for newly split owned cells
// - `IGNORE`:      Just ignore the conflict (has been/will be handled out of the task)
// - `THROW`:       Throw an error message and stop the program
enum class OwnedConflictResolutionStrategy {
  EXTRAPOLATE,
  IGNORE,
  THROW,
};

// Enumeration for strategies on how to handle conflicts for ghost cells.
// Possible values are:
// - `EXTRAPOLATE`:    Extrapolates values for newly split ghost cells
// - `IGNORE`:         Just ignore the conflict (has been/will be handled out of the task)
// - `SPLIT_IN_OWNER`: Sender must split to match ghost cell
// - `THROW`:          Throw an error message and stop the program
// - `TRY_COARSEN`:    Receiver try to coarsen the cell if the flags and structure allow it
enum class GhostConflictResolutionStrategy {
  EXTRAPOLATE,    // Extrapolates values for newly split ghost cells
  IGNORE,         // Just ignore the conflict (has been/will be handled out of the task)
  SPLIT_IN_OWNER, // Sender must split to match ghost cell
  THROW,          // Throw an error message and stop the program
  TRY_COARSEN     // Receiver try to coarsen the cell if the flags and structure allow it
};

template<typename GhostManagerType>
class GhostManagerTask {
  using CellType = typename GhostManagerType::CellType;
  using ExtrapolationFunctionType = typename GhostManagerType::TaskExtrapolationFunctionType;
  using TreeIteratorType = typename GhostManagerType::TreeIteratorType;

  //***********************************************************//
  //  VARIABLES                                                //
  //***********************************************************//
  // Associated ghost manager
  const GhostManagerType &ghost_manager;
 public:
  // Flags for checking task status
  bool is_finished;
 private:
  // Cells at partition interfaces to send to other process
  std::vector<std::vector<std::shared_ptr<CellType>>> cells_to_send;
  // Cells at partition interfaces to recv from other process
  std::vector<std::shared_ptr<CellType>> cells_to_recv;
  // Owned cells that were split because of ghost layer creation
  std::vector<std::shared_ptr<CellType>> extrapolate_owned_cells;
  // Ghost cells that were found split in this process
  std::vector<std::shared_ptr<CellType>> extrapolate_ghost_cells;
  // IDs of the first cells on each of the process
  std::vector<std::vector<unsigned>> partition_begin_ids;
  // IDs of the last cells on each of the process
  std::vector<std::vector<unsigned>> partition_end_ids;
  // Function on how to interpolate owned cell values to children
  ExtrapolationFunctionType owned_extrapolation_function;
  // Function on how to interpolate ghost cell values to children
  ExtrapolationFunctionType ghost_extrapolation_function;
  // Strategy on how to handle conflicts for owned cells
  std::vector<OwnedConflictResolutionStrategy> owned_strategies;
  // Strategy on how to handle conflicts for ghost cells
  std::vector<GhostConflictResolutionStrategy> ghost_strategies;
  // Resend owned cells after solving conflicts
  bool resend_owned;

  //***********************************************************//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //***********************************************************//
 public :
  // Constructor
  GhostManagerTask(const GhostManagerType &ghost_manager, const bool is_finished);
  GhostManagerTask(const GhostManagerType &ghost_manager, const bool is_finished, std::vector<std::vector<std::shared_ptr<CellType>>> &&cells_to_send, std::vector<std::shared_ptr<CellType>> &&cells_to_recv, std::vector<std::shared_ptr<CellType>> &&extrapolate_owned_cells, std::vector<std::shared_ptr<CellType>> &&extrapolate_ghost_cells, std::vector< std::vector<unsigned> > &&partition_begin_ids, std::vector< std::vector<unsigned> > &&partition_end_ids);
  // Destructor
  ~GhostManagerTask();

  //***********************************************************//
	//  ACCESSORS                                                //
	//***********************************************************//
 public:
  const std::vector<std::vector<std::shared_ptr<CellType>>>& getCellsToSend() const;
  const std::vector<std::shared_ptr<CellType>>& getCellsToRecv() const;

  //***********************************************************//
	//  MUTATORS                                                 //
	//***********************************************************//
 public:
  // Set the extrapolation function for owned cells
  void setOwnedExtrapolationFunction(ExtrapolationFunctionType extrapolation_function);
  // Set the extrapolation function for ghost cells
  void setGhostExtrapolationFunction(ExtrapolationFunctionType extrapolation_function);
  // Set the strategies on how to handle conflicts on owned cells.
  // First parameter `strategies` is the strategies on how to handle conflicts by priority.
  // The second parameter `resend` is if process should resend the cells to the other process.
  // Default is `strategies={IGNORE}`and `resend=false`.
  void setOwnedConflictResolutionStrategy(const std::vector<OwnedConflictResolutionStrategy> strategies, const bool resend = false);
  // Set the strategies on how to handle conflicts on ghost cells.
  // First parameter `strategies` is the strategies on how to handle conflicts by priority.
  // Default is `strategies={IGNORE}`.
  void setGhostConflictResolutionStrategy(const std::vector<GhostConflictResolutionStrategy> strategies);

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Continue the task following the specified resolution strategy
  void continueTask(TreeIteratorType &iterator);
  // Terminate the task by saying to all processes that nothing to do on this process
  bool terminateTask();
  // Cancel the task from the ghost manager
  void cancelTask();
 private:
  std::vector<bool> continueTaskOwned(TreeIteratorType &iterator);
  std::vector<bool> continueTaskGhost(TreeIteratorType &iterator);
  void continueExtrapolateTaskOwned(std::vector<bool> &resolution_flags);
  void continueExtrapolateTaskGhost(std::vector<bool> &resolution_flags);
  void continueIgnoreTask(std::vector<bool> &resolution_flags);
  void continueSplitInOwnerTaskGhost(std::vector<bool> &resolution_flags);
  void continueTryCoarseTaskGhost(std::vector<bool> &resolution_flags);
  bool applyExtrapolationFunctionRecurs(const std::shared_ptr<CellType> &cell, const ExtrapolationFunctionType &extrapolation_function);
};

#include "./GhostManagerTask.tpp"
