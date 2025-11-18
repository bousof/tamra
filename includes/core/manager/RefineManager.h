/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class that handles tree refinement.
 */

#pragma once

#include <cmath>
#include <memory>

template<typename CellType>
class RefineManager {
  using ExtrapolationFunctionType = std::function<void(const std::shared_ptr<CellType>&)>;

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
  RefineManager(int min_level, int max_level, int rank, int size);
  // Destructor
  ~RefineManager();

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Go through all the leaf cells and split them one time if needed.
	bool refine(const std::vector<std::shared_ptr<CellType>> &root_cells, ExtrapolationFunctionType extrapolation_function = [](const std::shared_ptr<CellType> &cell) {}) const;

 private:
  // Recursively refine child cells if needed
  bool refineRecurs(const std::shared_ptr<CellType> &cell, ExtrapolationFunctionType extrapolation_function) const;
};

#include "./RefineManager.tpp"
