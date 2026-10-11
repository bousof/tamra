/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class that handles cells IDs.
 */

#pragma once

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>


template<typename CellType>
class CellIdManager {
  //-----------------------------------------------------------//
	//  VARIABLES                                                //
	//-----------------------------------------------------------//
 protected:
  // Number of root cells
  const unsigned root_cell_count;
  // Tree max level
  const unsigned max_level;
  // Cell ID unsigned int slots
  unsigned cell_id_size;

  //-----------------------------------------------------------//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //-----------------------------------------------------------//
 public:
  // Constructor
  CellIdManager(const unsigned root_cell_count, const unsigned max_level);
  // Destructor
  ~CellIdManager() = default;

  //-----------------------------------------------------------//
  //  ACCESSOR                                                 //
  //-----------------------------------------------------------//
 public:
  // Get the size of cell ID vector
  unsigned getCellIdSize() const { return cell_id_size; }

  //-----------------------------------------------------------//
  //  METHODS                                                  //
  //-----------------------------------------------------------//
 public:
  // Generate an ID from the genealogy of a cell.
  std::vector<unsigned> orderPathToId(const std::vector<unsigned> &order_path) const;
  // Generate an ID from the genealogy of a cell.
  std::vector<unsigned> idToOrderPath(const std::vector<unsigned> &cell_id) const;
  // Start IDs of equal partitions at the given level. Each end is the next
  // start (exclusive); repeated starts represent empty partitions.
  std::vector<std::vector<unsigned>> getEqualPartitions(const unsigned level, const unsigned size) const;
  // Moves the cell ID to child cell
  void toChild(std::vector<unsigned> &cell_id, const unsigned order) const;
  // Moves the cell ID to parent cell
  void toParent(std::vector<unsigned> &cell_id) const;
  // Moves the cell ID to root cell
  void toRoot(std::vector<unsigned> &cell_id, const unsigned root_number) const;
  // Reset cell ID
  void resetCellId(std::vector<unsigned> &cell_id) const;
  // Check if a cell ID is greater than
  // True if 1.start > 2.end
  bool cellIdGt(const std::vector<unsigned> &cell_id_1, const std::vector<unsigned> &cell_id_2) const;
 private:
  bool cellIdGt(const std::vector<unsigned> &cell_id_1, const std::vector<unsigned> &cell_id_2, bool &sure) const;
 public:
  // Check if a cell ID is greater than or equal another ID
  // True if 1.start >= 2.start
  bool cellIdGte(const std::vector<unsigned> &cell_id_1, const std::vector<unsigned> &cell_id_2) const;
  // Check if a cell ID is smaller than
  // True if 1.end < 2.start
  bool cellIdLt(const std::vector<unsigned> &cell_id_1, const std::vector<unsigned> &cell_id_2) const;
 private:
  bool cellIdLt(const std::vector<unsigned> &cell_id_1, const std::vector<unsigned> &cell_id_2, bool &sure) const;
 public:
  // Check if a cell ID is smaller than or equal another ID
  // True if 1.end <= 2.end
  bool cellIdLte(const std::vector<unsigned> &cell_id_1, const std::vector<unsigned> &cell_id_2) const;
 protected:
  // Extract the cell_id level
  virtual int getIdLevel(const std::vector<unsigned> &cell_id) const;
  // Edit the cell_id level
  virtual void setIdLevel(std::vector<unsigned> &cell_id, const unsigned level) const;
  // Extract the cell_id root index
  virtual unsigned getIdRoot(const std::vector<unsigned> &cell_id) const;
  // Edit the cell_id root index and return the old one
  virtual void setIdRoot(std::vector<unsigned> &cell_id, const unsigned root_number) const;
  // Extract the cell_id child index
  virtual unsigned getIdChild(const std::vector<unsigned> &cell_id, const unsigned level) const;
  // Edit the cell_id child index and return the old one
  virtual void setIdChild(std::vector<unsigned> &cell_id, const unsigned level, const unsigned child_index) const;
 private:
  // Moves the cell ID to a leaf cell
  void toLeaf(std::vector<unsigned> &cell_id, const unsigned sweep_level, const bool reverse) const;
};

#include "CellIdManager.tpp"
