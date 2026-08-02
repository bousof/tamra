#include <doctest.h>

#include <cstring>
#include <iomanip>
#include <memory>
#include <vector>

#include <core/Cell.h>
#include <core/iterator/MortonIterator.h>
#include <core/RootCellEntry.h>
#include <core/manager/SnapshotManager.h>
#include <core/Tree.h>
#include <parallel/allreduce.h>

namespace core::manager::snapshot::mpi {
template<int Nx, int Ny, int Nz>
class TestCellData : public CellData {
 public:
  unsigned imin;   // Minimum i-coordinate
  unsigned imax;   // Maximum i-coordinate
  unsigned jmin;   // Minimum j-coordinate
  unsigned jmax;   // Maximum j-coordinate
  unsigned kmin;   // Minimum k-coordinate
  unsigned kmax;   // Maximum k-coordinate

  // Constructor
  TestCellData()
  : imin(0), imax(0), jmin(0), jmax(0), kmin(0), kmax(0) {};
  // Destructor
  ~TestCellData() = default;

  // Extrapolate from current cell to child cells
  void extrapolateToChild(TestCellData &child_cell_data, const unsigned sibling_number) const {
    const unsigned i = sibling_number % Nx,
                    j = (sibling_number / Nx) % Ny,
                    k = (sibling_number / (Nx * Ny));

    child_cell_data.imin = imin + (  i   * (imax-imin)) / Nx;
    child_cell_data.imax = imin + ((i+1) * (imax-imin)) / Nx;
    child_cell_data.jmin = jmin + (  j   * (jmax-jmin)) / Ny;
    child_cell_data.jmax = jmin + ((j+1) * (jmax-jmin)) / Ny;
    if constexpr (Nz > 0) {
      child_cell_data.kmin = kmin + (  k   * (kmax-kmin)) / Nz;
      child_cell_data.kmax = kmin + ((k+1) * (kmax-kmin)) / Nz;
    }
  }
  // Conversion as vector of double for data communication
  void fromVectorOfData(const std::vector<double> &buffer) override {
    std::memcpy(&imin, &buffer[0], sizeof(double));
    std::memcpy(&imax, &buffer[1], sizeof(double));
    std::memcpy(&jmin, &buffer[2], sizeof(double));
    std::memcpy(&jmax, &buffer[3], sizeof(double));
    if constexpr (Nz > 0) {
      std::memcpy(&kmin, &buffer[4], sizeof(double));
      std::memcpy(&kmax, &buffer[5], sizeof(double));
    }
  }
  std::vector<double> toVectorOfData() const override {
    std::vector<double> data(getDataSize());
    std::memcpy(&data[0], &imin, sizeof(double));
    std::memcpy(&data[1], &imax, sizeof(double));
    std::memcpy(&data[2], &jmin, sizeof(double));
    std::memcpy(&data[3], &jmax, sizeof(double));
    if constexpr (Nz > 0) {
      std::memcpy(&data[4], &kmin, sizeof(double));
      std::memcpy(&data[5], &kmax, sizeof(double));
    }
    return data;
  }
  unsigned getDataSize() const override {
    if constexpr (Nz > 0)
      return 6;
    else
      return 4;
  }
  // Dump the cell data to an output stream
  void dump(std::ostream& os, const bool binary) const override {
    if (!binary) {
      os << " " << imin << " " << imax;
      os << " " << jmin << " " << jmax;
      if constexpr (Nz > 0)
        os << " " << kmin << " " << kmax;
    } else {
      os.write(reinterpret_cast<const char*>(&imin), sizeof(unsigned));
      os.write(reinterpret_cast<const char*>(&imax), sizeof(unsigned));
      os.write(reinterpret_cast<const char*>(&jmin), sizeof(unsigned));
      os.write(reinterpret_cast<const char*>(&jmax), sizeof(unsigned));
      if constexpr (Nz > 0) {
        os.write(reinterpret_cast<const char*>(&kmin), sizeof(unsigned));
        os.write(reinterpret_cast<const char*>(&kmax), sizeof(unsigned));
      }
    }
  }
  // Restore the cell data from an input stream
  void restore(std::istream& is, const bool binary) override {
    if (!binary) {
      is >> imin >> imax;
      is >> jmin >> jmax;
      if constexpr (Nz > 0)
        is >> kmin >> kmax;
    } else {
      is.read(reinterpret_cast<char*>(&imin), sizeof(unsigned));
      is.read(reinterpret_cast<char*>(&imax), sizeof(unsigned));
      is.read(reinterpret_cast<char*>(&jmin), sizeof(unsigned));
      is.read(reinterpret_cast<char*>(&jmax), sizeof(unsigned));
      if constexpr (Nz > 0) {
        is.read(reinterpret_cast<char*>(&kmin), sizeof(unsigned));
        is.read(reinterpret_cast<char*>(&kmax), sizeof(unsigned));
      }
    }
  }
  bool isEqual(const TestCellData &cell_data) const {
    bool equal = true;
    equal &= (imin == cell_data.imin);
    equal &= (imax == cell_data.imax);
    equal &= (jmin == cell_data.jmin);
    equal &= (jmax == cell_data.jmax);
    if constexpr (Nz > 0) {
      equal &= (kmin == cell_data.kmin);
      equal &= (kmax == cell_data.kmax);
    }
    return equal;
  }
};

template<typename CellType>
void initialize_tree_cells_limits(std::shared_ptr<CellType> cell) {
  if (cell->isLeaf())
    return;

  const auto &cell_data = cell->getCellData();
  for (unsigned n{0}; n<CellType::number_children; ++n) {
    auto &child_cell_data = cell->getChildCell(n)->getCellData();
    cell_data.extrapolateToChild(child_cell_data, n);
    initialize_tree_cells_limits<CellType>(cell->getChildCell(n));
  }
}

template<typename CellType>
void check_cells_data(std::shared_ptr<CellType> cell, std::shared_ptr<CellType> restored_cell) {
  // Verify the coordinates
  CHECK(cell->getCellData().isEqual(restored_cell->getCellData()));
  CHECK(cell->isLeaf() == restored_cell->isLeaf()); // Both split or both leaves

  if (cell->isLeaf())
    return;

  const auto &cell_data = cell->getCellData();
  for (unsigned n{0}; n<CellType::number_children; ++n) {
    auto &child_cell_data = cell->getChildCell(n)->getCellData();
    cell_data.extrapolateToChild(child_cell_data, n);
    check_cells_data<CellType>(cell->getChildCell(n), restored_cell->getChildCell(n));
  }
}

template<typename CellType, typename CompareData>
bool compare_cells(const std::shared_ptr<CellType>& cell,
                   const std::shared_ptr<CellType>& restored_cell,
                   CompareData compare_data) {
  bool passed = true;
  passed &= compare_data(cell->getCellData(), restored_cell->getCellData());
  passed &= (cell->isLeaf() == restored_cell->isLeaf());
  passed &= (cell->belongToThisProc() == restored_cell->belongToThisProc());
  passed &= (cell->belongToOtherProc() == restored_cell->belongToOtherProc());

  if (!passed || cell->isLeaf())
    return passed;

  for (unsigned n{0}; n<CellType::number_children; ++n)
    passed &= compare_cells<CellType>(cell->getChildCell(n), restored_cell->getChildCell(n), compare_data);

  return passed;
}

template<typename TreeType, typename CompareData>
bool compare_trees(const TreeType& tree,
                   const TreeType& restored_tree,
                   CompareData compare_data) {
  bool passed = true;
  passed &= (tree.getRootCells().size() == restored_tree.getRootCells().size());
  passed &= (tree.countOwnedLeaves() == restored_tree.countOwnedLeaves());
  passed &= (tree.countCells() == restored_tree.countCells());

  if (!passed)
    return false;

  for (size_t i{0}; i<tree.getRootCells().size(); ++i)
    passed &= compare_cells<typename TreeType::CellType>(
      tree.getRootCells()[i],
      restored_tree.getRootCells()[i],
      compare_data
    );

  return passed;
}

template<typename TreeType>
void assign_incremental_values(TreeType& tree) {
  unsigned counter{0};
  tree.applyToAllCells(
    [&counter](const std::shared_ptr<typename TreeType::CellType> &cell, unsigned) mutable {
      cell->getCellData().setValue(static_cast<double>(++counter));
    }
  );
}

void printSnapshotDebug(const std::string& snapshot) {
  const bool binary = snapshot.find("BINARY 1") != std::string::npos;

  constexpr const char* tag = "CELL_DATA";
  const auto pos = snapshot.find(tag);
  if (pos == std::string::npos) {
    std::cout << snapshot << std::endl;
    return;
  }

  const auto payload_pos = snapshot.find('\n', pos);
  if (payload_pos == std::string::npos) {
    std::cout << snapshot << std::endl;
    return;
  }

  // Print everything up to and including CELL_DATA line
  std::cout.write(snapshot.data(), payload_pos + 1);
  if (!binary) {
    std::cout << "\n[TEXT PAYLOAD]\n";
    std::cout.write(
      snapshot.data() + payload_pos + 1,
      snapshot.size() - payload_pos - 1
    );

    std::cout << std::endl;
    return;
  }

  std::cout << "\n[BINARY PAYLOAD]\n";
  const unsigned char* data = reinterpret_cast<const unsigned char*>(snapshot.data());

  for (std::size_t i = payload_pos + 1; i < snapshot.size(); ++i) {
    std::cout
      << std::hex
      << std::setw(2)
      << std::setfill('0')
      << static_cast<unsigned>(data[i])
      << ' ';

    if ((i - payload_pos - 1) % 16 == 15)
      std::cout << '\n';
  }

  std::cout << std::dec << std::endl;
}

std::vector<std::string> make_partition_snapshots(const unsigned source_size,
                                                  const unsigned max_level,
                                                  const std::vector<unsigned>& partition_sizes) {
  using Cell1D = Cell<2>;
  using BinaryTree = Tree<Cell1D>;

  std::vector<std::string> snapshots;
  snapshots.reserve(source_size);
  unsigned partition_start{0};
  const unsigned number_leaf_cells = 1u << max_level;

  for (unsigned source_rank{0}; source_rank<source_size; ++source_rank) {
    auto root = std::make_shared<Cell1D>(nullptr);
    RootCellEntry<Cell1D> entry{root};
    BinaryTree tree(1, max_level, source_rank, source_size);
    tree.createRootCells({entry});

    auto split_to_level = [&tree](auto&& self, const std::shared_ptr<Cell1D>& cell) -> void {
      if (cell->getLevel() >= tree.getMaxLevel())
        return;
      if (cell->isLeaf())
        cell->split(tree.getMaxLevel());
      for (const auto& child : cell->getChildCells())
        self(self, child);
    };
    split_to_level(split_to_level, root);

    root->setToOtherProcRecurs();
    MortonIterator<Cell1D> iterator(tree.getRootCells(), tree.getMaxLevel());
    iterator.toBegin();
    const unsigned partition_end = partition_start + partition_sizes[source_rank];
    for (unsigned leaf_index{0}; leaf_index<number_leaf_cells; ++leaf_index) {
      iterator.getCell()->getCellData().setValue(static_cast<double>(leaf_index));
      if (leaf_index >= partition_start && leaf_index < partition_end)
        iterator.getCell()->setToThisProc();
      if (leaf_index + 1 < number_leaf_cells)
        iterator.next();
    }
    if (partition_sizes[source_rank] > 0)
      root->setToThisProc();

    SnapshotManager<BinaryTree> snapshot_manager(source_rank, source_size);
    snapshots.push_back(snapshot_manager.dumpMetaAndTreeToString(tree));
    partition_start = partition_end;
  }

  return snapshots;
}
}

