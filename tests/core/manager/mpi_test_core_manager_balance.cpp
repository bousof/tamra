#include <doctest.h>
#include <test_macros.h>

#include <memory>
#include <vector>

#include <core/Cell.h>
#include <core/iterator/MortonIterator.h>
#include <core/RootCellEntry.h>
#include <core/Tree.h>
#include <parallel/allreduce.h>

// Load balancing (empty partitions)
// Mesh at level 3 on first process (serial) and all other process have empty partitions
// Then load balance between process
TEST_CASE("[core][manager][balance][mpi] Load balancing (empty partitions)") {
  using Cell2D = Cell<2,2>;
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector<RootCellEntry<Cell2D>> entries { eA };

  // Construction of the tree
  unsigned min_level{2}, max_level{3};
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Split to level 3 in process 0
  if (rank == 1) {
    for (const auto &child : A->getChildCells())
      child->split(max_level);
    for (const auto &child : A->getChildCells())
      for (const auto &gc : child->getChildCells())
        gc->split(max_level);
    A->setToThisProcRecurs();
  } else
    A->setToOtherProcRecurs();

  // Load balance the tree
  tree.loadBalance();

  // Count number of owned leaf cells
  unsigned number_leaf_cells = A->countOwnedLeaves();

  // Compute the sum of all the leaf cells owned
  unsigned total_leaf_cells;
  unsignedSumAllReduce(number_leaf_cells, total_leaf_cells);

  // Also the number of cells should be equally distributed
  bool passed = number_leaf_cells >= total_leaf_cells/size-1;
  passed &= number_leaf_cells <= (total_leaf_cells/size+2);

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}

// Load balancing (empty partitions)
// Mesh cells on first process (serial) and all other process have empty partitions
// Set cell value data as the cell level then load balance between process
// All process then check if cell data is equal to level
// The cell structure in processor 0 is
//                      rank 0
//                 _______________ 
//                |_|_|   |   |   |
//                |_|_|___|___|___|
//                |   |_|_|   |   |
// structure  ->  |___|_|_|___|___|
//                |   |   |_|_|   |
//                |___|___|_|_|___|
//                |   |   |   |_|_|
//                |___|___|___|_|_|
TEST_CASE("[core][manager][balance][mpi] Load balancing (empty partitions, data exchange)") {
  using Cell2D = Cell<2,2>;
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector<RootCellEntry<Cell2D>> entries { eA };

  // Construction of the tree
  unsigned min_level{2}, max_level{3};
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Split to level 3 in process 0
  if (rank == 1) {
    unsigned counter = 0;
    for (const auto &child : A->getChildCells())
      if (++counter%2 == 0)
        child->split(max_level);
    for (const auto &child : A->getChildCells())
      if (++counter%2 == 0)
        for (const auto &gc : child->getChildCells())
          if (++counter%2 == 0)
            gc->split(max_level);
    A->setToThisProcRecurs();
  } else
    A->setToOtherProcRecurs();

  // Set cell values
  MortonIterator<Cell2D> iterator(tree.getRootCells(), tree.getMaxLevel());
  if (iterator.toOwnedBegin())
    do {
      iterator.getCell()->getCellData().setValue(iterator.getCell()->getLevel());
    } while (iterator.ownedNext());

  // Load balance the tree
  tree.loadBalance();

  // Count number of owned leaf cells
  unsigned number_leaf_cells = A->countOwnedLeaves();

  // Compute the sum of all the leaf cells owned
  unsigned total_leaf_cells;
  unsignedSumAllReduce(number_leaf_cells, total_leaf_cells);

  // Also the number of cells should be equally distributed
  bool passed = number_leaf_cells >= total_leaf_cells/size-1;
  passed &= number_leaf_cells <= (total_leaf_cells/size+2);

  // Check if cell data is valid (value==level)
  if (iterator.toOwnedBegin())
    do {
      passed &= iterator.getCell()->getCellData().getValue() == iterator.getCell()->getLevel();
    } while (iterator.ownedNext());

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}

