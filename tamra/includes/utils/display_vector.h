/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Facilitate the display of containers.
 */

#pragma once

#include <ostream>

// Display a vector
template<typename T>
std::ostream& displayVector(std::ostream &os, const std::vector<T> &c) {
  //static_assert(
  //  std::is_same<T, bool>::value || std::is_same<T, double>::value || std::is_same<T, int>::value || std::is_same<T, unsigned>::value,
  //  "vectorAllToAll only supports T = bool, double, int, or unsigned"
  //);

  os << "[ ";
  for (const auto &x : c)
    os << x << ", ";
  os << "]";
  return os;
}

// Display a vector of vectores
template<typename T>
std::ostream& displayVector(std::ostream &os, const std::vector<std::vector<T>> &c, const bool lineBreaks = false) {
  os << (lineBreaks ? "[\n" : "[ ");
  for (const auto &x : c) {
    os << (lineBreaks ? "  " : "");
    displayVector(os, x) << (lineBreaks ? ",\n" : ", ");
  }
  os << "]";
  return os;
}

// Display a vector of vectores
template<typename T>
std::ostream& displayVector(std::ostream &os, const std::vector<std::vector<std::vector<T>>> &c, const bool rowBreaks = true, const bool colBreaks = false) {
  os << (rowBreaks ? "[\n" : "[ ");
  for (const auto &x : c) {
    os << (rowBreaks ? "  " : "");
    displayVector(os, x, colBreaks) << (rowBreaks ? ",\n" : ", ");
  }
  os << "]";
  return os;
}
