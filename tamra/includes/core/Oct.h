/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class for oct manipulations.
 */

#pragma once

#include <array>
#include <memory>

template<typename CellType> class Oct;

#include"Cell.h"

template<typename CellType>
class Oct {
 public:
  static constexpr int number_children = CellType::number_children;
  static constexpr int number_neighbors = CellType::number_neighbors;
  static constexpr int number_plane_neighbors = CellType::number_plane_neighbors;
  static constexpr int number_volume_neighbors = CellType::number_volume_neighbors;

  //***********************************************************//
	//  VARIABLES                                                //
	//***********************************************************//
 private:
  // Oct parent
  std::shared_ptr<CellType> parent_cell;
  // Oct level
  int level;
  // Neighbor cells
  std::array< std::shared_ptr<CellType>, number_neighbors > neighbor_cells;
  // Child cells
  std::array< std::shared_ptr<CellType>, number_children > child_cells;

	//***********************************************************//
	//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
	//***********************************************************//
 public:
  // Constructor
  Oct();
  // Destructor
  ~Oct();
  // Oct initializer
  void init(std::shared_ptr<CellType> parent_cell, int level);
  // Clear oct
  void clear();
  // Reset oct
  void reset();

	//***********************************************************//
	//  ACCESSORS                                                //
	//***********************************************************//
 public:
  // Get parent cell
  std::shared_ptr<CellType> getParentCell() const;
  // Get level of the oct
  int getLevel() const;
  // Get neighbor cells
  const std::array< std::shared_ptr<CellType>, number_neighbors >& getNeighborCells() const;
  // Get child cells
  const std::array< std::shared_ptr<CellType>, number_children >& getChildCells() const;
  // Get a specific child cell
  std::shared_ptr<CellType> getChildCell(const int sibling_number) const;

  //***********************************************************//
	//  MUTATORS                                                 //
	//***********************************************************//
 public:
  // Set the parent cell
  void setParentCell(std::shared_ptr<CellType> cell);
  // Set the neighbor cell (only for direct neighbors)
  void setNeighborCell(const int dir, std::shared_ptr<CellType> cell);
  // Set a specific child cell
  void setChildCell(const int sibling_number, std::shared_ptr<CellType> cell);

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Get the sibling number (position of the cell in the child_cells array)
  unsigned getSiblingNumber(const CellType* ptr_child_cell) const;
  // Get a pointer to a neighbor cell
  std::shared_ptr<CellType> getNeighborCell(const int dir) const;
 private:
  // Get a pointer to a neighbor cell accessible by 2 consecutive othogonal direction (corners in 2D)
  std::shared_ptr<CellType> getPlaneNeighborCell(const int dir1, const int dir2) const;
  // Get a pointer to a neighbor cell accessible by 3 consecutive othogonal direction (corners in 3D)
  std::shared_ptr<CellType> getVolumeNeighborCell(const int dir1, const int dir2, const int dir3) const;
};

#include "Oct.tpp"
