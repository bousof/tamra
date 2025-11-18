#include "RootCellEntry.h"

//***********************************************************//
//  VARIABLES                                                //
//***********************************************************//

// Constructor
template<typename CellType>
RootCellEntry<CellType>::RootCellEntry(std::shared_ptr<CellType> cell)
: cell(cell) {
  neighbor_cells.fill(nullptr);
}

// Destructor
template<typename CellType>
RootCellEntry<CellType>::~RootCellEntry() {
  cell.reset();
  neighbor_cells.fill(nullptr);
}


//***********************************************************//
//  ACCESSORS                                                //
//***********************************************************//

// Get a neighbor cell
template<typename CellType>
std::shared_ptr<CellType> RootCellEntry<CellType>::getNeighbor(const int dir) const {
  if (dir<0 || dir>CellType::number_neighbors)
    throw std::runtime_error("Invalid neighbor index in RootCellEntry::getNeighbor()");
  return neighbor_cells[dir];
}


//***********************************************************//
//  MUTATORS                                                 //
//***********************************************************//

// Set a neighbor cell
template<typename CellType>
void RootCellEntry<CellType>::setNeighbor(int dir, std::shared_ptr<CellType> cell) {
  if (dir<0 || dir>CellType::number_neighbors)
    throw std::runtime_error("Invalid neighbor index in RootCellEntry::setNeighbor()");
  neighbor_cells[dir] = cell;
}
