/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Defines the interface of a class to be able to send it easily through MPI.
 */

#pragma once

#include <vector>

struct ParallelData {
  //***********************************************************//
	//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
	//***********************************************************//
 public:
  virtual ~ParallelData() = default;

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Init cell data as a vector of double
  virtual void fromVectorOfData(const std::vector<double> &buffer) = 0;
  // Return cell data as a vector of double
  virtual std::vector<double> toVectorOfData() const = 0;
  // Return cell data size
  virtual unsigned getDataSize() const = 0;
};
