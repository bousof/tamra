#include "SnapshotManager.h"
//#include "../../utils/display_vector.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename TreeType>
SnapshotManager<TreeType>::SnapshotManager(const unsigned rank, const unsigned size, const bool binary)
: rank(rank),
  size(size),
  binary(binary),
  metadata() {}


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Dump the tree metadata and data to an output stream
template<typename TreeType>
void SnapshotManager<TreeType>::dumpMetaAndTree(const TreeType& tree, std::ostream& os) {
  dumpMeta(tree, os);
  dump(tree, os);
}

// Read the tree metadata and data from an input stream
template<typename TreeType>
TreeType SnapshotManager<TreeType>::readMetaAndRestore(std::istream& is) {
  readMeta(is);
  return restore(is);
}

// Dump the tree metadata and data to a string representation
template<typename TreeType>
std::string SnapshotManager<TreeType>::dumpMetaAndTreeToString(const TreeType& tree) {
  std::ostringstream oss(std::ios::binary);
  dumpMeta(tree, oss);
  dump(tree, oss);
  return oss.str();
}

// Dump the tree metadata and data to a file
template<typename TreeType>
void SnapshotManager<TreeType>::dumpMetaAndTreeToFile(const TreeType& tree, const std::string& filename) {
  std::ofstream ofs(filename, std::ios::binary);
  if (!ofs)
    throw std::runtime_error("SnapshotManager::dumpMetaAndTreeToFile: cannot open file " + filename);
  dumpMeta(tree, ofs);
  dump(tree, ofs);
}

// Read the tree metadata and restore the tree data from a string representation
template<typename TreeType>
TreeType SnapshotManager<TreeType>::readMetaAndRestoreFromString(const std::string& snapshot_string) {
  std::istringstream iss(snapshot_string, std::ios::binary);
  readMeta(iss);
  return restore(iss);
}

// Read the tree metadata and restore the tree data from a file
template<typename TreeType>
TreeType SnapshotManager<TreeType>::readMetaAndRestoreFromFile(const std::string& filename) {
  std::ifstream ifs(filename, std::ios::binary);
  if (!ifs)
    throw std::runtime_error("SnapshotManager::readMetaAndRestoreFromFile: cannot open file " + filename);
  readMeta(ifs);
  return restore(ifs);
}

// Dump the tree metadata to a string representation
template<typename TreeType>
std::string SnapshotManager<TreeType>::dumpMetaToString(const TreeType& tree) {
  std::ostringstream oss(std::ios::binary);
  dumpMeta(tree, oss);
  return oss.str();
}

// Read the tree metadata from a string representation
template<typename TreeType>
void SnapshotManager<TreeType>::readMetaFromString(const std::string &snapshot_string) {
  std::istringstream iss(snapshot_string, std::ios::binary);
  return readMeta(iss);
}

// Dump the tree data to a string representation
template<typename TreeType>
std::string SnapshotManager<TreeType>::dumpToString(const TreeType& tree) {
  std::ostringstream oss(std::ios::binary);
  dump(tree, oss);
  return oss.str();
}

// Restore the tree data from a string representation
template<typename TreeType>
TreeType SnapshotManager<TreeType>::restoreFromString(const std::string &snapshot_string) {
  std::istringstream iss(snapshot_string, std::ios::binary);
  return restore(iss);
}

// Dump the tree metadata to a string representation
template<typename TreeType>
void SnapshotManager<TreeType>::dumpMeta(const TreeType& tree, std::ostream& os) {
  // Metadata for the snapshot
  os << "TAMRA_SNAPSHOT_METADATA\n";
  os << "VERSION " << VERSION_NUMBER << " " << SUBVERSION_NUMBER << "\n";
  os << "BINARY " << (binary ? "1" : "0") << "\n";
  os << "MPI_SIZE " << size << "\n";
  os << "LEVELS " << tree.getMinLevel() << " " << tree.getMaxLevel() << "\n";
  {
    const auto tag = TreeType::TreeIteratorType::CONFIG_SELECTION_NAME;
    os << "ITERATOR ";
    for (char c : tag) os << c;
    os << "\n";
  }
}

