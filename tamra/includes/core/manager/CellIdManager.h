/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class that handles cells IDs.
 */

#pragma once

#include <vector>

template<typename CellType>
class CellIdManager {
  //***********************************************************//
	//  VARIABLES                                                //
	//***********************************************************//
 protected:
  // Number of root cells
  const int number_root_cells;
  // Tree max level
  const int max_level;
  // Cell ID unsigned int slots
  unsigned cell_id_size;

  //***********************************************************//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //***********************************************************//
 public:
  // Constructor
  CellIdManager(const int number_root_cells, const int max_level);
  // Destructor
  ~CellIdManager() = default;

  //***********************************************************//
  //  ACCESSOR                                                 //
  //***********************************************************//
 public:
  // Get the size of cell ID vector
  unsigned getCellIdSize() const { return cell_id_size; }
  
  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Generate an ID from the genealogy of a cell.
  std::vector<unsigned> indexPathToId(const std::vector<unsigned> &index_path) const;
  // Generate an ID from the genealogy of a cell.
  std::vector<unsigned> idToIndexPath(const std::vector<unsigned> &cell_id) const;
  // Generate the IDs of the first and last leaf cells of the
  // partitions obatined by splitting into equal parts and taking
  // the n-th one
  std::vector< std::vector<unsigned> > getEqualPartitions(const int level, const int size);
  // Moves the cell ID to child cell
  void toChild(std::vector<unsigned> &cell_id, const unsigned order);
  // Moves the cell ID to parent cell
  void toParent(std::vector<unsigned> &cell_id);
  // Moves the cell ID to root cell
  void toRoot(std::vector<unsigned> &cell_id, const unsigned root_number);
  // Reset cell ID
  void resetCellID(std::vector<unsigned> &cell_id);
  // Check if a cell ID is greater than
  bool cellIdGt(std::vector<unsigned> &cell_id_1, std::vector<unsigned> &cell_id_2, bool &sure);
  bool cellIdGt(std::vector<unsigned> &cell_id_1, std::vector<unsigned> &cell_id_2);
  // Check if a cell ID is greater than or equal another ID
  bool cellIdGte(std::vector<unsigned> &cell_id_1, std::vector<unsigned> &cell_id_2);
  // Check if a cell ID is smaller than
  bool cellIdLt(std::vector<unsigned> &cell_id_1, std::vector<unsigned> &cell_id_2, bool &sure);
  bool cellIdLt(std::vector<unsigned> &cell_id_1, std::vector<unsigned> &cell_id_2);
  // Check if a cell ID is smaller than or equal another ID
  bool cellIdLte(std::vector<unsigned> &cell_id_1, std::vector<unsigned> &cell_id_2);
 protected:
  // Extract the cell_id level
  virtual int getIdLevel(const std::vector<unsigned> &cell_id) const;
  // Edit the cell_id level
  virtual void setIdLevel(std::vector<unsigned> &cell_id, const int level) const;
  // Extract the cell_id root index
  virtual unsigned getIdRoot(const std::vector<unsigned> &cell_id) const;
  // Edit the cell_id root index and return the old one
  virtual void setIdRoot(std::vector<unsigned> &cell_id, const unsigned root_number) const;
  // Extract the cell_id child index
  virtual unsigned getIdChild(const std::vector<unsigned> &cell_id, const int level) const;
  // Edit the cell_id child index and return the old one
  virtual void setIdChild(std::vector<unsigned> &cell_id, const int level, const unsigned child_index) const;
 private:
  // Moves the cell ID to a leaf cell
  void toLeaf(std::vector<unsigned> &cell_id, const int sweep_level, const bool reverse);
};

#include "CellIdManager.tpp"
