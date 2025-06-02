/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class for cell manipulations.
 */

#pragma once

#include <array>
#include <memory>
#include <tuple>

template<int Nx, int Ny, int Nz> class Cell;

#include "CellData.h"
#include "Oct.h"

template<int Nx = 2, int Ny = 1, int Nz = 1>
class Cell {
 public:
  using OctType = Oct<Cell<Nx, Ny, Nz>>;
  static constexpr int number_dimensions = (Nx>1) + (Ny>1) + (Nz>1);
  static constexpr int number_children = Nx * Ny * Nz;
  static constexpr int number_neighbors = 2 * number_dimensions;
  static constexpr int number_plane_neighbors = number_dimensions==3 ? 18 : number_dimensions==2 ? 8 : 2;
  static constexpr int number_volume_neighbors = number_dimensions==3 ? 26 : number_dimensions==2 ? 8 : 2;
  static constexpr int N1 = Nx>1 ? Nx : Ny>1 ? Ny : Nz;
  static constexpr int N2 = (Nx>1 && Ny>1) ? Ny : number_dimensions>1 ? Nz : 1;
  static constexpr int N3 = number_dimensions==3 ? Nz : 1;
  static constexpr int N12 = N1 * N2;
  static constexpr int N13 = N1 * N3;
  static constexpr int N23 = N2 * N3;

