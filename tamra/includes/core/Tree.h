/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class for tree manipulations.
 */

#pragma once

#include <memory>
#include <vector>

#include "Cell.h"
#include "manager/BalanceManager.h"
#include "manager/CoarseManager.h"
#include "manager/GhostManager.h"
#include "manager/MinLevelMeshManager.h"
#include "manager/RefineManager.h"
#include "RootCellEntry.h"
#include "TreeIterator.h"

template<typename CellTypeT, typename TreeIteratorTypeT = TreeIterator<CellTypeT>>
class Tree {
 public:
  using CellType = CellTypeT;
  using BalanceManagerType = BalanceManager<CellType>;
  using CoarseManagerType = CoarseManager<CellType>;
  using ExtrapolationFunctionType = std::function<void(const std::shared_ptr<CellType>&)>;
  using InterpolationFunctionType = std::function<void(const std::shared_ptr<CellType>&)>;
  using GhostManagerType = GhostManager<CellType>;
  using GhostManagerTaskType = typename GhostManager<CellType>::GhostManagerTaskType;
  using MinLevelMeshManagerType = MinLevelMeshManager<CellType>;
  using RefineManagerType = RefineManager<CellType>;
  using RootCellEntryType = RootCellEntry<CellType>;
  using TreeIteratorType = TreeIteratorTypeT;
  //***********************************************************//
  //  DATA                                                     //
  //***********************************************************//
 private:
  // Minimum mesh level
  int min_level;
  // Maximum mesh level
  int max_level;
  // Process rank
  const int rank;
  // Number of process
  const int size;
  // Root cells
  std::vector< std::shared_ptr<CellType> > root_cells;
  // Load balancing manager
	BalanceManagerType balanceManager;
  // Mesh coarsening manager
	CoarseManagerType coarseManager;
  // Ghost cell manager
	GhostManagerType ghostManager;
  // Min level meshing manager
	MinLevelMeshManagerType minLevelMeshManager;
  // Mesh refinement manager
	RefineManagerType refineManager;

  //***********************************************************//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //***********************************************************//
 public:
  // Constructor
  Tree(const int min_level = 1, const int max_level = 2, const int rank = 0, const int size = 1);
  // Destructor
  ~Tree();
  // Create root cell
  void createRootCells(const std::vector< RootCellEntryType > &root_cell_entries);
 private:
  // Create root cell
  std::vector< std::shared_ptr<CellType> > createBoundaryCells();

  //***********************************************************//
  //  ACCESSORS                                                //
  //***********************************************************//
 public:
  // Get root cells
  const std::vector< std::shared_ptr<CellType> >& getRootCells() const;
  // Get min mesh level
  int getMinLevel() const;
  // Get max mesh level
  int getMaxLevel() const;

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Recusively mesh the tree to ensure every cell is at least at min level
  void meshAtMinLevel();
  void meshAtMinLevel(TreeIteratorType& iterator);

  // Split all the leaf cells belonging to this proc that need to be refined and are not at max level
  void refine(ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<CellType>& cell) {});

  // Creation of ghost cells
  GhostManagerTaskType buildGhostLayer(TreeIteratorType &iterator);
  void exchangeGhostValues() {};

  // Redistribute cells among processes to balance computation load
  void loadBalance();
  void loadBalance(TreeIteratorType &iterator);

  //--- Propagating -------------------------------------------//
  void propagate() {};

  //--- Coarsening --------------------------------------------//
  void coarsen(InterpolationFunctionType interpolation_function = [](const std::shared_ptr<CellType>& cell) {});

  //--- Computing SFC indices ---------------------------------//
  void boundaryConditions() {};

  // Count the number of owned leaf cells
  unsigned countOwnedLeaves() const;
};

#include "Tree.tpp"
