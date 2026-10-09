#include "CoarseningManager.h"

//-----------------------------------------------------------//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//-----------------------------------------------------------//

// Constructor
template<typename CellType>
CoarseningManager<CellType>::CoarseningManager(const unsigned min_level, const unsigned max_level, const unsigned rank, const unsigned size)
: min_level(min_level),
  max_level(max_level),
  rank(rank),
  size(size) {}

// Destructor
template<typename CellType>
CoarseningManager<CellType>::~CoarseningManager() {}


//-----------------------------------------------------------//
//  METHODS                                                  //
//-----------------------------------------------------------//
// Go through all the parent cells and coarsen them one time if needed
template<typename CellType>
bool CoarseningManager<CellType>::coarsen(const std::vector<std::shared_ptr<CellType>> &root_cells, InterpolationFunctionType interpolation_function) const {
  // Looping on levels starting from high to low level
  bool structure_changed = false;
  for (unsigned coarse_level=(max_level-1); coarse_level>=min_level; --coarse_level)
    // Looping on all root cells
    for (const auto &root_cell : root_cells)
      // Recursively coarsen cells at min level
      structure_changed |= coarsenToLevelRecurs(root_cell, coarse_level, interpolation_function);
  return structure_changed;
}

// Recursively coarsen cells at a specific level
template<typename CellType>
bool CoarseningManager<CellType>::coarsenToLevelRecurs(const std::shared_ptr<CellType> &cell, const unsigned coarse_level, InterpolationFunctionType interpolation_function) const {
  if (cell->isLeaf() || cell->getLevel()>coarse_level)
    return false;

  // If the cell is too low level we apply to children
  if (cell->getLevel() < coarse_level) {
    bool structure_changed = false;
    for (const auto &child : cell->getChildCells())
      structure_changed |= coarsenToLevelRecurs(child, coarse_level, interpolation_function);
    return structure_changed;
  }

  // Try to coarsen the cell
  for (const auto &child : cell->getChildCells())
    if (!child->isMarkedForCoarsening())
      return false;
  return cell->coarsen(min_level, interpolation_function);
}
