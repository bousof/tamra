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
  virtual void fromVectorOfData(const std::vector<double> &buffer) = 0;
  virtual std::vector<double> toVectorOfData() const = 0;
  virtual unsigned getDataSize() const = 0;
  virtual ~ParallelData() = default;
};
