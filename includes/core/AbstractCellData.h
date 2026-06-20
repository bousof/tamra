/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Abstract class for storing cell data and methods during simulations.
 */

#pragma once

#include <istream>
#include <ostream>

#include "../parallel/ParallelData.h"

class AbstractCellData : public ParallelData {
  //***********************************************************//
	//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
	//***********************************************************//
 public:
  AbstractCellData() {};
  ~AbstractCellData() = default;

  //***********************************************************//
  //  ACCESSORS                                                //
  //***********************************************************//
 public:
  // Get the computation load of the cell
  virtual double getLoad(bool isLeaf, const std::shared_ptr<void> cell=nullptr) const = 0;

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Set values from vector of double after data communication or backup
  virtual void fromVectorOfData(const std::vector<double> &buffer) = 0;
  // Convert cell data to vector of double for data communication or backup
  virtual std::vector<double> toVectorOfData() const = 0;
  // Return cell data size
  virtual unsigned getDataSize() const = 0;
  // Dump the cell data to an output stream
  virtual void dump(std::ostream& os, const bool binary=false) const = 0;
  // Restore the cell data from an input stream
  virtual void restore(std::istream& is, const bool binary=false) = 0;
};
