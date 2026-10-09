/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class for iterating through tree cells based on Morton space-filling curves.
 */

#pragma once

#include <algorithm>
#include <array>
#include <memory>
#include <numeric>
#include <vector>

#include <tamra/mesh/traversal/AbstractTreeIterator.h>
#include <tamra/mesh/traversal/CellIdManager.h>
#include <tamra/mesh/traversal/HilbertTables.h>


template<typename CellType>
class HilbertIterator : public AbstractTreeIterator<CellType> {
 public:
  static constexpr std::array<char, 4> CONFIG_SELECTION_NAME{
    'H', '0', '0', '0'
  };
  using CellIdManagerType = typename AbstractTreeIterator<CellType>::CellIdManagerType;
  using ExtrapolationFunctionType = typename AbstractTreeIterator<CellType>::ExtrapolationFunctionType;
  static constexpr unsigned corner_count = 1u << CellType::split_dimension_count;
  static constexpr unsigned orientation_count = CellType::split_dimension_count * corner_count;

  //-----------------------------------------------------------//
  //  DATA                                                     //
  //-----------------------------------------------------------//
 protected:
  using AbstractTreeIterator<CellType>::index_path;
 private:
  // Possible leaf orientations
  const std::vector<unsigned> &possible_leaf_orientations;
  // Map order to sibling number for each mother orientation
  const std::array<std::array<unsigned, corner_count>, orientation_count> &child_orderings;
  // Map order to child cell orientation for each mother orientation
  const std::array<std::array<unsigned, corner_count>, orientation_count> &child_orientations;
  // Map sibling number to order for each mother orientation
  const std::array<std::array<unsigned, corner_count>, orientation_count> &reverse_child_orderings;
  // Default leaf orientation
  const unsigned default_leaf_orientation;
  // Root cell orientations
  std::vector<unsigned> root_cell_orientations;
  // Vector of orders
  std::vector<unsigned> orientation_path;

  //-----------------------------------------------------------//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //-----------------------------------------------------------//
 public:
  // Constructor
  HilbertIterator(const std::vector<std::shared_ptr<CellType>> &root_cells, const unsigned max_level);

  //-----------------------------------------------------------//
  //  METHODS                                                  //
  //-----------------------------------------------------------//
 public:
 protected:
  // Converts the genealogy of a cell
  std::vector<unsigned> indexToOrderPath(const std::vector<unsigned> &index_path) const override;
  // Return the sibling number from the order (number along
  // the curve) with respect to the mother orientation.
  unsigned orderToSiblingNumber(unsigned order, const bool compute_orientation=false) const override;
  // Go to child cell
  void toChild(const unsigned order) override;
  // Go to parent cell
  void toParent() override;
  // Go to root cell
  void toRoot(const unsigned root_number) override;
 private:
  // Next orientation
  unsigned nextOrientation(unsigned order) const;
};

#include "HilbertIterator.tpp"
