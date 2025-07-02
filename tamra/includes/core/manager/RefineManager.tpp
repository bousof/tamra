#include "./RefineManager.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType>
RefineManager<CellType>::RefineManager(const int min_level, const int max_level, const int rank, const int size)
: min_level(min_level),
  max_level(max_level),
  rank(rank),
  size(size) {}

// Destructor
template<typename CellType>
RefineManager<CellType>::~RefineManager() {};


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//
// Go through all the root cells and call the recursive function
template<typename CellType>
void RefineManager<CellType>::refine(const std::vector< std::shared_ptr<CellType> >& root_cells) const {
  // Looping on all root cells
  for (const auto &root_cell: root_cells)
    // Recursively mesh cells at min level
    refineRecurs(root_cell);
}

// Refine child cells if needed or call function to child recursively
template<typename CellType>
void RefineManager<CellType>::refineRecurs(const std::shared_ptr<CellType>& cell) const {
  if (!cell->belongToThisProc() || cell->getLevel()>=max_level)
    return;
  if (!cell->isLeaf()) {
    std::array< std::shared_ptr<CellType>, CellType::number_children > child_cells = cell->getChildCells();

    for (const auto &child: child_cells)
      refineRecurs(child);
  } else if (cell->isToRefine())
    cell->split(max_level);
}
