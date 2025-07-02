#include "./GhostManagerTask.h"
//#include "../../../display/display_vector.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename GhostManagerType>
GhostManagerTask<GhostManagerType>::GhostManagerTask(const GhostManagerType &ghost_manager, const bool is_finished)
: ghost_manager(ghost_manager),
  is_finished(is_finished) {}

template<typename GhostManagerType>
GhostManagerTask<GhostManagerType>::GhostManagerTask(const GhostManagerType &ghost_manager, const bool is_finished, std::vector<std::vector<std::shared_ptr<CellType>>> &&cells_to_send, std::vector<std::shared_ptr<CellType>> &&extrapolate_owned_cells, std::vector<std::shared_ptr<CellType>> &&extrapolate_ghost_cells, std::vector< std::vector<unsigned> > &&partition_begin_ids, std::vector< std::vector<unsigned> > &&partition_end_ids)
: ghost_manager(ghost_manager),
  is_finished(is_finished),
  cells_to_send(cells_to_send),
  extrapolate_owned_cells(extrapolate_owned_cells),
  extrapolate_ghost_cells(extrapolate_ghost_cells),
  partition_begin_ids(partition_begin_ids),
  partition_end_ids(partition_end_ids) {
  owned_extrapolation_function = [](const std::shared_ptr<CellType>& cell) { return false; };
  ghost_extrapolation_function = [](const std::shared_ptr<CellType>& cell) { return false; };
}

// Destructor
template<typename GhostManagerType>
GhostManagerTask<GhostManagerType>::~GhostManagerTask() {};


//***********************************************************//
//  MUTATORS                                                 //
//***********************************************************//

// Set the extrapolation function for owned cells
template<typename GhostManagerType>
void GhostManagerTask<GhostManagerType>::setOwnedExtrapolationFunction(ExtrapolationFunction extrapolation_function) {
  owned_extrapolation_function = extrapolation_function;
}

// Set the extrapolation function for ghost cells
template<typename GhostManagerType>
void GhostManagerTask<GhostManagerType>::setGhostExtrapolationFunction(ExtrapolationFunction extrapolation_function) {
  ghost_extrapolation_function = extrapolation_function;
}

// Set the strategies on how to handle conflicts on owned cells.
template<typename GhostManagerType>
void GhostManagerTask<GhostManagerType>::setOwnedConflictResolutionStrategy(const std::vector<OwnedConflictResolutionStrategy> &strategies, const bool resend) {
  owned_strategies = strategies;
  resend_owned = resend;
}

// Set the strategies on how to handle conflicts on ghost cells.
template<typename GhostManagerType>
void GhostManagerTask<GhostManagerType>::setGhostConflictResolutionStrategy(const std::vector<GhostConflictResolutionStrategy> &strategies) {
  ghost_strategies = strategies;
}


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Continue the task following the specified resolution strategy
template<typename GhostManagerType>
void GhostManagerTask<GhostManagerType>::continueTask(TreeIteratorType &iterator) {
  std::vector<bool> owned_resolution_flags = continueTaskOwned(iterator);
  std::vector<bool> ghost_resolution_flags = continueTaskGhost(iterator);

  is_finished = all(owned_resolution_flags) && all(ghost_resolution_flags);

  // Build and resend data if needed
  if (resend_owned) {
    // SEND ERROR and terminate MPI
    throw std::runtime_error("Error resend_owned=true not implemented in GhostManagerTask::continueTask()");
    mpi_finalize();

    // Call updateGhostLayer to handle the resend logic
    ghost_manager.updateGhostLayer(*this, iterator);
  }
}

// Terminate the task by saying to all processes that nothing to do on this process
template<typename GhostManagerType>
bool GhostManagerTask<GhostManagerType>::terminateTask() {
  // TODO: Implement this

  return true;
}

// Cancel the task from the ghost manager
template<typename GhostManagerType>
void GhostManagerTask<GhostManagerType>::cancelTask() {
  // TODO: Implement this
}