namespace core::manager::balance {
class TestCellData : public AbstractCellData {
 private:
  unsigned u_value;
  double d_value;
 public:
  TestCellData() : u_value(0), d_value(1.) {};
  ~TestCellData() = default;
 public:
  unsigned getUnsigned() { return u_value; }
  void setUnsigned(unsigned u_value) { this->u_value = u_value; }
  double getDouble() { return d_value; }
  void setDouble(double d_value) { this->d_value = d_value; }
  double getLoad(bool isLeaf, const std::shared_ptr<void> =nullptr) const override {
    return isLeaf ? 1. : 0.; // Can use custom load here for load balancing
  }
  // Conversion as vector of double for data communication
  void fromVectorOfData(const std::vector<double> &buffer) override {
    std::memcpy(&u_value, &buffer[0], sizeof(double));
    d_value = buffer[1];
  }
  std::vector<double> toVectorOfData() const override {
    double u_value_as_d;
    std::memcpy(&u_value_as_d, &u_value, sizeof(double));
    return { u_value_as_d, d_value };
  }
  unsigned getDataSize() const override {
    return 2;
  }
};
}

// Load balancing (empty partitions) with custom cell data
// Same as previous test with custom CellData (TestCellData)
TEST_CASE("[core][manager][balance][mpi] Load balancing (empty partitions, custom data)") {
  using Cell2D = Cell<2, 2, 0, core::manager::balance::TestCellData>;
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector<RootCellEntry<Cell2D>> entries { eA };

  // Construction of the tree
  unsigned min_level{2}, max_level{3};
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Split to level 3 in process 0
  if (rank == 1) {
    unsigned counter = 0;
    for (const auto &child : A->getChildCells())
      if (++counter%2 == 0)
        child->split(max_level);
    for (const auto &child : A->getChildCells())
      if (++counter%2 == 0)
        for (const auto &gc : child->getChildCells())
          if (++counter%2 == 0)
            gc->split(max_level);
    A->setToThisProcRecurs();
  } else
    A->setToOtherProcRecurs();

  // Set cell values
  MortonIterator<Cell2D> iterator(tree.getRootCells(), tree.getMaxLevel());
  if (iterator.toOwnedBegin())
    do {
      core::manager::balance::TestCellData &cellData = iterator.getCell()->getCellData();
      cellData.setUnsigned(iterator.getCell()->getLevel());
      cellData.setDouble(2.*iterator.getCell()->getLevel());
    } while (iterator.ownedNext());

  // Load balance the tree
  tree.loadBalance();

  // Count number of owned leaf cells
  unsigned number_leaf_cells = A->countOwnedLeaves();

  // Compute the sum of all the leaf cells owned
  unsigned total_leaf_cells;
  unsignedSumAllReduce(number_leaf_cells, total_leaf_cells);

  // Also the number of cells should be equally distributed
  bool passed = number_leaf_cells >= total_leaf_cells/size-1;
  passed &= number_leaf_cells <= (total_leaf_cells/size+2);

  // Check if cell data is valid (value==level)
  if (iterator.toOwnedBegin())
    do {
      core::manager::balance::TestCellData &cellData = iterator.getCell()->getCellData();
      passed &= (cellData.getUnsigned() == iterator.getCell()->getLevel());
      passed &= (cellData.getDouble() == 2.*iterator.getCell()->getLevel());
    } while (iterator.ownedNext());

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}

