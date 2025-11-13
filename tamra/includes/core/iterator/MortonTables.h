/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class for computing and providing Morton iterator ordering tables.
 *  Avoids recomputing tables at each instanciation of iterator.
 */

#pragma once

#include<array>

//***********************************************************//
//  PROTOTYPES.                                              //
//***********************************************************//

template<typename CellType, short MORTON_ORIENTATION>
constexpr std::array<unsigned, CellType::number_children> make_orderings();

template<size_t ARRAY_SIZE>
std::array<unsigned, ARRAY_SIZE> reverse_orderings(const std::array<unsigned, ARRAY_SIZE> &orderings);


//***********************************************************//
//  MAIN CLASS                                               //
//***********************************************************//

template<class CellType, short MORTON_ORIENTATION>
struct MortonTables {
  static const auto& get_orderings()         { static const auto t = make_orderings<CellType, MORTON_ORIENTATION>(); return t; }
  static const auto& get_reverse_orderings() { static const auto t = reverse_orderings<CellType::number_children>(get_orderings()); return t; }
};


//***********************************************************//
//  IMPLEMEBNTATIONS                                         //
//***********************************************************//

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
