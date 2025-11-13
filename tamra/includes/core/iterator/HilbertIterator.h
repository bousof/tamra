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
#include <limits>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

#include "../manager/CellIdManager.h"
#include "AbstractTreeIterator.h"
#include "HilbertTables.h"

template<typename CellType>
class HilbertIterator : public AbstractTreeIterator<CellType> {
 public:
  using CellIdManagerType = typename AbstractTreeIterator<CellType>::CellIdManagerType;
  using ExtrapolationFunctionType = typename AbstractTreeIterator<CellType>::ExtrapolationFunctionType;
  static constexpr unsigned number_of_corners = 1u << CellType::number_split_dimensions;
  static constexpr unsigned number_of_orientations = CellType::number_split_dimensions * number_of_corners;

  //***********************************************************//
  //  DATA                                                     //
  //***********************************************************//
 protected:
  using AbstractTreeIterator<CellType>::index_path;
 private:
  // Possible leaf orientations
  const std::vector<unsigned> &possible_leaf_orientations;
  // Map order to sibling number for each mother orientation
  const std::array<std::array<unsigned, number_of_corners>, number_of_orientations> &child_orderings;
  // Map order to child cell orientation for each mother orientation
  const std::array<std::array<unsigned, number_of_corners>, number_of_orientations> &child_orientations;
  // Map sibling number to order for each mother orientation
  const std::array<std::array<unsigned, number_of_corners>, number_of_orientations> &reverse_child_orderings;
  // Default leaf orientation
  const unsigned default_leaf_orientation;
  // Root cell orientations
  std::vector<unsigned> root_cell_orientations;
  // Vector of orders
  std::vector<unsigned> orientation_path;

  //***********************************************************//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //***********************************************************//
 public:
  // Constructor
  HilbertIterator(const std::vector<std::shared_ptr<CellType>> &root_cells, const int max_level);

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
