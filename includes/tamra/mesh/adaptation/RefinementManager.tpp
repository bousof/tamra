#include "RefinementManager.h"

//-----------------------------------------------------------//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//-----------------------------------------------------------//

// Constructor
template<typename CellType>
RefinementManager<CellType>::RefinementManager(const unsigned min_level, const unsigned max_level, const unsigned rank, const unsigned size)
: min_level(min_level),
  max_level(max_level),
  rank(rank),
  size(size) {}

// Destructor
template<typename CellType>
RefinementManager<CellType>::~RefinementManager() {}


//-----------------------------------------------------------//
//  METHODS                                                  //
//-----------------------------------------------------------//
// Go through all the root cells and call the recursive function
template<typename CellType>
bool RefinementManager<CellType>::refine(const std::vector<std::shared_ptr<CellType>> &root_cells, ExtrapolationFunctionType extrapolation_function) const {
  // Looping on all root cells
  bool structure_changed = false;
  for (const auto &root_cell : root_cells)
    // Recursively mesh cells at min level
    structure_changed |= refineRecurs(root_cell, extrapolation_function);
  return structure_changed;
}

// Refine child cells if needed or call function to child recursively
template<typename CellType>
bool RefinementManager<CellType>::refineRecurs(const std::shared_ptr<CellType> &cell, ExtrapolationFunctionType extrapolation_function) const {
  if (!cell->isOwned() || (cell->getLevel()>=max_level))
    return false;

  bool structure_changed = false;
  if (!cell->isLeaf()) {
    std::array<std::shared_ptr<CellType>, CellType::child_count> child_cells = cell->getChildCells();

    for (const auto &child : child_cells)
      structure_changed |= refineRecurs(child, extrapolation_function);
  } else if (cell->isMarkedForRefinement()) {
    cell->split(max_level, extrapolation_function);
    structure_changed = true;
  }
  return structure_changed;
}
