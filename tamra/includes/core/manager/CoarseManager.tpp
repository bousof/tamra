#include "./CoarseManager.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType>
CoarseManager<CellType>::CoarseManager(const int min_level, const int max_level, const int rank, const int size)
: min_level(min_level),
  max_level(max_level),
  rank(rank),
  size(size) {}

// Destructor
template<typename CellType>
CoarseManager<CellType>::~CoarseManager() {};


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//
// Go through all the parent cells and coarse them one time if needed
template<typename CellType>
bool CoarseManager<CellType>::coarsen(const std::vector< std::shared_ptr<CellType> >& root_cells, InterpolationFunctionType interpolation_function) const {
  // Looping on levels starting from high to low level
  bool structure_changed = false;
  for (int coarse_level={max_level-1}; coarse_level>=min_level; --coarse_level)
    // Looping on all root cells
    for (const auto &root_cell: root_cells)
      // Recursively coarse cells at min level
      structure_changed |= coarsenToLevelRecurs(root_cell, coarse_level, interpolation_function);
  return structure_changed;
}

// Recursively coarse cells at a specific level
template<typename CellType>
bool CoarseManager<CellType>::coarsenToLevelRecurs(const std::shared_ptr<CellType>& cell, const int &coarse_level, InterpolationFunctionType interpolation_function) const {
  if (cell->isLeaf() || cell->getLevel()>coarse_level)
    return false;

  // If the cell is too low level we apply to children
  if (cell->getLevel()<coarse_level) {
    bool structure_changed = false;
    for (const auto &child: cell->getChildCells())
      structure_changed |= coarsenToLevelRecurs(child, coarse_level, interpolation_function);
    return structure_changed;
  }

  // Try to coarse the cell
  for (const auto &child: cell->getChildCells())
    if (!child->isToCoarse())
      return false;
  return cell->coarsen(min_level, interpolation_function);
}
