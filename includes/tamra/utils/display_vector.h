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
std::ostream& display_vector(std::ostream &os, const std::vector<T> &c) {
  //static_assert(
  //  std::is_same<T, bool>::value || std::is_same<T, double>::value || std::is_same<T, int>::value || std::is_same<T, unsigned>::value,
  //  "display_vector only supports T = bool, double, int, or unsigned"
  //);

  os << "[ ";
  for (const auto &x : c)
    os << x << ", ";
  os << "]";
  return os;
}

// Display a vector of vectores
template<typename T>
std::ostream& display_vector(std::ostream &os, const std::vector<std::vector<T>> &c, const bool line_breaks = false) {
  os << (line_breaks ? "[\n" : "[ ");
  for (const auto &x : c) {
    os << (line_breaks ? "  " : "");
    display_vector(os, x) << (line_breaks ? ",\n" : ", ");
  }
  os << "]";
  return os;
}

// Display a vector of vectores
template<typename T>
std::ostream& display_vector(std::ostream &os, const std::vector<std::vector<std::vector<T>>> &c, const bool row_breaks = true, const bool col_breaks = false) {
  os << (row_breaks ? "[\n" : "[ ");
  for (const auto &x : c) {
    os << (row_breaks ? "  " : "");
    display_vector(os, x, col_breaks) << (row_breaks ? ",\n" : ", ");
  }
  os << "]";
  return os;
}
