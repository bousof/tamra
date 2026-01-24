/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Abstract class for iterating through tree cells.
 */

#pragma once

#include <limits>
#include <memory>
#include <stack>
#include <utility>
#include <vector>

#include "../manager/CellIdManager.h"

template<typename CellType>
class AbstractTreeIterator {
 public:
  using CellIdManagerType = CellIdManager<CellType>;
  using ExtrapolationFunctionType = std::function<void(const std::shared_ptr<CellType>&)>;

  //***********************************************************//
  //  DATA                                                     //
  //***********************************************************//
 private:
  // Root cells
  const std::vector<std::shared_ptr<CellType>> root_cells;
  // Tree max level
  const unsigned max_level;
 protected:
  // Vector of sibling numbers
  std::vector<unsigned> index_path;
 private:
  // Vector of orders
  std::vector<unsigned> order_path;
  // Current cell pointed by iterator
  std::shared_ptr<CellType> current_cell;
  // Size of partition for each level
  std::vector<size_t> level_partition_sizes;
  // Partition of the current cell
  std::pair<int, int> current_cell_partition;
  // Partition of the current cell
  std::vector<unsigned> current_cell_id;
  // Cell ID manager
  CellIdManagerType cell_id_manager;

  //***********************************************************//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //***********************************************************//
 public:
  // Constructor
  AbstractTreeIterator(const std::vector<std::shared_ptr<CellType>> &root_cells, const unsigned max_level);
  // Destructor
  virtual ~AbstractTreeIterator() = default;

  //***********************************************************//
  //  ACCESSORS                                                //
  //***********************************************************//
 public:
  // Get current cell
  std::shared_ptr<CellType> getCell() const;
  // Get current cell partition
  const std::pair<int, int>& getPartition() const;
  // Get current index path
  const std::vector<unsigned>& getIndexPath() const;
  // Get current order path
  const std::vector<unsigned>& getOrderPath() const;
  // Get current cell ID
  std::vector<unsigned> getCellId() const;
  // Get cell ID manager
  CellIdManagerType getCellIdManager() const;
  // Construct cell index path
  std::vector<unsigned> getCellId(const std::shared_ptr<CellType> &cell) const;
 private:
  // Construct cell index path
  std::vector<unsigned> getCellIndexPath(const std::shared_ptr<CellType> &cell) const;

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Go to the next leaf cell
  bool next(const unsigned sweep_level = std::numeric_limits<int>::max());
  // Go to the next leaf cell belonging to this proc
  bool ownedNext(const unsigned sweep_level = std::numeric_limits<int>::max());
  // Go to the previous leaf cell
  bool prev(const unsigned sweep_level = std::numeric_limits<int>::max());
  // Go to the previous leaf cell belonging to this proc
  bool ownedPrev(const unsigned sweep_level = std::numeric_limits<int>::max());
  // Go to the first leaf cell of first root
  void toBegin(const unsigned sweep_level = std::numeric_limits<int>::max());
  // Go to the first leaf cell of first root belonging to this process
  bool toOwnedBegin(const unsigned sweep_level = std::numeric_limits<int>::max());
  // Go to the last leaf cell of last root
  void toEnd(const unsigned sweep_level = std::numeric_limits<int>::max());
  // Go to the last leaf cell of last root belonging to this process
  bool toOwnedEnd(const unsigned sweep_level = std::numeric_limits<int>::max());
  // Moves the iterator to a leaf cell of the current cell
  void toLeaf(const unsigned sweep_level = std::numeric_limits<int>::max(), const bool reverse = false);
  // Moves the iterator to a leaf cell of the current cell that belong to the process
  void toOwnedLeaf(const unsigned sweep_level = std::numeric_limits<int>::max(), const bool reverse = false);
  // Move iterator to a specific cell ID (can also create it with a flag)
  void toCellId(const std::vector<unsigned> &cell_id, const bool create = false, ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<CellType> &cell) { (void)cell; });
  // Check if a cell ID is greater than
  bool cellIdGt(const std::vector<unsigned> &cell_id) const { return cell_id_manager.cellIdGt(current_cell_id, cell_id); }
  // Check if a cell ID is greater than or equal another ID
  bool cellIdGte(const std::vector<unsigned> &cell_id) const { return cell_id_manager.cellIdGte(current_cell_id, cell_id); }
  // Check if a cell ID is smaller than
  bool cellIdLt(const std::vector<unsigned> &cell_id) const { return cell_id_manager.cellIdLt(current_cell_id, cell_id); }
  // Check if a cell ID is smaller than or equal another ID
  bool cellIdLte(const std::vector<unsigned> &cell_id) const { return cell_id_manager.cellIdLte(current_cell_id, cell_id); }
  // Generate an ID from the genealogy of a cell
  std::vector<unsigned> indexPathToId(const std::vector<unsigned> &index_path) const;
  // Generate an ID from the genealogy of a cell
  std::vector<unsigned> orderPathToId(const std::vector<unsigned> &order_path) const;
  // Generate an ID from the genealogy of a cell
  std::vector<unsigned> idToOrderPath(const std::vector<unsigned> &cell_id) const;
 protected:
  // Converts the genealogy of a cell
  virtual std::vector<unsigned> indexToOrderPath(const std::vector<unsigned> &index_path) const = 0;
  // Return the sibling number from the order (number along
  // the curve) with respect to the mother orientation.
  virtual unsigned orderToSiblingNumber(unsigned order, const bool compute_orientation=false) const = 0;
  // Go to child cell
  virtual void toChild(const unsigned order);
  // Go to parent cell
  virtual void toParent();
  // Go to root cell
  virtual void toRoot(const unsigned root_number);
  // Return the child cell from order and mother cell orientation (obtained by following the curve)
  std::shared_ptr<CellType> getChildCellFromOrder(std::shared_ptr<CellType> cell, unsigned order, const bool compute_orientation=false);
};

#include "AbstractTreeIterator.tpp"
