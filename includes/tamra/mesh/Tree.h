/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class for tree manipulations.
 */

#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <numeric>
#include <vector>

#include <tamra/mesh/adaptation/CoarseningManager.h>
#include <tamra/mesh/adaptation/MinLevelMeshManager.h>
#include <tamra/mesh/adaptation/RefinementManager.h>
#include <tamra/mesh/Cell.h>
#include <tamra/mesh/distributed/LoadBalanceManager.h>
#include <tamra/mesh/distributed/GhostManager.h>
#include <tamra/mesh/RootCellEntry.h>
#include <tamra/mesh/traversal/MortonIterator.h>


template<typename CellTypeT, typename TreeIteratorTypeT = MortonIterator<CellTypeT>>
class Tree {
 public:
  using CellType = CellTypeT;
  using LoadBalanceManagerType = LoadBalanceManager<CellType, TreeIteratorTypeT>;
  using CoarseningManagerType = CoarseningManager<CellType>;
  using ExtrapolationFunctionType = std::function<void(const std::shared_ptr<CellType>&)>;
  using InterpolationFunctionType = std::function<void(const std::shared_ptr<CellType>&)>;
  using MergeFunctionType = std::function<void(const std::shared_ptr<CellType>&, const std::shared_ptr<CellType>&)>;
  using GhostManagerType = GhostManager<CellType, TreeIteratorTypeT>;
  using GhostManagerTaskType = typename GhostManager<CellType, TreeIteratorTypeT>::GhostManagerTaskType;
  using MinLevelMeshManagerType = MinLevelMeshManager<CellType, TreeIteratorTypeT>;
  using RefinementManagerType = RefinementManager<CellType>;
  using RootCellEntryType = RootCellEntry<CellType>;
  using TreeIteratorType = TreeIteratorTypeT;

  //-----------------------------------------------------------//
  //  DATA                                                     //
  //-----------------------------------------------------------//
 private:
  // Minimum mesh level
  unsigned min_level;
  // Maximum mesh level
  unsigned max_level;
  // Process rank
  const unsigned rank;
  // Number of process
  const unsigned size;
  // Root cells
  std::vector<std::shared_ptr<CellType>> root_cells;
  // Load balancing manager
	LoadBalanceManagerType load_balance_manager;
  // Mesh coarsening manager
	CoarseningManagerType coarsening_manager;
  // Ghost cell manager
	GhostManagerType ghost_manager;
  // Min level meshing manager
	MinLevelMeshManagerType min_level_mesh_manager;
  // Mesh refinement manager
	RefinementManagerType refinement_manager;

  //-----------------------------------------------------------//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //-----------------------------------------------------------//
 public:
  // Constructor
  Tree(const unsigned min_level = 1, const unsigned max_level = 2, const unsigned rank = 0, const unsigned size = 1);
  // Destructor
  ~Tree();
  // Create root cell
  void createRootCells(const std::vector<RootCellEntryType> &root_cell_entries);

  //-----------------------------------------------------------//
  //  ACCESSORS                                                //
  //-----------------------------------------------------------//
 public:
  // Get root cells
  const std::vector<std::shared_ptr<CellType>>& getRootCells() const;
  // Get min mesh level
  unsigned getMinLevel() const;
  // Get max mesh level
  unsigned getMaxLevel() const;
  // Get ghost manager
  GhostManagerType getGhostManager() const;
  // Default directions
  static const std::vector<int>& default_directions() {
    static const std::vector<int> dirs = [] {
        std::vector<int> v(CellType::neighbor_count); // 4 connexity
        std::iota(v.begin(), v.end(), 0);
        return v;
    }();
    return dirs;
  }
  // Check if the tree is empty
  bool isEmpty() const;

  //-----------------------------------------------------------//
  //  METHODS                                                  //
  //-----------------------------------------------------------//
 public:
  // Recusively mesh the tree to ensure every cell is at least at min level
  void meshAtMinLevel();
  void meshAtMinLevel(TreeIteratorType &iterator);

  // Split all the leaf cells belonging to this proc that need to be refined and are not at max level
  bool refine(
    ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<CellType> &cell) { (void)cell; }
  );

  // Creation of ghost cells
  GhostManagerTaskType buildGhostLayer(
    ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<CellType> &cell) { (void)cell; },
    const std::vector<int> &directions = default_directions()
  );
  // Exchange ghost cell values
  void exchangeGhostValues(
    GhostManagerTaskType &task,
    ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<CellType> &cell) { (void)cell; }
  );

  // Redistribute cells among processes to balance computation load
  void loadBalance(
    ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<CellType> &cell) { (void)cell; },
    const double max_relative_load_imbalance = 0.1
  );

  //--- Propagating -------------------------------------------//
  void propagate() {}

  //--- Coarsening --------------------------------------------//
  bool coarsen(
    InterpolationFunctionType interpolation_function = [](const std::shared_ptr<CellType> &cell) { (void)cell; }
  );

  //--- Computing SFC indices ---------------------------------//
  void boundaryConditions() {}

  // Count the number of owned leaf cells
  unsigned countOwnedLeaves() const;

  // Count the number of cells
  unsigned countCells(const std::shared_ptr<CellType> &cell = nullptr) const;

  // Count the number of ghost leaf cells
  unsigned countGhostLeaves() const;

  // Apply a function to owned leaf cells
  void applyToOwnedLeaves(const std::function<void(const std::shared_ptr<CellType>&, const unsigned)> &f) const;

  // Apply a function to all cells
  void applyToAllCells(const std::function<void(const std::shared_ptr<CellType>&, const unsigned)> &f) const;

  // Share the partitions start and end cells
 public:
  void sharePartitions(std::vector<std::vector<unsigned>> &begin_ids, std::vector<std::vector<unsigned>> &end_ids) const;
 private:
  void sharePartitions(std::vector<std::vector<unsigned>> &begin_ids, std::vector<std::vector<unsigned>> &end_ids, TreeIteratorType &iterator) const;

  // Apply a function to ghost leaf cells
 public:
  void applyToGhostLeavesRanks(const std::function<void(const std::shared_ptr<CellType>&, const unsigned, const unsigned)> &f) const;
 private:
  void applyToGhostLeavesRanks(const std::function<void(const std::shared_ptr<CellType>&, const unsigned, const unsigned)> &f, TreeIteratorType &iterator) const;
  void applyToGhostLeaves(const std::function<void(const std::shared_ptr<CellType>&, const unsigned, const unsigned)> &f, std::vector<std::vector<unsigned>> &begin_ids, std::vector<std::vector<unsigned>> &end_ids, unsigned &index, TreeIteratorType &iterator) const;
  void applyToAllCellsRecurs(const std::shared_ptr<CellType> &cell, const std::function<void(const std::shared_ptr<CellType>&, const unsigned)> &f, unsigned &index) const;

  // Merge the tree with another one with partition following the same SFC
 public:
  void mergeWith(
    const Tree &other_tree, ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<CellType> &cell) { (void)cell; },
    MergeFunctionType merge_function = [](const std::shared_ptr<CellType> &cell, const std::shared_ptr<CellType> &other_cell) {
      if (!cell->isOwned() && other_cell->isOwned()) {
        cell->getCellData().fromVectorOfData(other_cell->getCellData().toVectorOfData());
      }
    }
  );
  void mergeCellRecurs(const std::shared_ptr<CellType> &cell, const std::shared_ptr<CellType> &other_cell, ExtrapolationFunctionType extrapolation_function, MergeFunctionType merge_function);
};

#include "Tree.tpp"