// Small load balancing (one root)
// Mesh at min level 2 then split the first child cell until max level 3
// X show the leaf cells that belong to each process
// Example for size==3
//                      rank 0               rank 1               rank 2
//                 _______________      _______________      _______________ 
//                |       |       |    |   |   |       |    | X | X | X | X |
//                |       |       |    |___|___|       |    |___|___|___|___|
//                |       |       |    | X | X |       |    |   |   | X | X |
// structure  ->  |_______|_______|    |___|___|_______|    |___|___|___|___|
//                |X|X|X|X|   |   |    |       | X | X |    |       |       |
//                |X|X|X|X|___|___|    |       |___|___|    |       |       |
//                |X|X|X|X| X |   |    |       |   | X |    |       |       |
//                |X|X|X|X|___|___|    |_______|___|___|    |_______|_______|
// The loads (nb leaf cells) are [17, 5, 6] and after load balancing the partitions become:
//                      rank 0               rank 1               rank 2
//                 _______________      _______________      _______________ 
//                |       |       |    |   |   |       |    | X | X | X | X |
//                |       |       |    |___|___|       |    |___|___|___|___|
//                |       |       |    |   |   |       |    | X | X | X | X |
// structure  ->  |_______|_______|    |___|___|_______|    |___|___|___|___|
//                |_|_|_|_|   |   |    |X|X|X|X| X |   |    |       |   | X |
//                |X|X|_|_|___|___|    |_|_|X|X|___|___|    |       |___|___|
//                |X|X|X|X|   |   |    |   |   | X | X |    |       |   |   |
//                |X|X|X|X|___|___|    |___|___|___|___|    |_______|___|___|
// The new loads (nb leaf cells) are [17, 5, 6]
TEST_CASE("[core][manager][balance][mpi] Small load balancing (one root)") {
  using Cell2D = Cell<2,2>;
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector<RootCellEntry<Cell2D>> entries { eA };

  // Construction of the tree
  unsigned min_level{2}, max_level{3};
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Split first 4 cells in process 0
  if (rank == 0)
    for (const auto &child : A->getChildCell(0)->getChildCells())
      if (child->belongToOtherProc())
        child->split(max_level);

  // Load balance the tree
  tree.loadBalance();

  // Count number of owned leaf cells
  unsigned number_leaf_cells = A->countOwnedLeaves();

  // Compute the sum of all the leaf cells owned
  unsigned total_leaf_cells;
  unsignedSumAllReduce(number_leaf_cells, total_leaf_cells);

  //std::cout << rank << " " << number_leaf_cells << " " << total_leaf_cells << std::endl;

  // Also the number of cells should be equally distributed
  bool passed = number_leaf_cells >= total_leaf_cells/size-1;
  passed &= number_leaf_cells <= (total_leaf_cells/size+2);

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}

