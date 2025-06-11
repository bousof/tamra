/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class for storing cell data and methods during simulations.
 */

#pragma once


#include "../parallel/ParallelData.h"

template<typename CellType>
class CellData: public ParallelData {
	//***********************************************************//
  //  VARIABLES                                                //
  //***********************************************************//
 private:
  double value;

  //***********************************************************//
	//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
	//***********************************************************//
 public:
  CellData();
  ~CellData() = default;

  //***********************************************************//
  //  ACCESSORS                                                //
  //***********************************************************//
 public:
  // Get the computation load of the cell
  double getLoad(const std::shared_ptr<CellType> &cell) const;
  // Get the value of the cell
  double getValue() const { return value; };

  //***********************************************************//
  //  MUTATORS                                                 //
  //***********************************************************//
 public:
  // Set the value of the cell
  void setValue(const double value) { this->value = value; };

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Init cell data as a vector of double
  void fromVectorOfData(const std::vector<double> &buffer) override;
  // Return cell data as a vector of double
  std::vector<double> toVectorOfData() const override;
  // Return cell data size
  unsigned getDataSize() const override;
};

#include"CellData.tpp"