// Snapshot of a binary tree with 1 root cell and 2 leaf cells in proc 0
//                  rank 0
//                │   A   │
// structure  ->  └───┴───┘
// All other processes have empty partitions
TEST_CASE("[core][manager][snapshot][mpi] Snapshot of a binary tree with 1 root cell and 2 leaf cells in proc 0") {
  using Cell1D = Cell<2>;
  using BinaryTree = Tree<Cell1D>;
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  // Create root cell
  auto A = std::make_shared<Cell1D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell1D> eA{A};
  std::vector<RootCellEntry<Cell1D>> entries { eA };

  // Construction of the tree
  unsigned min_level{1}, max_level{2};
  BinaryTree tree(min_level, max_level);
  tree.createRootCells(entries);

  // Process 0 owns the tree, all other processes have empty partitions
  if (rank == 0)
    A->setToThisProcRecurs();
  else
    A->setToOtherProcRecurs();

  core::manager::snapshot::mpi::assign_incremental_values(tree);

  bool passed = true;

  // Count number of leaf cells
  int number_owned_leaf_cells = A->countOwnedLeaves();
  passed &= (number_owned_leaf_cells == ((rank == 0) ? 2 : 0));

  // Create a snapshot manager
  SnapshotManager<BinaryTree> snapshot_manager(rank, size);
  // Dump the tree to a string representation
  std::string snapshot_string = snapshot_manager.dumpMetaAndTreeToString(tree);
  // Restore the tree from the snapshot string
  SnapshotManager<BinaryTree> restore_manager(rank, size);
  BinaryTree restored_tree = restore_manager.readMetaAndRestoreFromString(snapshot_string);
  passed &= core::manager::snapshot::mpi::compare_trees<BinaryTree>(
    tree,
    restored_tree,
    [](const auto& lhs, const auto& rhs) {
      return lhs.getValue() == rhs.getValue();
    }
  );

  bool all_passed;
  boolAndAllreduce(passed, all_passed);
  CHECK(all_passed);
}

