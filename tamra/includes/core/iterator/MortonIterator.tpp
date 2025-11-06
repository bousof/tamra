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

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

//***********************************************************//
//  ACCESSORS                                                //
//***********************************************************//

//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Return the sibling number from the order (number along
// the curve) with respect to the mother orientation.
// For Mortong Z curve sibling_number = order
template<typename CellType, short MORTON_ORIENTATION>
unsigned MortonIterator<CellType, MORTON_ORIENTATION>::orderToSiblingNumber(unsigned order, const bool compute_orientation) {
  return order_to_sibling_number[order];
}