template<typename GhostManagerType>
std::vector<bool> GhostManagerTask<GhostManagerType>::continueTaskOwned(TreeIteratorType &iterator) {
  std::vector<bool> resolution_flags(extrapolate_owned_cells.size(), false);
  for (const auto strategy: owned_strategies)
    if (!all(resolution_flags))
      switch (strategy) {
        case OwnedConflictResolutionStrategy::EXTRAPOLATE:
          continueExtrapolateTaskOwned(resolution_flags);
          break;
        case OwnedConflictResolutionStrategy::IGNORE:
          continueIgnoreTask(resolution_flags);
          break;
        case OwnedConflictResolutionStrategy::THROW:
          bool throw_error = !all(resolution_flags);
          boolAndAllReduce(throw_error, throw_error);
          if (throw_error)
            throw std::runtime_error("Error thrown in GhostManagerTask::continueTaskOwned()");
          break;
      }
  return resolution_flags;
}

template<typename GhostManagerType>
std::vector<bool> GhostManagerTask<GhostManagerType>::continueTaskGhost(TreeIteratorType &iterator) {
  // SEND ERROR and terminate MPI if SPLIT_IN_OWNER or TRY_COARSEN is used
  if (std::any_of(ghost_strategies.begin(), ghost_strategies.end(), [](const GhostConflictResolutionStrategy &strategy) {
    return strategy == GhostConflictResolutionStrategy::SPLIT_IN_OWNER || strategy == GhostConflictResolutionStrategy::TRY_COARSEN;
  })) {
    throw std::runtime_error("Strategy SPLIT_IN_OWNER and TRY_COARSEN not implemented in GhostManagerTask::continueTaskGhost()");
    mpi_finalize();
  }

  std::vector<bool> resolution_flags(extrapolate_ghost_cells.size(), false);
  for (const auto strategy: ghost_strategies)
    if (!all(resolution_flags))
      switch (strategy) {
        case GhostConflictResolutionStrategy::EXTRAPOLATE:
          continueExtrapolateTaskGhost(resolution_flags);
          break;
        case GhostConflictResolutionStrategy::IGNORE:
          continueIgnoreTask(resolution_flags);
          break;
        case GhostConflictResolutionStrategy::SPLIT_IN_OWNER:
          continueSplitInOwnerTaskGhost(resolution_flags);
          break;
        case GhostConflictResolutionStrategy::TRY_COARSEN:
          continueTryCoarseTaskGhost(resolution_flags);
          break;
        case GhostConflictResolutionStrategy::THROW:
          bool throw_error = !all(resolution_flags);
          boolAndAllReduce(throw_error, throw_error);
          if (throw_error)
            throw std::runtime_error("Error thrown in GhostManagerTask::continueTaskGhost()");
          break;
      }
  return resolution_flags;
}

template<typename GhostManagerType>
bool GhostManagerTask<GhostManagerType>::applyExtrapolationFunctionRecurs(const std::shared_ptr<CellType> &cell, const ExtrapolationFunction &extrapolation_function) {
  bool success = extrapolation_function(cell);
  for (auto &child: cell->getChildCells())
    if (!child->isLeaf())
      success &= applyExtrapolationFunctionRecurs(child, extrapolation_function);
  return success;
}

template<typename GhostManagerType>
void GhostManagerTask<GhostManagerType>::continueExtrapolateTaskOwned(std::vector<bool> &resolution_flags) {
  for (int i{0}; i<extrapolate_owned_cells.size(); ++i)
    resolution_flags[i] = applyExtrapolationFunctionRecurs(extrapolate_owned_cells[i], owned_extrapolation_function);
}

template<typename GhostManagerType>
void GhostManagerTask<GhostManagerType>::continueExtrapolateTaskGhost(std::vector<bool> &resolution_flags) {
  for (int i{0}; i<extrapolate_ghost_cells.size(); ++i)
    resolution_flags[i] = applyExtrapolationFunctionRecurs(extrapolate_ghost_cells[i], ghost_extrapolation_function);
}

template<typename GhostManagerType>
void GhostManagerTask<GhostManagerType>::continueIgnoreTask(std::vector<bool> &resolution_flags) {
  std::fill(resolution_flags.begin(), resolution_flags.end(), true);
}

template<typename GhostManagerType>
void GhostManagerTask<GhostManagerType>::continueSplitInOwnerTaskGhost(std::vector<bool> &resolution_flags) {
  // For SPLIT_IN_OWNER strategy, we need to request the owner process to split its cell
  // This is a complex operation that requires communication between processes
  // TODO: Implement this
}

template<typename GhostManagerType>
void GhostManagerTask<GhostManagerType>::continueTryCoarseTaskGhost(std::vector<bool> &resolution_flags) {
  // For TRY_COARSEN strategy, we attempt to coarsen the ghost cell if possible
  // TODO: Implement this
}