// Read the tree metadata from an input stream
template<typename TreeType>
void SnapshotManager<TreeType>::readMeta(std::istream& is) {
  {
    expect(is, "TAMRA_SNAPSHOT_METADATA", "SnapshotManager::restore: invalid snapshot metadata format");
  }
  {
    expect(is, "VERSION");
    metadata.version = get<unsigned>(is);
    metadata.subversion = get<unsigned>(is);
  }
  {
    expect(is, "BINARY");
    metadata.binary = !!get<unsigned>(is);
  }
  {
    expect(is, "MPI_SIZE");
    metadata.size = get<unsigned>(is);
    if (metadata.size > 1)
      throw std::runtime_error("Cannot dump partitioned tree yet");
  }
  {
    expect(is, "LEVELS");
    metadata.min_level = get<unsigned>(is);
    metadata.max_level = get<unsigned>(is);
  }
  {
    expect(is, "ITERATOR");
    metadata.iterator_str_tag = get<std::string>(is);
  }
}

// Dump the tree data to a string representation
template<typename TreeType>
void SnapshotManager<TreeType>::dump(const TreeType& tree, std::ostream& os) {
  // Metadata for the snapshot
  os << "TAMRA_SNAPSHOT_DATA\n";
  dumpRootCells(tree, os);
  dumpLeafCells(tree, os);
  dumpCellData(tree, os);
}

// Restore the tree data from an input stream
template<typename TreeType>
TreeType SnapshotManager<TreeType>::restore(std::istream& is) {
  // First assert that the current tree structure is compatible with the snapshot data
  {
    expect(is, "TAMRA_SNAPSHOT_DATA", "SnapshotManager::restore: invalid snapshot format");
  }
  { // Verify snapshot manager version
    if (metadata.version != VERSION_NUMBER || metadata.subversion != SUBVERSION_NUMBER)
      throw std::runtime_error("Expected version \"" + std::to_string(VERSION_NUMBER) + " " + std::to_string(SUBVERSION_NUMBER) + "\"" +
                               ", but got \"" + std::to_string(metadata.version) + "." + std::to_string(metadata.subversion) + "\"");
  }
  { // Verify size
    // - if the size for restorE is bigger, the last processes have empty partitions
    // - if size for restore is smaller, throw an error
    if (metadata.size > 1)
      throw std::runtime_error("Cannot restore partitioned tree yet");
    if (metadata.size > size)
      throw std::runtime_error("Cannot restore from a snapshot taken with a higher number of processes");
  }
  { // Verify snapshot iterator compatibility.
    // One can either:
    // - reload a snapshot from all iterators if size==1
    // - only from same tree iterator if size > 1
    const auto restore_iterator_tag = TreeType::TreeIteratorType::CONFIG_SELECTION_NAME;
    const std::string restore_iterator_str_tag = std::string(restore_iterator_tag.begin(), restore_iterator_tag.end());
    if ((metadata.iterator_str_tag != restore_iterator_str_tag) && (size > 1))
      throw std::runtime_error("Cannot restore from a different tree iterator for >1 processes");
  }
  // Create a new tree with the snapshot's max levels to ensure cell Ids are compatible.
  // The tree will be cast to the correct max level after the snapshot is restored.
  TreeType tree(metadata.min_level, metadata.max_level, rank, size);
  restoreRootCells(tree, is);
  restoreLeafCells(tree, is, metadata.iterator_str_tag);
  restoreCellData(tree, is);
  return tree;
}

// Dump the tree root cells and their neighbors to an output stream
template<typename TreeType>
void SnapshotManager<TreeType>::dumpRootCells(const TreeType& tree, std::ostream& os) {
  using CellType = typename TreeType::CellType;

  // Dump the number of root cells
  os << "ROOT_CELLS " << tree.getRootCells().size() << "\n";
  // For each root cell, dump its data
  std::unordered_map<CellType*, unsigned> root_cell_indices;
  for (const auto& root_cell : tree.getRootCells())
    root_cell_indices[root_cell.get()] = root_cell_indices.size();
  // For each root cell, find the neighboring root cells
  std::vector<std::vector<int>> root_cell_neighbors(tree.getRootCells().size());
  for (size_t i{0}; i<tree.getRootCells().size(); ++i) {
    // initialize with out-of-range index (use size as sentinel)
    root_cell_neighbors[i].assign(CellType::number_neighbors, -1);
    const auto& root_cell = tree.getRootCells()[i];
    for (unsigned dir{0}; dir<CellType::number_neighbors; ++dir)
      if (root_cell->getNeighborCell(dir))
        root_cell_neighbors[i][dir] = root_cell_indices[root_cell->getNeighborCell(dir).get()];
  }
  // Dump the root cell neighbors
  for (size_t i{0}; i<tree.getRootCells().size(); ++i) {
    os << "ROOT_CELL " << i << " NEIGHBORS";
    for (unsigned dir{0}; dir<CellType::number_neighbors; ++dir)
      os << " " << root_cell_neighbors[i][dir];
    os << "\n";
  }
}