TEST_CASE("[core][manager][snapshot][mpi] Snapshot of a quadtree meshed at min level (one root, parallel)") {
  using Cell2D = Cell<2, 2, 0, core::manager::snapshot::mpi::TestCellData<2, 2, 0>>;
  using QuadTree = Tree<Cell2D, MortonIterator<Cell2D, 213>>;
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  auto A = std::make_shared<Cell2D>(nullptr);
  RootCellEntry<Cell2D> eA{A};
  std::vector<RootCellEntry<Cell2D>> entries { eA };

  unsigned min_level{2}, max_level{3};
  QuadTree tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);
  tree.meshAtMinLevel();

  A->getCellData().imin = 0;
  A->getCellData().imax = 1u << min_level;
  A->getCellData().jmin = 0;
  A->getCellData().jmax = 1u << min_level;
  core::manager::snapshot::mpi::initialize_tree_cells_limits(A);

  SnapshotManager<QuadTree> snapshot_manager(rank, size);
  std::string snapshot_string = snapshot_manager.dumpMetaAndTreeToString(tree);
  SnapshotManager<QuadTree> restore_manager(rank, size);
  QuadTree restored_tree = restore_manager.readMetaAndRestoreFromString(snapshot_string);

  bool passed = core::manager::snapshot::mpi::compare_trees<QuadTree>(
    tree,
    restored_tree,
    [](const auto& lhs, const auto& rhs) {
      return lhs.isEqual(rhs);
    }
  );

  bool all_passed;
  boolAndAllreduce(passed, all_passed);
  CHECK(all_passed);
}

