/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Some functions for basic inout.
 */

#pragma once

#include <algorithm>
#include <array>
#include <istream>
#include <stdexcept>
#include <string>


//-----------------------------------------------------------//
//  PROTOTYPES                                               //
//-----------------------------------------------------------//

inline void expect(std::istream& is, const std::string& expected, const std::string& error_message = "");

template<typename T>
inline T get(std::istream& is);

inline std::array<char, 4> get_tree_iterator_tag(std::istream& is);


//-----------------------------------------------------------//
//  IMPLEMENTATIONS                                          //
//-----------------------------------------------------------//

// Throws an exception if the next value in the input stream is not equal to the expected value
inline void expect(std::istream& is, const std::string& expected, const std::string& error_message) {
  std::string value = get<std::string>(is);
  if (value != expected) {
    if (!error_message.empty())
      throw std::runtime_error(error_message);
    throw std::runtime_error("Expected " + expected + ", got " + value);
  }
}

// Get a value of type T from an input stream
template<typename T>
inline T get(std::istream& is) {
  T value;
  if (!(is >> value))
    throw std::runtime_error("Failed to read value from input stream");
  return value;
}

inline std::array<char, 4> get_tree_iterator_tag(std::istream& is) {
  using IteratorTag = std::array<char, 4>;

  std::string tag_string = get<std::string>(is);

  if (tag_string.size() != 4)
    throw std::runtime_error("Invalid tree iterator tag size should be 4, got " + std::to_string(tag_string.size()) + " for tag: \"" + tag_string + "\"");

  IteratorTag tag{};
  std::copy(tag_string.begin(), tag_string.end(), tag.begin());

  return tag;
}