// Restore the tree root cells and their neighbors from an input stream
template<typename TreeType>
void SnapshotManager<TreeType>::restoreRootCells(TreeType& tree, std::istream& is) {
  using CellType = typename TreeType::CellType;

  // Get the number of root cells
  expect(is, "ROOT_CELLS");
  const unsigned root_cells_size = get<unsigned>(is);

  // Create root cells with nullptr parents and store them in a vector
  std::vector<std::shared_ptr<CellType>> root_cells;
  root_cells.reserve(root_cells_size);
  for (unsigned i = 0; i < root_cells_size; ++i)
    root_cells.push_back(std::make_shared<CellType>(nullptr));

  // Create root cell entries
  std::vector<RootCellEntry<CellType>> entries;
  entries.reserve(root_cells_size);
  for (unsigned i = 0; i < root_cells_size; ++i)
    entries.emplace_back(root_cells[i]);

  // Restore the neighbor relationships for each root cell
  for (unsigned i = 0; i < root_cells_size; ++i) {
    expect(is, "ROOT_CELL");
    const unsigned cell_index = get<unsigned>(is);
    if (cell_index != i)
      throw std::runtime_error("SnapshotManager::restoreRootCells: unexpected root cell index");

    expect(is, "NEIGHBORS");
    for (unsigned dir = 0; dir < CellType::number_neighbors; ++dir) {
      const int neighbor_index = get<int>(is);
      if (neighbor_index >= 0) {
        if (neighbor_index >= static_cast<int>(root_cells_size))
          throw std::runtime_error("SnapshotManager::restoreRootCells: invalid neighbor index");

        entries[i].setNeighbor(dir,root_cells[neighbor_index]);
      }
    }
  }

  // Create the root cells in the tree
  tree.createRootCells(entries);
}

// Dump the tree leaf cells structure to an output stream
template<typename TreeType>
void SnapshotManager<TreeType>::dumpLeafCells(const TreeType& tree, std::ostream& os) {
  using TreeIteratorType = typename TreeType::TreeIteratorType;

  // Dump the index path of the first leaf cell of the tree
  TreeIteratorType iterator(tree.getRootCells(), tree.getMaxLevel());
  if (tree.getRootCells().empty()) {
    os << "FIRST_LEAF_CELL 0\n";
    os << "PARTITION -1 -1\n";
    os << "LEAF_LEVELS 0\n";
    return;
  }

  iterator.toBegin();
  os << "FIRST_LEAF_CELL " << iterator.getIndexPath().size();
  for (unsigned i: iterator.getIndexPath())
    os << " " << i;
  os << "\n";

  int first_owned_leaf = -1;
  int last_owned_leaf = -1;
  unsigned number_leaf_cells = 0;
  std::vector<unsigned> leaf_levels;
  do {
    if (iterator.getCell()->belongToThisProc()) {
      if (first_owned_leaf < 0)
        first_owned_leaf = static_cast<int>(number_leaf_cells);
      last_owned_leaf = static_cast<int>(number_leaf_cells);
    }
    leaf_levels.push_back(iterator.getCell()->getLevel());
    ++number_leaf_cells;
  } while (iterator.next());

  os << "PARTITION " << first_owned_leaf << " " << last_owned_leaf << "\n";
  os << "LEAF_LEVELS " << number_leaf_cells;
  for (unsigned leaf_level : leaf_levels)
    os << " " << leaf_level;
  os << "\n";
}

// Restore the tree leaf cells structure from an input stream using a specific iterator type
template<typename TreeType>
void SnapshotManager<TreeType>::restoreLeafCells(TreeType& tree, std::istream& is, const std::string& iterator_type) {
  using CellType = typename TreeType::CellType;

  dispatch_on_tree_iterator<CellType>(
    iterator_type,
    [this, &tree, &is](auto iterator_tag) {
      using IteratorType = typename decltype(iterator_tag)::type;

      this->template restoreLeafCellsWithIterator<IteratorType>(tree, is);
    }
  );
}

