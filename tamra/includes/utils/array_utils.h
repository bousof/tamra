/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Some functions for basic array manipulations.
 */

#pragma once

#include <vector>

int cumulative_sum(const std::vector<int>& counts, std::vector<int>& displacements, bool startAtZero = false);
