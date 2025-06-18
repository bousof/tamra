/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Some functions for basic array manipulations.
 */

#pragma once

#include <vector>

template <typename T>
T cumulative_sum(const std::vector<T>& counts, std::vector<T>& displacements, bool startAtZero = false) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, unsigned>::value || std::is_same<T, int>::value || std::is_same<T, double>::value,
    "cumulative_sum only supports T = bool, double, int, or unsigned"
  );

  displacements.resize(counts.size());
  int sum = 0;
  for (unsigned i = 0; i < counts.size(); ++i) {
    if (startAtZero)
      displacements[i] = sum;
    sum += counts[i];
    if (!startAtZero)
      displacements[i] = sum;
  }
  return sum;
}

std::vector<double> linspace(const double min, const double max, const unsigned count);

template <typename T>
std::vector<T> concatenate(const std::vector<T> &v1, const std::vector<T> &v2) {
  static_assert(
    std::is_same<T, bool>::value || std::is_same<T, unsigned>::value || std::is_same<T, int>::value || std::is_same<T, double>::value,
    "concatenate only supports T = bool, double, int, or unsigned"
  );

  std::vector<T> result;
  result.reserve(v1.size() + v2.size());
  result.insert(result.end(), v1.begin(), v1.end());
  result.insert(result.end(), v2.begin(), v2.end());
  return result;
}

// Split a vector to a vector of vectors
template <typename T>
void split(const std::vector<T> &buffer, std::vector< std::vector<T> > &buffers, const std::vector<int> &displacements) {
  buffers.resize(displacements.size());
  for (int p{0},i; p<displacements.size(); ++p) {
    int end_index = p<(displacements.size()-1) ? displacements[p+1] : buffer.size();
    buffers[p].resize(end_index-displacements[p]);
    for (i=0; i<buffers[p].size(); ++i)
      buffers[p][i] = std::move(buffer[i+displacements[p]]);
  }
}
