/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Some functions for basic array compression.
 */

#pragma once

#include <vector>

// Compress an array of unsigned by stacking bits.
// For instance if max_value = 3, value can be 0,1,2, or 3 so it require 3 bits.
// First value is the size of the array then one unsigned (32 bits) can store 16 values.
void compress_unsigned_vector(const std::vector<unsigned> &v, std::vector<unsigned> &result, const unsigned max_value);

// Unompress an array of unsigned by stacking bits.
// For instance if max_value = 3, value can be 0,1,2, or 3 so it require 3 bits.
// First value is the size of the array then one unsigned (32 bits) can store 16 values.
void uncompress_unsigned_vector(const std::vector<unsigned> &v, std::vector<unsigned> &result, const unsigned max_value);
