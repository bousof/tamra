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


template<typename CellType>
class CoarseningManager {
  using InterpolationFunctionType = std::function<void(const std::shared_ptr<CellType>&)>;

  //-----------------------------------------------------------//
  //  VARIABLES                                                //
  //-----------------------------------------------------------//
  // Minimum mesh level
  const unsigned min_level;
  // Maximum mesh level
  const unsigned max_level;
  // Process rank
  const unsigned rank;
  // Number of process
  const unsigned size;

  //-----------------------------------------------------------//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //-----------------------------------------------------------//
 public :
  // Constructor
  CoarseningManager(const unsigned min_level, const unsigned max_level, const unsigned rank, const unsigned size);
  // Destructor
  ~CoarseningManager();

  //-----------------------------------------------------------//
  //  METHODS                                                  //
  //-----------------------------------------------------------//
 public:
  // Go through all the parent cells and coarsen them one time if needed
	bool coarsen(const std::vector<std::shared_ptr<CellType>> &root_cells, InterpolationFunctionType interpolation_function = [](const std::shared_ptr<CellType> &cell) { (void)cell; }) const;

 private:
  // Recursively coarsen cells at a specific level
  bool coarsenToLevelRecurs(const std::shared_ptr<CellType> &cell, const unsigned coarse_level, InterpolationFunctionType interpolation_function) const;
};

#include "CoarseningManager.tpp"
