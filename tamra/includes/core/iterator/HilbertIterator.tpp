#include "HilbertIterator.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template <typename CellType>
HilbertIterator<CellType>::HilbertIterator(const std::vector<std::shared_ptr<CellType>> &root_cells, const int max_level)
: AbstractTreeIterator<CellType>(root_cells, max_level),
  possible_leaf_orientations(HilbertTables<CellType>::get_possible_leaf_orientations()),
  child_orderings(HilbertTables<CellType>::child_orderings()),
  child_orientations(HilbertTables<CellType>::child_orientations()),
  reverse_child_orderings(HilbertTables<CellType>::reverse_child_orderings()),
  default_leaf_orientation(possible_leaf_orientations[0]) {

  root_cell_orientations = std::vector<unsigned>(root_cells.size(), default_leaf_orientation);
  orientation_path.reserve(max_level+1);
  orientation_path.resize(1);
  orientation_path[0] = root_cell_orientations[index_path[0]];
};

//***********************************************************//
//  ACCESSORS                                                //
//***********************************************************//

//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Converts the genealogy of a cell
template <typename CellType>
std::vector<unsigned> HilbertIterator<CellType>::indexToOrderPath(const std::vector<unsigned> &index_path) const {
  std::vector<unsigned> order_path(index_path.size());
  order_path[0] = index_path[0];
  unsigned orientation = root_cell_orientations[index_path[0]];
  for (int i{1}; i<index_path.size(); ++i) {
    order_path[i] = reverse_child_orderings[orientation][index_path[i]];
    orientation = child_orientations[orientation][order_path[i]];
  }
  return order_path;
}

// Return the sibling number from the order (number along
// the curve) with respect to the mother orientation.
// For Mortong Z curve sibling_number = order
template <typename CellType>
unsigned HilbertIterator<CellType>::orderToSiblingNumber(unsigned order, const bool compute_orientation) const {
  if (compute_orientation)
    return child_orderings[nextOrientation(order)][order];
  else
    return child_orderings[orientation_path.back()][order];
}

// Go to child cell
template <typename CellType>
void HilbertIterator<CellType>::toChild(const unsigned order) {
  orientation_path.push_back(nextOrientation(order));
  AbstractTreeIterator<CellType>::toChild(order);
}

// Go to parent cell
template <typename CellType>
void HilbertIterator<CellType>::toParent() {
  orientation_path.pop_back();
  AbstractTreeIterator<CellType>::toParent();
}

// Go to parent cell
template <typename CellType>
void HilbertIterator<CellType>::toRoot(const unsigned root_number) {
  orientation_path.clear();
  orientation_path.push_back(root_cell_orientations[root_number]);
  AbstractTreeIterator<CellType>::toRoot(root_number);
}

// Next orientation
template <typename CellType>
unsigned HilbertIterator<CellType>::nextOrientation(unsigned order) const {
  return child_orientations[orientation_path.back()][order];
}
