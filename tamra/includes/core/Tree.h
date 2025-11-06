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
#include "iterator/MortonIterator.h"
#include "manager/BalanceManager.h"
#include "manager/CoarseManager.h"
#include "manager/GhostManager.h"
#include "manager/MinLevelMeshManager.h"
#include "manager/RefineManager.h"
#include "RootCellEntry.h"

template<typename CellTypeT, typename TreeIteratorTypeT = MortonIterator<CellTypeT>>
class Tree {
 public:
  using CellType = CellTypeT;
  using BalanceManagerType = BalanceManager<CellType, TreeIteratorTypeT>;
  using CoarseManagerType = CoarseManager<CellType>;
  using ExtrapolationFunctionType = std::function<void(const std::shared_ptr<CellType>&)>;
  using InterpolationFunctionType = std::function<void(const std::shared_ptr<CellType>&)>;
  using GhostManagerType = GhostManager<CellType, TreeIteratorTypeT>;
  using GhostManagerTaskType = typename GhostManager<CellType, TreeIteratorTypeT>::GhostManagerTaskType;
  using MinLevelMeshManagerType = MinLevelMeshManager<CellType, TreeIteratorTypeT>;
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
  std::vector<std::shared_ptr<CellType>> root_cells;
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
  void createRootCells(const std::vector<RootCellEntryType> &root_cell_entries);

  //***********************************************************//
  //  ACCESSORS                                                //
  //***********************************************************//
 public:
  // Get root cells
  const std::vector<std::shared_ptr<CellType>>& getRootCells() const;
  // Get min mesh level
  int getMinLevel() const;
  // Get max mesh level
  int getMaxLevel() const;
  // Get ghost manager
  GhostManagerType getGhostManager() const;
  // Default directions
  static const std::vector<int>& defaultDirections() {
    static const std::vector<int> dirs = [] {
        std::vector<int> v(CellType::number_neighbors); // 4 connexity
        std::iota(v.begin(), v.end(), 0);
        return v;
    }();
    return dirs;
  }

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Recusively mesh the tree to ensure every cell is at least at min level
  void meshAtMinLevel();
  void meshAtMinLevel(TreeIteratorType &iterator);

  // Split all the leaf cells belonging to this proc that need to be refined and are not at max level
  bool refine(ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<CellType> &cell) {});

  // Creation of ghost cells
  GhostManagerTaskType buildGhostLayer(InterpolationFunctionType interpolation_function = [](const std::shared_ptr<CellType> &cell) {}, const std::vector<int> &directions = defaultDirections());
  // Exchange ghost cell values
  void exchangeGhostValues(GhostManagerTaskType &task, InterpolationFunctionType interpolation_function = [](const std::shared_ptr<CellType> &cell) {});

  // Redistribute cells among processes to balance computation load
  void loadBalance(InterpolationFunctionType interpolation_function = [](const std::shared_ptr<CellType> &cell) {}, const double max_pct_unbalance = 0.1);

  //--- Propagating -------------------------------------------//
  void propagate() {};

  //--- Coarsening --------------------------------------------//
  bool coarsen(InterpolationFunctionType interpolation_function = [](const std::shared_ptr<CellType> &cell) {});

  //--- Computing SFC indices ---------------------------------//
  void boundaryConditions() {};

  // Count the number of owned leaf cells
  unsigned countOwnedLeaves() const;

  // Apply a function to owned leaf cells
  void applyToOwnedLeaves(const std::function<void(const std::shared_ptr<CellType>&, unsigned)> &f) const;
};

#include "Tree.tpp"
