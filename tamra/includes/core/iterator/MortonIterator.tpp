#include "MortonIterator.h"

template<typename CellType, short MORTON_ORIENTATION>
constexpr std::array<unsigned, CellType::number_children> make_orderings() {
  std::array<unsigned, CellType::number_children> orderings{};
  const unsigned Nx = std::max(CellType::Nx, 1),
                 Ny = std::max(CellType::Ny, 1),
                 Nz = std::max(CellType::Nz, 1);
  unsigned counter{0};
  if constexpr (MORTON_ORIENTATION==132)
    for (int j{0}; j<Ny; ++j)
      for (int k{0}; k<Nz; ++k)
        for (int i{0}; i<Nx; ++i)
          orderings[counter++] = i + j * Nx + k * Nx * Ny;
  else if constexpr (MORTON_ORIENTATION==213)
    for (int k{0}; k<Nz; ++k)
      for (int i{0}; i<Nx; ++i)
        for (int j{0}; j<Ny; ++j)
          orderings[counter++] = i + j * Nx + k * Nx * Ny;
  else if constexpr (MORTON_ORIENTATION==231)
    for (int i{0}; i<Nx; ++i)
      for (int k{0}; k<Nz; ++k)
        for (int j{0}; j<Ny; ++j)
          orderings[counter++] = i + j * Nx + k * Nx * Ny;
  else if constexpr (MORTON_ORIENTATION==312)
    for (int j{0}; j<Ny; ++j)
      for (int i{0}; i<Nx; ++i)
        for (int k{0}; k<Nz; ++k)
          orderings[counter++] = i + j * Nx + k * Nx * Ny;
  else if constexpr (MORTON_ORIENTATION==321)
    for (int i{0}; i<Nx; ++i)
      for (int j{0}; j<Ny; ++j)
        for (int k{0}; k<Nz; ++k)
          orderings[counter++] = i + j * Nx + k * Nx * Ny;
  else // MORTON_ORIENTATION==123 is default if unhandled value
    for (int k{0}; k<Nz; ++k)
      for (int j{0}; j<Ny; ++j)
        for (int i{0}; i<Nx; ++i)
          orderings[counter++] = i + j * Nx + k * Nx * Ny;
  return orderings;
}

template<size_t ARRAY_SIZE>
std::array<unsigned, ARRAY_SIZE> reverse_orderings(const std::array<unsigned, ARRAY_SIZE> &orderings) {
  std::array<unsigned, ARRAY_SIZE> reverse_orderings;
  for (int i{0}; i<ARRAY_SIZE; ++i)
    reverse_orderings[orderings[i]] = i;
  return reverse_orderings;
}

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType, short MORTON_ORIENTATION>
MortonIterator<CellType, MORTON_ORIENTATION>::MortonIterator(const std::vector<std::shared_ptr<CellType>> &root_cells, const int max_level)
: AbstractTreeIterator<CellType>(root_cells, max_level),
  order_to_sibling_number(make_orderings<CellType, MORTON_ORIENTATION>()),
  sibling_number_to_order(reverse_orderings<CellType::number_children>(order_to_sibling_number)) {};

//***********************************************************//
//  ACCESSORS                                                //
//***********************************************************//

//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Converts the genealogy of a cell
template<typename CellType, short MORTON_ORIENTATION>
std::vector<unsigned> MortonIterator<CellType, MORTON_ORIENTATION>::indexToOrderPath(const std::vector<unsigned> &index_path) const {
  std::vector<unsigned> order_path(index_path.size());
  order_path[0] = index_path[0];
  for (int i{1}; i<index_path.size(); ++i)
    order_path[i] = sibling_number_to_order[index_path[i]];
  return order_path;
}

// Return the sibling number from the order (number along
// the curve) with respect to the mother orientation.
// For Mortong Z curve sibling_number = order
template<typename CellType, short MORTON_ORIENTATION>
unsigned MortonIterator<CellType, MORTON_ORIENTATION>::orderToSiblingNumber(unsigned order, const bool compute_orientation) const {
  return order_to_sibling_number[order];
}