TEST_CASE("[core][manager][snapshot][mpi] Snapshot of a refined quadtree with two roots (binary)") {
  using Cell2D = Cell<2, 2, 0, core::manager::snapshot::mpi::TestCellData<2, 2, 0>>;
  using QuadTree = Tree<Cell2D>;
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  auto A = std::make_shared<Cell2D>(nullptr);
  auto B = std::make_shared<Cell2D>(nullptr);

  RootCellEntry<Cell2D> eA{A}, eB{B};
  eA.setNeighbor(1, B);
  eB.setNeighbor(0, A);
  std::vector<RootCellEntry<Cell2D>> entries { eA, eB };

  unsigned min_level{2}, max_level{3};
  QuadTree tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);
  tree.meshAtMinLevel();

  MortonIterator<Cell2D> iterator(tree.getRootCells(), tree.getMaxLevel());
  std::vector<std::shared_ptr<Cell2D>> owned_cells_to_split;
  unsigned owned_counter{0};
  if (iterator.toOwnedBegin()) {
    do {
      if (iterator.getCell()->getLevel() < max_level && ((owned_counter + rank) % 2 == 0))
        owned_cells_to_split.push_back(iterator.getCell());
      ++owned_counter;
    } while (iterator.ownedNext());
  }
  for (const auto& cell : owned_cells_to_split)
    if (cell->isLeaf())
      cell->split(max_level);

  A->getCellData().imin = 0;
  A->getCellData().imax = 1u << min_level;
  A->getCellData().jmin = 0;
  A->getCellData().jmax = 1u << min_level;
  B->getCellData().imin = 1u << min_level;
  B->getCellData().imax = 2u << min_level;
  B->getCellData().jmin = 0;
  B->getCellData().jmax = 1u << min_level;
  core::manager::snapshot::mpi::initialize_tree_cells_limits(A);
  core::manager::snapshot::mpi::initialize_tree_cells_limits(B);

  SnapshotManager<QuadTree> snapshot_manager(rank, size, true);
  std::string snapshot_string = snapshot_manager.dumpMetaAndTreeToString(tree);
  SnapshotManager<QuadTree> restore_manager(rank, size);
  QuadTree restored_tree = restore_manager.readMetaAndRestoreFromString(snapshot_string);

  bool passed = core::manager::snapshot::mpi::compare_trees<QuadTree>(
    tree,
    restored_tree,
    [](const auto& lhs, const auto& rhs) {
      return lhs.isEqual(rhs);
    }
  );

  bool all_passed;
  boolAndAllreduce(passed, all_passed);
  CHECK(all_passed);
}

