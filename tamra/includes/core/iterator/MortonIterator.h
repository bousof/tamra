/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class for iterating through tree cells based on Morton space-filling curves.
 */

#pragma once

#include <array>
#include <limits>
#include <memory>
#include <stack>
#include <utility>
#include <vector>

#include "../manager/CellIdManager.h"
#include "AbstractTreeIterator.h"

template<typename CellType, short MORTON_ORIENTATION>
static constexpr std::array<unsigned, CellType::number_children> make_orderings();

template<typename CellType, short MORTON_ORIENTATION = 123>
class MortonIterator : public AbstractTreeIterator<CellType> {
 public:
  using CellIdManagerType = typename AbstractTreeIterator<CellType>::CellIdManagerType;
  using ExtrapolationFunctionType = typename AbstractTreeIterator<CellType>::ExtrapolationFunctionType;

  //***********************************************************//
  //  DATA                                                     //
  //***********************************************************//
 private:
  // Map order to sibling number for each mother orientation
  const std::array<unsigned, CellType::number_children> order_to_sibling_number;
  // Map sibling number to order for each mother orientation
  const std::array<unsigned, CellType::number_children> sibling_number_to_order;

  //***********************************************************//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //***********************************************************//
 public:
  // Constructor
  MortonIterator(const std::vector<std::shared_ptr<CellType>> &root_cells, const int max_level);

  //***********************************************************//
  //  ACCESSORS                                                //
  //***********************************************************//
 public:
 private:

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
 protected:
  // Converts the genealogy of a cell
  std::vector<unsigned> indexToOrderPath(const std::vector<unsigned> &index_path) const override;
  // Return the sibling number from the order (number along
  // the curve) with respect to the mother orientation.
  unsigned orderToSiblingNumber(unsigned order, const bool compute_orientation=false) const override;
};

#include "MortonIterator.tpp"
