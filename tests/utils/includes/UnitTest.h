/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Handles unit tests.
 */

#pragma once

#include <functional>
#include <iostream>
#include <string>

using FuncType = std::function<bool(void)>;

class UnitTest {
  //***********************************************************//
  //  DATA                                                     //
  //***********************************************************//
 private:
  std::string name;
  std::string group;
  FuncType func;

  //***********************************************************//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //***********************************************************//
 public:
  UnitTest(std::string name, std::string group, FuncType func);

  //***********************************************************//
  //  ACCESSORS                                                //
  //***********************************************************//
 public:
  // Get test name
  const std::string& getName() const { return name; }
  // Get test group
  const std::string& getGroup() const { return group; }

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Call the funcition to run the test
  bool run() const;
};
