/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class that handles tree meshing at min level.
 */

#pragma once

#include <cmath>
#include <memory>

#include "../../parallel/bcast.h"

template<typename CellType, typename TreeIteratorType>
class MinLevelMeshManager {
  //***********************************************************//
  //  VARIABLES                                                //
  //***********************************************************//
  // Minimum mesh level
  const unsigned min_level;
  // Maximum mesh level
  const unsigned max_level;
  // Process rank
  const unsigned rank;
  // Number of process
  const unsigned size;

  //***********************************************************//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //***********************************************************//
 public :
  // Constructor
  MinLevelMeshManager(const unsigned min_level, const unsigned max_level, const unsigned rank, const unsigned size);
  // Destructor
  ~MinLevelMeshManager();

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Serial meshing at min level
  void meshAtMinLevel(const std::vector<std::shared_ptr<CellType>> &root_cells) const;
  // Parallel meshing at min level
  void meshAtMinLevel(const std::vector<std::shared_ptr<CellType>> &root_cells, TreeIteratorType &iterator) const;

 private:
  // Recursively mesh cells at min level
  void serialMeshAtMinLevelRecurs(const std::shared_ptr<CellType> &cell) const;
  // Mesh all cells in the process partition at min level
  void parallelMeshAtMinLevel(const std::vector<std::shared_ptr<CellType>> &root_cells, TreeIteratorType &iterator) const;
  // Set a parent to belong to this proc if any of its child do
  bool backPropagateToThisProc(const std::shared_ptr<CellType> &cell) const;
};

#include "MinLevelMeshManager.tpp"
