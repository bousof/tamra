/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class that handles cells IDs with lower memory consuption.
 */

#pragma once

#include <vector>

#include "CellIdManager.h"

template<typename CellType>
class BitStackedCellIdManager: public CellIdManager<CellType> {
  //***********************************************************//
	//  VARIABLES                                                //
	//***********************************************************//
 protected:
  // Cell ID unsigned int slots
  unsigned bits_for_root;
  unsigned bits_for_level;
  unsigned bits_per_child;
  unsigned bits_per_unsigned;
  unsigned nb_child_first_unsigned;
  unsigned nb_child_unsigned;
  unsigned mask_level;
  unsigned mask_root;
  unsigned mask_child;

  //***********************************************************//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //***********************************************************//
 public:
  // Constructor
  BitStackedCellIdManager(const int number_root_cells, const int max_level);
  // Destructor
  ~BitStackedCellIdManager() = default;

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 private:
  // Extract the cell_id level
  int getIdLevel(const std::vector<unsigned> &cell_id) const override;
  // Edit the cell_id level
  void setIdLevel(std::vector<unsigned> &cell_id, const int level) const override;
  // Extract the cell_id root index
  unsigned getIdRoot(const std::vector<unsigned> &cell_id) const override;
  // Edit the cell_id root index and return the old one
  void setIdRoot(std::vector<unsigned> &cell_id, const unsigned root_number) const override;
  // Extract the cell_id child index
  unsigned getIdChild(const std::vector<unsigned> &cell_id, const int level) const override;
  // Edit the cell_id child index and return the old one
  void setIdChild(std::vector<unsigned> &cell_id, const int level, const unsigned child_index) const override;
};

#include "BitStackedCellIdManager.tpp"
