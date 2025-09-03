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
#include "../TreeIterator.h"

template<typename CellType, typename TreeIteratorType = TreeIterator<CellType>>
class MinLevelMeshManager {
  //***********************************************************//
  //  VARIABLES                                                //
  //***********************************************************//
  // Minimum mesh level
  const int min_level;
  // Maximum mesh level
  const int max_level;
  // Process rank
  const int rank;
  // Number of process
  const int size;

  //***********************************************************//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //***********************************************************//
 public :
  // Constructor
  MinLevelMeshManager(int min_level, int max_level, int rank, int size);
  // Destructor
  ~MinLevelMeshManager();

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Serial meshing at min level
  void meshAtMinLevel(const std::vector< std::shared_ptr<CellType> >& root_cells) const;
  // Parallel meshing at min level
  void meshAtMinLevel(const std::vector< std::shared_ptr<CellType> >& root_cells, TreeIteratorType &iterator) const;

 private:
  // Recursively mesh cells at min level
  void serialMeshAtMinLevelRecurs(const std::shared_ptr<CellType>& cell) const;
  // Mesh all cells in the process partition at min level
  void parallelMeshAtMinLevel(const std::vector< std::shared_ptr<CellType> >& root_cells, TreeIteratorType &iterator) const;
  // Set a parent to belong to this proc if any of its child do
  bool backPropagateToThisProc(const std::shared_ptr<CellType>& cell) const;
};

#include "./MinLevelMeshManager.tpp"
