/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class that handles tree coarsening.
 */

#pragma once

#include <memory>
#include <vector>

#include "../TreeIterator.h"

template<typename CellType>
class CoarseManager {
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
  CoarseManager(int min_level, int max_level, int rank, int size);
  // Destructor
  ~CoarseManager();

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Go through all the parent cells and coarse them one time if needed
	void coarsen(const std::vector< std::shared_ptr<CellType> >& root_cells) const;

 private:
  // Recursively coarse cells at a specific level
  void coarsenToLevelRecurs(const std::shared_ptr<CellType>& cell, const int &coarse_level) const;
};

#include "./CoarseManager.tpp"