// Big load balancing (one root)
// Mesh at min level 2 then split the first child cell until max level 4
// The loads (nb leaf cells) are [68, 5, 6]
// Note that in this case, the number of cells are not as equal as before
// Hence, the load balancing cell sharing leads to a split of cells in the
// last process partition increasing the nb of leafs in the last process
// That's why the test only verify the sum of leafxsize to verify total area
// Note also that it may lead to extrapolation cells that need to be handled:
// Example for size==3
//                      rank 0               rank 1               rank 2
//                 _______________      _______________      _______________ 
//                |       |       |    |   |   |       |    | X | X | X | X |
//                |       |       |    |___|___|       |    |___|___|___|___|
//                |       |       |    | X | X |       |    |   |   | X | X |
// structure  ->  |_______|_______|    |___|___|_______|    |___|___|___|___|
//                |+|+|+|+|   |   |    |       | X | X |    |       |       |
//                |+|+|+|+|___|___|    |       |___|___|    |       |       |
//                |+|+|+|+|X|X|   |    |       |   | X |    |       |       |
//                |+|+|+|+|X|X|___|    |_______|___|___|    |_______|_______|
// X show the leaf cells that belong to each process
// After load balancing, partition of rank 2 becomes:
//                              rank 2
//                 _______________________________ 
//                |       |       |       |       |
//                |   X   |   X   |   X   |   X   |
//                |       |       |       |       |
// structure  ->  |_______|_______|_______|_______|
//                |       | Y | Y |       |       |
//                |   X   |___|___|   X   |   X   |
//                |       | Y | Y |       |       |
//                |_______|___|___|_______|_______|
//                |   |   |X|X|X|X| Y | Y |       |
//                |___|___|X|X|X|X|___|___|   X   |
//                |   |   |   |X|X| Y | Y |       |
//                |___|___|___|_|X|___|___|_______|
//                |   |   |   |   | X | X |       |
//                |___|___|___|___|___|___|   X   |
//                |   |   |   |   | X | X |       |
//                |___|___|___|___|___|___|_______|
// Cells marked as Y are split to respect the mesh consistency.
// We then try to set their mother cells from data received from rank 1
// An extrapolation is needed to extend received mother cell data to childs Y
TEST_CASE("[core][manager][balance][mpi] Big load balancing (one root)") {
  using Cell2D = Cell<2,2>;
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector<RootCellEntry<Cell2D>> entries { eA };

  // Construction of the tree
  unsigned min_level{2}, max_level{3};
  Tree<Cell2D, MortonIterator<Cell2D, 123>> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Split first 4 cells in process 0
  if (rank == 0) {
    for (const auto &child : A->getChildCell(0)->getChildCells())
      if (child->belongToOtherProc())
        child->split(max_level);
    for (const auto &child : A->getChildCell(0)->getChildCells())
      if (child->belongToOtherProc())
        for (const auto &gc : child->getChildCells())
          if (gc->belongToOtherProc())
            gc->split(max_level);
  }

  // Load balance the tree
  tree.loadBalance();

  // Count number of leaf cells
  MortonIterator<Cell2D, 123> iterator(tree.getRootCells(), tree.getMaxLevel());
  unsigned area = 0, level;
  if (iterator.toOwnedBegin())
    do {
      level = iterator.getCell()->getLevel();
      area += (unsigned)(pow(Cell2D::number_children, max_level-level));
    } while (iterator.ownedNext());

  // Compute the sum of all the leaf cells owned
  unsigned sum_area;
  unsignedSumAllReduce(area, sum_area);

  // Compute the number of cells on each process
  unsigned total_area = (unsigned)(pow(Cell2D::number_children, max_level));

  //std::cout << "P_" << rank << ": " << area << " " << sum_area << " " << total_area << std::endl;

  // Also the number of cells should be equally distributed
  bool passed = sum_area == total_area;

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}

// Small load balancing (two roots)
// Mesh at min level 2 then split the first root cell until max level 3
// The new loads (nb leaf cells) are [40, 29, 11]
TEST_CASE("[core][manager][balance][mpi] Small load balancing (two roots)") {
  using Cell2D = Cell<2,2>;
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);
  auto B = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A}, eB{B};
  eA.setNeighbor(1, B);          // A +x -> B
  eB.setNeighbor(0, A);          // B -x -> A
  std::vector<RootCellEntry<Cell2D>> entries { eA, eB };

  // Construction of the tree
  unsigned min_level{2}, max_level{3};
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Split first 4 cells in process 0
  for (const auto &child : A->getChildCells())
    if (child->belongToThisProc())
      for (const auto &gc : child->getChildCells())
        if (gc->belongToThisProc())
          gc->split(max_level);

  // Load balance the tree
  tree.loadBalance();

  // Count number of owned leaf cells
  unsigned number_leaf_cells = A->countOwnedLeaves() + B->countOwnedLeaves();

  // Compute the sum of all the leaf cells owned
  unsigned total_leaf_cells;
  unsignedSumAllReduce(number_leaf_cells, total_leaf_cells);

  //std::cout << rank << " " << number_leaf_cells << " " << total_leaf_cells << std::endl;

  // Also the number of cells should be equally distributed
  bool passed = number_leaf_cells >= total_leaf_cells/size-1;
  passed &= number_leaf_cells <= (total_leaf_cells/size+2);

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}
