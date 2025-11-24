/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class for storing cell data (for fast entrance in the code but users are supposed to make their own).
 */

#pragma once

#include "AbstractCellData.h"

class CellData : public AbstractCellData {
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
  double getLoad(bool isLeaf, const std::shared_ptr<void> =nullptr) const override;
  // Get the value of the cell
  double getValue() const;

  //***********************************************************//
  //  MUTATORS                                                 //
  //***********************************************************//
 public:
  // Set the value of the cell
  void setValue(const double value);

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