  //***********************************************************//
  //  VARIABLES                                                //
  //***********************************************************//
 private:
  std::unique_ptr<CellData> data;
  std::shared_ptr<OctType> parent_oct;
  std::shared_ptr<OctType> child_oct;
  // Cell flags indicator
	// - 0 : BELONG TO THIS  PROC  &  DOESN'T NEED TO BE CHANGED
	// - 1 : BELONG TO THIS  PROC  &  NEED TO BE COARSENED
	// - 2 : BELONG TO THIS  PROC  &  NEED TO BE REFINED
	// - 3 : BELONG TO OTHER PROC  &  DOESN'T NEED TO BE CHANGED
	// - 4 : BELONG TO OTHER PROC  &  NEED TO BE COARSENED
	// - 5 : BELONG TO OTHER PROC  &  NEED TO BE REFINED
	// - 6 : IS BOUNDARY CELL      &  DOESN'T NEED TO BE CHANGED
	// - 7 : IS BOUNDARY CELL      &  NEED TO BE COARSENED
	// - 8 : IS BOUNDARY CELL      &  NEED TO BE REFINED
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
  std::shared_ptr<Cell> getChildCell(const unsigned neighbor_sibling_number) const;
  // Get child cells
  const std::array< std::shared_ptr<Cell>, number_children >& getChildCells() const;
  // Get level of the cell
  int getLevel() const;
  // Get parent oct
  std::shared_ptr<OctType> getParentOct() const;
  // Get the sibling number (position of the cell in the parent oct child_cells array)
  unsigned getSiblingNumber() const;
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
 // Flags mutators
  void setToThisProc()  { indicator = indicator%3; }
  void setToOtherProc() { indicator = 3 + indicator%3; }
  void setToUnchange()  { indicator = indicator - (indicator%3); }
  void setToRefine()    { indicator = indicator + 1 - (indicator%3); }
  void setToCoarse()    { indicator = indicator + 2 - (indicator%3); }
  void setToThisProcRecurs() {
    setToThisProc();
    if (!isLeaf())
      for (const auto &child: getChildCells())
        child->setToThisProcRecurs();
  }
  void setToOtherProcRecurs() {
    setToOtherProc();
    if (!isLeaf())
      for (const auto &child: getChildCells())
        child->setToOtherProcRecurs();
  }
  void setToUnchangeRecurs() {
    setToUnchange();
    if (!isLeaf())
      for (const auto &child: getChildCells())
        child->setToUnchangeRecurs();
  }
  void setToRefineRecurs() {
    setToRefine();
    if (!isLeaf())
      for (const auto &child: getChildCells())
        child->setToRefineRecurs();
  }
  void setToCoarseRecurs() {
    setToCoarse();
    if (!isLeaf())
      for (const auto &child: getChildCells())
        child->setToCoarseRecurs();
  }
 private:
  bool setToBoundary()     { return indicator = 6+indicator%3; }

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // True if leaf cell (no child oct)
  bool isLeaf() const;
  // True if root cell (no parent oct)
  bool isRoot() const;
  // Split a root cell (a pointer to the root is needed for back reference in child oct)
  const std::array< std::shared_ptr<Cell>, number_children >& splitRoot(const int max_level, std::shared_ptr<Cell> root_cell);
  // Split a cell and it's direct neighbors if needed for mesh conformity
  const std::array< std::shared_ptr<Cell>, number_children >& split(const int max_level);
  // Coarsen a cell if neighbors cell allow to preserve consistency else nothing is done
  bool coarsen(const int min_level);
  //_________________________________________________________________
  //  Priority  |   Available if   |   Indexes (dir)
  //____________|__________________|_________________________________
  //  -X        |  Nx>1            |
  //  +X        |  Nx>1            |
  //     -Y     |       Ny>1       |  MIN: 0
  //     +Y     |       Ny>1       |  MAX: number_neighbors-1
  //        -Z  |            Nz>1  |
  //        +Z  |            Nz>1  |_________________________________
  //  -X -Y     |  Nx>1 Ny>1       |
  //  +X -Y     |  Nx>1 Ny>1       |
  //  -X +Y     |  Nx>1 Ny>1       |
  //  +X +Y     |  Nx>1 Ny>1       |
  //  -X    -Z  |  Nx>1      Nz>1  |
  //  +X    -Z  |  Nx>1      Nz>1  |  MIN: number_neighbors
  //  -X    +Z  |  Nx>1      Nz>1  |  MAX: number_plane_neighbors-1
  //  +X    +Z  |  Nx>1      Nz>1  |
  //     -Y -Z  |       Ny>1 Nz>1  |
  //     +Y -Z  |       Ny>1 Nz>1  |
  //     -Y +Z  |       Ny>1 Nz>1  |
  //     +Y +Z  |       Ny>1 Nz>1  |_________________________________
  //  -X -Y -Z  |  Nx>1 Ny>1 Nz>1  |
  //  +X -Y -Z  |  Nx>1 Ny>1 Nz>1  |
  //  -X +Y -Z  |  Nx>1 Ny>1 Nz>1  |
  //  +X +Y -Z  |  Nx>1 Ny>1 Nz>1  |  MIN: number_plane_neighbors
  //  -X -Y +Z  |  Nx>1 Ny>1 Nz>1  |  MAX: number_volume_neighbors-1
  //  +X -Y +Z  |  Nx>1 Ny>1 Nz>1  |
  //  -X +Y +Z  |  Nx>1 Ny>1 Nz>1  |
  //  +X +Y +Z  |  Nx>1 Ny>1 Nz>1  |
  //____________|__________________|_________________________________
  // Get a pointer to a neighbor cell
  std::shared_ptr<Cell> getNeighborCell(const int dir) const;
 private:
  // Verify if neighbors splitting is needed before cell splitting
  bool verifySplitNeighbors(const int max_level);
  // Split neighbors first if needed before cell splitting
  void checkSplitNeighbors(const int max_level);
  // Verify if children coarsening is needed before cell coarsening
  bool verifyCoarsenChildren();
  // Verify if neighbors coarsening is needed before cell coarsening
  bool verifyCoarsenNeighbors();
  // Transform sibling number to (i,j,k) coordinates
  inline std::tuple<unsigned, unsigned, unsigned> siblingNumberToCoords(const int sibling_number) const;
  // Transform (i,j,k) coordinates to sibling number
  inline int coordsToSiblingNumber(const unsigned sibling_coord_1, const unsigned sibling_coord_2, const unsigned sibling_coord_3) const;
  // For a given sibling number, determine if the neighbor cell in a given direction:
  // - shares the same parent cell (true) or belongs to another cell (false)
  // - it's sibling number
  std::pair<bool, unsigned> getDirectNeighborCellInfos(const int sibling_number, const int dir) const;
  // Get a pointer to a neighbor cell accessible by 2 consecutive othogonal direction (corners in 2D)
  std::shared_ptr<Cell> getPlaneNeighborCell(const int sibling_number, const int dir1, const int dir2) const;
  // Get a pointer to a neighbor cell accessible by 3 consecutive othogonal direction (corners in 3D)
  std::shared_ptr<Cell> getVolumeNeighborCell(const int sibling_number, const int dir1, const int dir2, const int dir3) const;
  // Flags propagation from parent to children
  void setIndicatorFromParent(const Cell& parent_cell);
};

#include "Cell.tpp"
