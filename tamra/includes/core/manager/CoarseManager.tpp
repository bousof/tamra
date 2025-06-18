#include "./CoarseManager.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType>
CoarseManager<CellType>::CoarseManager(const int min_level, const int max_level, const int rank, const int size)
: min_level(min_level), max_level(max_level), rank(rank), size(size) {}

// Destructor
template<typename CellType>
CoarseManager<CellType>::~CoarseManager() {};


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//
// Go through all the parent cells and coarse them one time if needed
template<typename CellType>
void CoarseManager<CellType>::coarsen(const std::vector< std::shared_ptr<CellType> >& root_cells) const {
  // Looping on all root cells
  for (const auto &root_cell: root_cells)
    // Looping on levels starting from high to low level
    for (int coarse_level={max_level-1}; coarse_level>=min_level; --coarse_level)
      // Recursively coarse cells at min level
      coarsenToLevelRecurs(root_cell, coarse_level);
}

// Recursively coarse cells at a specific level
template<typename CellType>
void CoarseManager<CellType>::coarsenToLevelRecurs(const std::shared_ptr<CellType>& cell, const int &coarse_level) const {
  if (cell->isLeaf() || cell->getLevel()>coarse_level)
    return;

  // If the cell is too low level we apply to children
  if (cell->getLevel()<coarse_level) {
    for (const auto &child: cell->getChildCells())
      coarsenToLevelRecurs(child, coarse_level);
    return;
  }

  // Try to coarse the cell
  for (const auto &child: cell->getChildCells())
    if (!child->isToCoarse())
      return;
  cell->coarsen(min_level);
}
