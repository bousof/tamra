/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class for cell manipulations.
 */

#pragma once

#include <array>
#include <functional>
#include <memory>
#include <tuple>

#include "ChildAndDirectionTables.h"

template<int NX, int NY, int NZ, typename DataType> class Cell;

#include "CellData.h"
#include "Oct.h"

template<int NX = 2, int NY = 0, int NZ = 0, typename DataType = CellData>
class Cell {
 public:
  static constexpr int Nx = NX;
  static constexpr int Ny = NY;
  static constexpr int Nz = NZ;
  using CellDataType = DataType;
  using ChildAndDirectionTablesType = ChildAndDirectionTables<Nx, Ny, Nz>;
  using ExtrapolationFunctionType = std::function<void(const std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>&)>;
  using InterpolationFunctionType = std::function<void(const std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>&)>;
  using OctType = Oct<Cell<Nx, Ny, Nz, DataType>>;
  static constexpr int number_dimensions = ChildAndDirectionTablesType::number_dimensions;
  static constexpr int number_split_dimensions = ChildAndDirectionTablesType::number_split_dimensions;
  static constexpr int number_neighbors = ChildAndDirectionTablesType::number_neighbors;
  static constexpr int number_plane_neighbors = ChildAndDirectionTablesType::number_plane_neighbors;
  static constexpr int number_volume_neighbors = ChildAndDirectionTablesType::number_volume_neighbors;
  static constexpr int number_children = ChildAndDirectionTablesType::number_children;

  //***********************************************************//
  //  VARIABLES                                                //
  //***********************************************************//
 private:
  std::unique_ptr<DataType> data;
  std::shared_ptr<OctType> parent_oct;
  std::shared_ptr<OctType> child_oct;
  // Cell flags indicator
	// - 0 : BELONG TO THIS  PROC  &  DOESN'T NEED TO BE CHANGED
	// - 1 : BELONG TO THIS  PROC  &  NEED TO BE REFINED
	// - 2 : BELONG TO THIS  PROC  &  NEED TO BE COARSENED
	// - 3 : BELONG TO OTHER PROC  &  DOESN'T NEED TO BE CHANGED
	// - 4 : BELONG TO OTHER PROC  &  NEED TO BE REFINED
	// - 5 : BELONG TO OTHER PROC  &  NEED TO BE COARSENED
	// - 6 : IS BOUNDARY CELL      &  DOESN'T NEED TO BE CHANGED
	// - 7 : IS BOUNDARY CELL      &  NEED TO BE REFINED
	// - 8 : IS BOUNDARY CELL      &  NEED TO BE COARSENED
  int indicator;

  //***********************************************************//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //***********************************************************//
 public:
  // Constructor
  Cell();
  // Constructor (parent_oct=nullptr for root cell)
  Cell(std::shared_ptr<OctType> parent_oct, int indicator=0);
  // Destructor
  ~Cell();
  // Clear cell
  void clear();
  // Reset cell
  void reset();

  //***********************************************************//
  //  ACCESSORS                                                //
  //***********************************************************//
 public:
  // Get child oct
  std::shared_ptr<OctType> getChildOct() const;
  // Get a specific child cell
  std::shared_ptr<Cell> getChildCell(const unsigned sibling_number) const;
  // Get child cells
  const std::array<std::shared_ptr<Cell>, number_children>& getChildCells() const;
  // Get child cells in a specific direction
  const std::vector<std::shared_ptr<Cell>> getDirChildCells(const int dir) const;
  // Get level of the cell
  unsigned getLevel() const;
  // Get parent oct
  std::shared_ptr<OctType> getParentOct() const;
  // Get the sibling number (position of the cell in the parent oct child_cells array)
  unsigned getSiblingNumber() const;
  // Get cell data
  DataType& getCellData() const { return *data; };
  // Get the computation load of the cell
  double getLoad() const;
  // Flags accessors
  bool belongToThisProc() const  { return (indicator < 3); }
  bool belongToOtherProc() const { return (indicator >= 3) && (indicator < 6); }
  bool isBoundaryCell() const    { return (indicator >= 6); }
  bool isToRefine() const        { return (indicator%3) == 1; }
  bool isToCoarse() const        { return (indicator%3) == 2; }
 private:
  // Get the cell as a smart pointer for referencing
  std::shared_ptr<Cell> thisAsSmartPtr() const;

