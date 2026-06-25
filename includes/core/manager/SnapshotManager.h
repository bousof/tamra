/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class that handles tree swnapshot and restore.
 */

#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <istream>
#include <limits>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <core/RootCellEntry.h>
#include <core/iterator/tree_iterator_types.h>
#include "../../utils/inout_utils.h"

// Enumeration for strategies on how to handle conflicts for owned cells.
// Possible values are:
// - `SFC_COMPRESSED`:    Assumes restart uses the same SFC (efficient in storage, I/O, and repartitioning)
// - `PORTABLE_EXPLICIT`: More compatible (larger files and potentially more expensive redistribution, because cells from one old partition may be scattered across many new partitions)
enum class SnapshotPortability {
    SFC_COMPRESSED,
    PORTABLE_EXPLICIT
};

class SnapshotMetadata {
 public:
  unsigned version;
  unsigned subversion;
  bool binary;
  unsigned size;
  unsigned min_level;
  unsigned max_level;
  std::string iterator_str_tag;

  SnapshotMetadata() {};
};

template<typename TreeType>
class SnapshotManager {
  static constexpr unsigned VERSION_NUMBER = 1;
  static constexpr unsigned SUBVERSION_NUMBER = 0;

  //***********************************************************//
  //  VARIABLES                                                //
  //***********************************************************//
  // Minimum mesh level
  const unsigned min_level;
  // Maximum mesh level
  const unsigned max_level;
  // Process rank
  const unsigned rank;
  // Number of process
  const unsigned size;
  // Binary file format
  const bool binary;
  // Metadata
  SnapshotMetadata metadata;

  //***********************************************************//
  //  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
  //***********************************************************//
 public :
  // Constructor
  SnapshotManager(const unsigned min_level, const unsigned max_level, const unsigned rank, const unsigned size, const bool binary = false);
  // Destructor
  ~SnapshotManager() = default;

  //***********************************************************//
  //  METHODS                                                  //
  //***********************************************************//
 public:
  // Dump the tree metadata and data to a string representation
  std::string dumpMetaAndTreeToString(const TreeType& tree);
  // Dump the tree metadata and data to a file
  void dumpMetaAndTreeToFile(const TreeType& tree, const std::string& filename);
  // Read the tree metadata and restore the tree data from a string representation
  TreeType readMetaAndRestoreFromString(const std::string& snapshot_string);
  // Read the tree metadata and restore the tree data from a file
  TreeType readMetaAndRestoreFromFile(const std::string& filename);
  // Dump the tree metadata to a string representation
  std::string dumpMetaToString(const TreeType& tree);
  // Dump the tree metadata to a file
  void dumpMetaToFile(const TreeType& tree, const std::string& filename);
  // Read the tree metadata from a string representation
  void readMetaFromString(const std::string& snapshot_string);
  // Restore the tree metadata from a file
  void readMetaFromFile(const std::string& filename);
  // Dump the tree data to a string representation
  std::string dumpToString(const TreeType& tree);
  // Dump the tree data to a file
  void dumpToFile(const TreeType& tree, const std::string& filename);
  // Restore the tree data from a string representation
  TreeType restoreFromString(const std::string& snapshot_string);
  // Restore the tree data from a file
  TreeType restoreFromFile(const std::string& filename);
 private:
  // Dump the tree metadata to an output stream
  void dumpMeta(const TreeType& tree, std::ostream& os);
  // Read the tree metadata from an input stream
  void readMeta(std::istream& is);
  // Dump the tree data to an output stream
  void dump(const TreeType& tree, std::ostream& os);
  // Restore the tree data from an input stream
  TreeType restore(std::istream& is);
  // Dump the tree root cells and their neighbors to an output stream
  void dumpRootCells(const TreeType& tree, std::ostream& os);
  // Restore the tree root cells and their neighbors from an input stream
  void restoreRootCells(TreeType& tree, std::istream& is);
  // Dump the tree leaf cells structure to an output stream
  void dumpLeafCells(const TreeType& tree, std::ostream& os);
  // Restore the tree leaf cells structure from an input stream using a specific iterator type
  void restoreLeafCells(TreeType& tree, std::istream& is, const std::string& iterator_type);
  // Restore the tree leaf cells structure from an input stream using a specific templated iterator type
  template<typename IteratorType>
  void restoreLeafCellsWithIterator(TreeType& tree, std::istream& is);
  // Dump the tree cells data to an output stream
  void dumpCellData(const TreeType& tree, std::ostream& os);
  // Restore the tree cells data from an input stream
  void restoreCellData(TreeType& tree, std::istream& is);
};

#include "SnapshotManager.tpp"
