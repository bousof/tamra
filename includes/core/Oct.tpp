#include "Oct.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType>
Oct<CellType>::Oct() {}

// Destructor
template<typename CellType>
Oct<CellType>::~Oct() {}

// Oct initializer
template<typename CellType>
void Oct<CellType>::init(std::shared_ptr<CellType> parent_cell, unsigned level) {
  this->parent_cell = parent_cell;
  this->level = level;
  neighbor_cells.fill(nullptr);
  child_cells.fill(nullptr);
}

// Clear oct
template<typename CellType>
void Oct<CellType>::clear() {
  for (auto &child : child_cells)
    if (child)
      child->clear();
  reset();
}

// Reset oct
template<typename CellType>
void Oct<CellType>::reset() {
  parent_cell.reset();
  level = 0;
  neighbor_cells.fill(nullptr);
  child_cells.fill(nullptr);
}


//***********************************************************//
//  ACCESSORS                                                //
//***********************************************************//

// Get parent cell
template<typename CellType>
std::shared_ptr<CellType> Oct<CellType>::getParentCell() const { return parent_cell; };

// Get level of the oct
template<typename CellType>
unsigned Oct<CellType>::getLevel() const { return level; };

// Get neighbor cells
template<typename CellType>
const std::array<std::shared_ptr<CellType>, Oct<CellType>::number_neighbors>& Oct<CellType>::getNeighborCells() const { return neighbor_cells; };

// Get child cells
template<typename CellType>
const std::array<std::shared_ptr<CellType>, Oct<CellType>::number_children>& Oct<CellType>::getChildCells() const { return child_cells; };

// Get a specific child cell
template<typename CellType>
std::shared_ptr<CellType> Oct<CellType>::getChildCell(const unsigned sibling_number) const {
  if (sibling_number>=number_children)
    throw std::runtime_error("Invalid siblign number in Oct::getChildCell()");
  return child_cells[sibling_number];
};


//***********************************************************//
//  MUTATORS                                                 //
//***********************************************************//

// Set the parent cell
template<typename CellType>
void Oct<CellType>::setParentCell(std::shared_ptr<CellType> cell) {
  parent_cell = cell;
}

// Set the neighbor cell (only for direct neighbors)
template<typename CellType>
void Oct<CellType>::setNeighborCell(const unsigned dir, std::shared_ptr<CellType> cell) {
  if (dir>=number_neighbors)
    throw std::runtime_error("Cannot set neighbor in Oct::setNeighborCell()");
  neighbor_cells[dir] = cell;
};

// Set a specific child cell
template<typename CellType>
void Oct<CellType>::setChildCell(const unsigned sibling_number, std::shared_ptr<CellType> cell) {
  if (sibling_number<0 || sibling_number>=number_children)
    throw std::runtime_error("Cannot set child in Oct::setChildCell()");
  child_cells[sibling_number] = cell;
};


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Get the sibling number (position of the cell in the child_cells array)
template<typename CellType>
unsigned Oct<CellType>::getSiblingNumber(const CellType* ptr_child_cell) const {
  for (size_t i{0}; i<child_cells.size(); ++i)
    if (child_cells[i].get() == ptr_child_cell)
      return i;
  throw std::runtime_error("Child Cell not found in Oct::getSiblingNumber()");
}

// Get a pointer to a neighbor cell
template<typename CellType>
std::shared_ptr<CellType> Oct<CellType>::getNeighborCell(const unsigned dir) const {
  if (dir>=number_neighbors)
    throw std::runtime_error("Invalid direct neighbor direction in Oct::getNeighborCell()");

  return neighbor_cells[dir];
}