  //***********************************************************//
	//  MUTATORS                                                 //
	//***********************************************************//
 public:
  // Set cell data
  void setCellData(std::unique_ptr<DataType> &&new_data) { data = std::move(new_data); };
  // Flags mutators
  void setToThisProc()  { indicator = indicator%3; }
  void setToOtherProc() { indicator = 3 + indicator%3; }
  void setToUnchange()  { indicator = indicator - (indicator%3); }
  void setToRefine()    { indicator = indicator + 1 - (indicator%3); }
  void setToCoarse()    { indicator = indicator + 2 - (indicator%3); }
  void setToThisProcRecurs() {
    setToThisProc();
    if (!isLeaf())
      for (const auto &child : getChildCells())
        child->setToThisProcRecurs();
  }
  void setToOtherProcRecurs() {
    setToOtherProc();
    if (!isLeaf())
      for (const auto &child : getChildCells())
        child->setToOtherProcRecurs();
  }
  void setToUnchangeRecurs() {
    setToUnchange();
    if (!isLeaf())
      for (const auto &child : getChildCells())
        child->setToUnchangeRecurs();
  }
  void setToRefineRecurs() {
    setToRefine();
    if (!isLeaf())
      for (const auto &child : getChildCells())
        child->setToRefineRecurs();
  }
  void setToCoarseRecurs() {
    setToCoarse();
    if (!isLeaf())
      for (const auto &child : getChildCells())
        child->setToCoarseRecurs();
  }
 private:
  bool setToBoundary() { return indicator = 6+indicator%3; }

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // True if leaf cell (no child oct)
  bool isLeaf() const;
  // True if root cell (no parent oct)
  bool isRoot() const;
  // Count the number of leaf cells
  unsigned countLeaves() const;
  // Count the number of owned leaf cells
  unsigned countOwnedLeaves() const;
  // Split a root cell (a pointer to the root is needed for back reference in child oct)
  const std::array<std::shared_ptr<Cell>, number_children>& splitRoot(const int max_level, std::shared_ptr<Cell> root_cell, ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<Cell> &cell) {});
  // Split a cell and it's direct neighbors if needed for mesh conformity
  const std::array<std::shared_ptr<Cell>, number_children>& split(const int max_level, ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<Cell> &cell) {});
  // Coarsen a cell if neighbors cell allow to preserve consistency else nothing is done
  bool coarsen(const int min_level, InterpolationFunctionType interpolation_function = [](const std::shared_ptr<Cell> &cell) {});
  //┌────────────┬──────────────────┬──────────────────────────────────┐
  //│  Priority  │   Available if   │   Indexes (dir)                  │
  //├────────────┼──────────────────┼──────────────────────────────────┤
  //│  -X        │  Nx>0            │                                  │
  //│  +X        │  Nx>0            │                                  │
  //│     -Y     │       Ny>0       │  MIN: 0                          │
  //│     +Y     │       Ny>0       │  MAX: number_neighbors-1         │
  //│        -Z  │            Nz>0  │                                  │
  //│        +Z  │            Nz>0  │                                  │
  //├────────────┼──────────────────┼──────────────────────────────────┤
  //│  -X -Y     │  Nx>0 Ny>0       │                                  │
  //│  +X -Y     │  Nx>0 Ny>0       │                                  │
  //│  -X +Y     │  Nx>0 Ny>0       │                                  │
  //│  +X +Y     │  Nx>0 Ny>0       │                                  │
  //│  -X    -Z  │  Nx>0      Nz>0  │                                  │
  //│  +X    -Z  │  Nx>0      Nz>0  │  MIN: number_neighbors           │
  //│  -X    +Z  │  Nx>0      Nz>0  │  MAX: number_plane_neighbors-1   │
  //│  +X    +Z  │  Nx>0      Nz>0  │                                  │
  //│     -Y -Z  │       Ny>0 Nz>0  │                                  │
  //│     +Y -Z  │       Ny>0 Nz>0  │                                  │
  //│     -Y +Z  │       Ny>0 Nz>0  │                                  │
  //│     +Y +Z  │       Ny>0 Nz>0  │                                  │
  //├────────────┼──────────────────┼──────────────────────────────────┤
  //│  -X -Y -Z  │  Nx>0 Ny>0 Nz>0  │                                  │
  //│  +X -Y -Z  │  Nx>0 Ny>0 Nz>0  │                                  │
  //│  -X +Y -Z  │  Nx>0 Ny>0 Nz>0  │                                  │
  //│  +X +Y -Z  │  Nx>0 Ny>0 Nz>0  │  MIN: number_plane_neighbors     │
  //│  -X -Y +Z  │  Nx>0 Ny>0 Nz>0  │  MAX: number_volume_neighbors-1  │
  //│  +X -Y +Z  │  Nx>0 Ny>0 Nz>0  │                                  │
  //│  -X +Y +Z  │  Nx>0 Ny>0 Nz>0  │                                  │
  //│  +X +Y +Z  │  Nx>0 Ny>0 Nz>0  │                                  │
  //└────────────┴──────────────────┴──────────────────────────────────┘
  // Get a pointer to a neighbor cell
  std::shared_ptr<Cell> getNeighborCell(const int dir) const;
  // Loop on all neighbor cells in a specific direction and apply a function
  void applyToDirNeighborCells(const unsigned dir, const std::function<void(const std::shared_ptr<Cell>&, const std::shared_ptr<Cell>&, const unsigned&)> &&f) const;
  //Apply extrapolation function to all non-leaf descendent cells recursively
  void extrapolateRecursively(ExtrapolationFunctionType extrapolation_function) const;
 private:
  // Verify if neighbors splitting is needed before cell splitting
  bool verifySplitNeighbors(const int max_level);
  // Split neighbors first if needed before cell splitting
  void checkSplitNeighbors(const int max_level, ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<Cell> &cell) {});
  // Verify if children coarsening is needed before cell coarsening
  bool verifyCoarsenChildren();
  // Verify if neighbors coarsening is needed before cell coarsening
  bool verifyCoarsenNeighbors();
 public:
 // Transform sibling number to (i,j,k) coordinates
  static std::tuple<unsigned, unsigned, unsigned> siblingNumberToCoords(const int sibling_number) { return ChildAndDirectionTablesType::siblingNumberToCoords(sibling_number); };
  // Transform (i,j,k) coordinates to sibling number
  static int coordsToSiblingNumber(const unsigned sibling_coord_1, const unsigned sibling_coord_2, const unsigned sibling_coord_3) { return ChildAndDirectionTablesType::coordsToSiblingNumber(sibling_coord_1, sibling_coord_2, sibling_coord_3); };
 private:
  // For a given sibling number, determine if the neighbor cell in a given direction:
  // - shares the same parent cell (true) or belongs to another cell (false)
  // - it's sibling number
  static std::pair<bool, unsigned> getDirectNeighborCellInfos(const int sibling_number, const int dir) { return ChildAndDirectionTablesType::directNeighborCellInfos(sibling_number, dir); };
  // convert two direct neighbor directions to a plane direction
  static int directToPlaneDir(const int dir1, const int dir2) { return ChildAndDirectionTablesType::directToPlaneDir(dir1, dir2); };
  // Get a pointer to a neighbor cell accessible by 2 consecutive othogonal direction (corners in 2D)
  std::shared_ptr<Cell> getPlaneNeighborCell(const int sibling_number, const int dir) const;
  // Get a pointer to a neighbor cell accessible by 3 consecutive othogonal direction (corners in 3D)
  std::shared_ptr<Cell> getVolumeNeighborCell(const int sibling_number, const int dir) const;
  // Flags propagation from parent to children
  void setIndicatorFromParent(const Cell &parent_cell);
};

#include "Cell.tpp"