TEST_CASE("[core][manager][snapshot][mpi] Restore and merge partitions from a larger snapshot") {
  using Cell1D = Cell<2>;
  using BinaryTree = Tree<Cell1D>;
  const unsigned rank = mpi_rank();
  const unsigned size = mpi_size();
  const unsigned source_size = 2 * size;
  const unsigned max_level = (source_size == 1) ? 0 : static_cast<unsigned>(std::log2(source_size));

  std::vector<unsigned> partition_sizes(source_size, 1);
  const auto snapshots = core::manager::snapshot::mpi::make_partition_snapshots(source_size, max_level, partition_sizes);

  SnapshotManager<BinaryTree> restore_manager(rank, size);
  std::istringstream first_snapshot(snapshots[2 * rank]);
  BinaryTree restored_tree = restore_manager.readMetaAndRestore(first_snapshot);
  std::istringstream second_snapshot(snapshots[2 * rank + 1]);
  restore_manager.readMetaAndMerge(second_snapshot, restored_tree);

  bool passed = restored_tree.countOwnedLeaves() == 2;
  std::vector<double> owned_values;
  restored_tree.applyToOwnedLeaves(
    [&owned_values](const std::shared_ptr<Cell1D>& cell, unsigned) {
      owned_values.push_back(cell->getCellData().getValue());
    }
  );
  passed &= owned_values.size() == 2;
  passed &= owned_values[0] == static_cast<double>(2 * rank);
  passed &= owned_values[1] == static_cast<double>(2 * rank + 1);

  bool all_passed;
  boolAndAllreduce(passed, all_passed);
  CHECK(all_passed);
}

TEST_CASE("[core][manager][snapshot][mpi] Rebalance a merged snapshot tree") {
  using Cell1D = Cell<2>;
  using BinaryTree = Tree<Cell1D>;
  const unsigned rank = mpi_rank();
  const unsigned size = mpi_size();
  const unsigned source_size = 2 * size;
  const unsigned number_leaf_cells = 4 * size;
  const unsigned max_level = static_cast<unsigned>(std::log2(number_leaf_cells));

  std::vector<unsigned> partition_sizes(source_size, 1);
  partition_sizes.back() = number_leaf_cells - (source_size - 1);
  const auto snapshots = core::manager::snapshot::mpi::make_partition_snapshots(source_size, max_level, partition_sizes);

  SnapshotManager<BinaryTree> restore_manager(rank, size);
  std::istringstream first_snapshot(snapshots[2 * rank]);
  BinaryTree restored_tree = restore_manager.readMetaAndRestore(first_snapshot);
  std::istringstream second_snapshot(snapshots[2 * rank + 1]);
  restore_manager.readMetaAndMerge(second_snapshot, restored_tree);

  const unsigned expected_initial_owned = partition_sizes[2 * rank] + partition_sizes[2 * rank + 1];
  bool passed = restored_tree.countOwnedLeaves() == expected_initial_owned;

  restored_tree.loadBalance();

  const unsigned owned_leaf_count = restored_tree.countOwnedLeaves();
  unsigned total_owned_leaf_count;
  scalarSumAllreduce<unsigned>(owned_leaf_count, total_owned_leaf_count);
  passed &= total_owned_leaf_count == number_leaf_cells;
  passed &= owned_leaf_count >= number_leaf_cells / size - 1;
  passed &= owned_leaf_count <= number_leaf_cells / size + 1;

  double owned_value_sum{0.0};
  restored_tree.applyToOwnedLeaves(
    [&owned_value_sum](const std::shared_ptr<Cell1D>& cell, unsigned) {
      owned_value_sum += cell->getCellData().getValue();
    }
  );
  double total_value_sum;
  scalarSumAllreduce<double>(owned_value_sum, total_value_sum);
  passed &= total_value_sum == static_cast<double>(number_leaf_cells * (number_leaf_cells - 1) / 2);

  bool all_passed;
  boolAndAllreduce(passed, all_passed);
  CHECK(all_passed);
}