// Restore the tree leaf cells structure from an input stream using a specific templated iterator type
template<typename TreeType>
template<typename IteratorType>
void SnapshotManager<TreeType>::restoreLeafCellsWithIterator(TreeType& tree, std::istream& is) {
  // Create an iterator instance for the tree
  IteratorType iterator(tree.getRootCells(), tree.getMaxLevel());

  // Initialize all cells to belong to other process
  for (const auto &root_cell : tree.getRootCells())
    root_cell->setToOtherProcRecurs();

  // Get the index path of the first leaf cell of the partition
  expect(is, "FIRST_LEAF_CELL");
  const unsigned first_leaf_cell_index_path_size = get<unsigned>(is);
  if (first_leaf_cell_index_path_size == 0) {
    expect(is, "PARTITION");
    expect(is, "-1");
    expect(is, "-1");
    expect(is, "LEAF_LEVELS");
    expect(is, "0");
    return;
  }
  std::vector<unsigned> first_leaf_cell_index_path(first_leaf_cell_index_path_size);
  for (unsigned i = 0; i < first_leaf_cell_index_path_size; ++i)
    first_leaf_cell_index_path[i] = get<unsigned>(is);
  iterator.toIndexPath(first_leaf_cell_index_path, true);

  expect(is, "PARTITION");
  const int first_owned_leaf = get<int>(is);
  const int last_owned_leaf = get<int>(is);

  // Get the levels of the leaf cells
  expect(is, "LEAF_LEVELS");
  const unsigned number_leaf_cells = get<unsigned>(is);
  for (unsigned i{0}; i < number_leaf_cells; ++i) {
    const unsigned leaf_level = get<unsigned>(is);

    // Split cell until reaching correct leaf level, and set the cell to belong to this process
    while (iterator.getCell()->getLevel()<leaf_level) {
      if (iterator.getCell()->isLeaf())
        iterator.getCell()->split(metadata.max_level);
      iterator.toLeaf(leaf_level);
    }

    if (first_owned_leaf >= 0 &&
        static_cast<int>(i) >= first_owned_leaf &&
        static_cast<int>(i) <= last_owned_leaf)
      iterator.getCell()->setToThisProcRecurs();

    if (!iterator.getCell()->isLeaf())
      // Conformity-driven splits can turn the current location into a non-leaf.
      // Always advance to avoid stalling traversal.
      iterator.next();
    else
      iterator.next();
  }

  // Backpropagate flags from leaf to all cells
  for (const auto &root_cell : tree.getRootCells())
    backPropagateOwnershipFlags(root_cell);
}

// Dump the tree cells data to an output stream
template<typename TreeType>
void SnapshotManager<TreeType>::dumpCellData(const TreeType& tree, std::ostream& os) {
  using CellType = typename TreeType::CellType;

  os << "CELL_DATA " << tree.countCells() << "\n";
  if (this->binary) {
    tree.applyToAllCells(
      [&os](const std::shared_ptr<CellType> &cell, unsigned) mutable {
        cell->getCellData().dump(os, true);
      }
    );
  } else {
    unsigned counter{0};
    tree.applyToAllCells(
      [&os, &counter](const std::shared_ptr<CellType> &cell, unsigned) mutable {
        cell->getCellData().dump(os, false);
        if (++counter%10==0)
          os << "\n";
      }
    );
    if (counter%10!=0)
      os << "\n";
  }
}

// Restore the tree cells data from an input stream
template<typename TreeType>
void SnapshotManager<TreeType>::restoreCellData(TreeType& tree, std::istream& is) {
  using CellType = typename TreeType::CellType;

  // Get the levels of the leaf cells
  expect(is, "CELL_DATA");
  const unsigned number_cells = get<unsigned>(is);
  is.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore line break
  if (number_cells != tree.countCells())
    throw std::runtime_error("SnapshotManager::restoreCellData: cell count mismatch");
  if (number_cells > 0) {
    // Restore the tree cells
    const bool binary = this->metadata.binary;
    tree.applyToAllCells(
      [&is, &binary](const std::shared_ptr<CellType> &cell, unsigned) mutable {
        // Restoring the values of the cells
        cell->getCellData().restore(is, binary);
      }
    );
  }
}

// Set a parent to belong to this proc if any of its child do else set to other proc
template<typename TreeType>
bool SnapshotManager<TreeType>::backPropagateOwnershipFlags(const std::shared_ptr<CellType> &cell) const {
  if (cell->isLeaf())
    return cell->belongToThisProc();

  bool to_this_proc = false;
  for (const auto &child : cell->getChildCells())
    if (backPropagateOwnershipFlags(child))
      to_this_proc = true;

  if (to_this_proc)
    cell->setToThisProc();
  else
    cell->setToOtherProc();

  return to_this_proc;
}
