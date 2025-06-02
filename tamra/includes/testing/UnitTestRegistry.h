/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Enables to register/run unit tests.
 */

#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "UnitTest.h"

class UnitTestRegistry {
  //***********************************************************//
  //  DATA                                                     //
  //***********************************************************//
 private:
  // Tests that run in serial
  static std::vector<UnitTest> serialTests;
  // Tests that run in parallel
  static std::vector<UnitTest> parallelTests;
 public:
  // Label for prefix outputs
  static std::string label;

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Register a serial test
  static void registerSerialTest(const std::string &name, FuncType func, const std::string &group = "default");
  // Register a parallel test
  static void registerParallelTest(const std::string &name, FuncType func, const std::string &group = "default");
  // Run all registered serial tests
  static void runSerial();
  // Run all registered parallel tests
  static void runParallel(const bool display = true);
 private:
  // Run an array of tests
  static void runTests(std::vector<UnitTest> &tests, const bool display = true);
};
