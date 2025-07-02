/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class that handles ghost cells.
 */

#pragma once

#include <memory>
#include <vector>

#include "../../parallel/allgather.h"
#include "../../parallel/alltoallv.h"
#include "../TreeIterator.h"

template<typename CellTypeT, typename TreeIteratorType> class GhostManager;

#include "./GhostManagerTask.h"

template<typename CellTypeT, typename TreeIteratorTypeT = TreeIterator<CellTypeT>>
class GhostManager {
 public:
  using CellType = CellTypeT;
  using TreeIteratorType = TreeIteratorTypeT;
  using GhostManagerTaskType = GhostManagerTask<GhostManager<CellTypeT, TreeIteratorType>>;

  //***********************************************************//
  //  VARIABLES                                                //
  //***********************************************************//
 private:
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
  GhostManager(int min_level, int max_level, int rank, int size);
  // Destructor
  ~GhostManager();

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Creation of ghost cells and exchange of ghost values
	GhostManagerTaskType buildGhostLayer(std::vector< std::shared_ptr<CellType> >& root_cells, TreeIteratorType &iterator) const;
  // Update ghost cells and exchange values for solving conflicts
	void updateGhostLayer(GhostManagerTaskType &task, TreeIteratorType &iterator) const;
 private:
  // Share the partitions start and end cells
  void sharePartitions(std::vector< std::vector<unsigned> > &begin_ids, std::vector< std::vector<unsigned> > &end_ids, TreeIteratorType &iterator) const;
  // Loop on owned cells and check if neighbors belong to another process
  void findCellsToSend(const std::vector< std::shared_ptr<CellType> > &root_cells, const std::vector< std::vector<unsigned> > &begin_ids, const std::vector< std::vector<unsigned> > &end_ids, std::vector< std::vector<std::shared_ptr<CellType>> > &cells_to_send, TreeIteratorType &iterator) const;
  // Set all ghost cells to coarse
  void setGhostToCoarseRecurs(const std::shared_ptr<CellType> &cell) const;
};

#include "./GhostManager.tpp"
