/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class for storing root cells and their neighbors in order to facilitate tree intialization.
 */

#pragma once

#include <array>
#include <memory>

template<typename CellType>
struct RootCellEntry {
 public:
  // Root cell
  std::shared_ptr<CellType> cell;
  // Associated neighbor cells
  std::array<std::shared_ptr<CellType>, CellType::number_neighbors> neighbor_cells;

  //***********************************************************//
	//  VARIABLES                                                //
	//***********************************************************//
 public:
  // Constructor
  RootCellEntry(std::shared_ptr<CellType> cell);
  ~RootCellEntry();

	//***********************************************************//
	//  ACCESSORS                                                //
	//***********************************************************//
 public:
  // Get a neighbor cell
  std::shared_ptr<CellType> getNeighbor(const unsigned dir) const;

  //***********************************************************//
	//  MUTATORS                                                 //
	//***********************************************************//
 public:
  // Set a neighbor cell
  void setNeighbor(unsigned dir, std::shared_ptr<CellType> cell);
};

#include "RootCellEntry.tpp"
