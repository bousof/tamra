#include "MortonIterator.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType, short MORTON_ORIENTATION>
MortonIterator<CellType, MORTON_ORIENTATION>::MortonIterator(const std::vector<std::shared_ptr<CellType>> &root_cells, const unsigned max_level)
: AbstractTreeIterator<CellType>(root_cells, max_level),
  order_to_sibling_number(MortonTables<CellType, MORTON_ORIENTATION>::get_orderings()),
  sibling_number_to_order(MortonTables<CellType, MORTON_ORIENTATION>::get_reverse_orderings()) {};

//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Converts the genealogy of a cell
template<typename CellType, short MORTON_ORIENTATION>
std::vector<unsigned> MortonIterator<CellType, MORTON_ORIENTATION>::indexToOrderPath(const std::vector<unsigned> &index_path) const {
  std::vector<unsigned> order_path(index_path.size());
  order_path[0] = index_path[0];
  for (size_t i{1}; i<index_path.size(); ++i)
    order_path[i] = sibling_number_to_order[index_path[i]];
  return order_path;
}

// Return the sibling number from the order (number along
// the curve) with respect to the mother orientation.
// For Mortong Z curve sibling_number = order
template<typename CellType, short MORTON_ORIENTATION>
unsigned MortonIterator<CellType, MORTON_ORIENTATION>::orderToSiblingNumber(unsigned order, const bool) const {
  return order_to_sibling_number[order];
}
