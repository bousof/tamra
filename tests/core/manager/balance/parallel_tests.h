#include <mpi.h>

#include <core/Cell.h>
#include <core/RootCellEntry.h>
#include <core/Tree.h>
#include <memory>
#include <parallel/allreduce.h>
#include <testing/UnitTestRegistry.h>
#include <vector>

bool balanceEmptyPartitionsParallel(int rank, int size);
bool balanceEmptyPartitionsDataParallel(int rank, int size);
bool balanceSmallOneRootParallel(int rank, int size);
bool balanceBigOneRootParallel(int rank, int size);
bool balanceSmallTwoRootsParallel(int rank, int size);

void registerCoreManagerBalanceParallelTests() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  UnitTestRegistry::label = "P_" + std::to_string(rank) + ": ";
  UnitTestRegistry::registerParallelTest("Load balancing (empty partitions)", [=]() { return balanceEmptyPartitionsParallel(rank, size); }, "core/manager/balance");
  UnitTestRegistry::registerParallelTest("Load balancing (empty partitions, data exchange)", [=]() { return balanceEmptyPartitionsDataParallel(rank, size); }, "core/manager/balance");
  UnitTestRegistry::registerParallelTest("Small load balancing (one root)", [=]() { return balanceSmallOneRootParallel(rank, size); }, "core/manager/balance");
  UnitTestRegistry::registerParallelTest("Big load balancing (one root)", [=]() { return balanceBigOneRootParallel(rank, size); }, "core/manager/balance");
  UnitTestRegistry::registerParallelTest("Small load balancing (two roots)", [=]() { return balanceSmallTwoRootsParallel(rank, size); }, "core/manager/balance");
}

// Load balancing (empty partitions)
// Mesh at level 3 on first process (serial) and all other process have empty opartitions
// Then load balance between process
bool balanceEmptyPartitionsParallel(int rank, int size) {
  using Cell2D = Cell<2,2>;
  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector< RootCellEntry<Cell2D> > entries { eA };

  // Construction of the tree
  int min_level = 2, max_level = 3;
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Split to level 3 in process 0
  if (rank==1) {
    for (const auto &child: A->getChildCells())
      child->split(max_level);
    for (const auto &child: A->getChildCells())
      for (const auto &gc: child->getChildCells())
        gc->split(max_level);
    A->setToThisProcRecurs();
  } else
    A->setToOtherProcRecurs();

  // Load balance the tree
  tree.loadBalance();

  // Count number of leaf cells
  TreeIterator<Cell2D> iterator(tree.getRootCells(), tree.getMaxLevel());
  int number_leaf_cells = 1;
  iterator.toOwnedBegin();
  while (iterator.ownedNext())
    ++number_leaf_cells;

  // Compute the sum of all the leaf cells owned
  unsigned total_leaf_cells;
  unsignedSumAllReduce(number_leaf_cells, total_leaf_cells);

  // Also the number of cells should be equally distributed
  bool passed = number_leaf_cells >= total_leaf_cells/size-1;
  passed &= number_leaf_cells <= (total_leaf_cells/size+2);

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}

// Load balancing (empty partitions)
// Mesh cells on first process (serial) and all other process have empty opartitions
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
bool balanceEmptyPartitionsDataParallel(int rank, int size) {
  using Cell2D = Cell<2,2>;
  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector< RootCellEntry<Cell2D> > entries { eA };

  // Construction of the tree
  int min_level = 2, max_level = 3;
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Split to level 3 in process 0
  if (rank==1) {
    unsigned counter = 0;
    for (const auto &child: A->getChildCells())
      if (++counter%2==0)
        child->split(max_level);
    for (const auto &child: A->getChildCells())
      if (++counter%2==0)
        for (const auto &gc: child->getChildCells())
          if (++counter%2==0)
            gc->split(max_level);
    A->setToThisProcRecurs();
  } else
    A->setToOtherProcRecurs();

  // Set cell values
  TreeIterator<Cell2D> iterator(tree.getRootCells(), tree.getMaxLevel());
  if (iterator.toOwnedBegin())
    do {
      iterator.getCell()->setCellValue(iterator.getCell()->getLevel());
    } while (iterator.ownedNext());

  // Load balance the tree
  tree.loadBalance();

  // Count number of leaf cells
  int number_leaf_cells = 1;
  iterator.toOwnedBegin();
  while (iterator.ownedNext())
    ++number_leaf_cells;

  // Compute the sum of all the leaf cells owned
  unsigned total_leaf_cells;
  unsignedSumAllReduce(number_leaf_cells, total_leaf_cells);

  // Also the number of cells should be equally distributed
  bool passed = number_leaf_cells >= total_leaf_cells/size-1;
  passed &= number_leaf_cells <= (total_leaf_cells/size+2);

  // Check if cell data is valid (value==level)
  if (iterator.toOwnedBegin())
    do {
      passed &= iterator.getCell()->getCellValue() == iterator.getCell()->getLevel();
    } while (iterator.ownedNext());

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
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
bool balanceSmallOneRootParallel(int rank, int size) {
  using Cell2D = Cell<2,2>;
  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector< RootCellEntry<Cell2D> > entries { eA };

  // Construction of the tree
  int min_level = 2, max_level = 3;
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Split first 4 cells in process 0
  if (rank==0)
    for (const auto &child: A->getChildCell(0)->getChildCells())
      child->split(max_level);

  // Load balance the tree
  tree.loadBalance();

  // Count number of leaf cells
  TreeIterator<Cell2D> iterator(tree.getRootCells(), tree.getMaxLevel());
  int number_leaf_cells = 1;
  iterator.toOwnedBegin();
  while (iterator.ownedNext())
    ++number_leaf_cells;

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
  return all_passed;
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
bool balanceBigOneRootParallel(int rank, int size) {
  using Cell2D = Cell<2,2>;
  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector< RootCellEntry<Cell2D> > entries { eA };

  // Construction of the tree
  int min_level = 2, max_level = 4;
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Split first 4 cells in process 0
  if (rank==0) {
    for (const auto &child: A->getChildCell(0)->getChildCells())
      child->split(max_level);
    for (const auto &child: A->getChildCell(0)->getChildCells())
      for (const auto &gc: child->getChildCells())
        gc->split(max_level);
  }

  // Load balance the tree
  tree.loadBalance();

  // Count number of leaf cells
  TreeIterator<Cell2D> iterator(tree.getRootCells(), tree.getMaxLevel());
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
  return all_passed;
}

// Small load balancing (two roots)
// Mesh at min level 2 then split the first root cell until max level 3
// The new loads (nb leaf cells) are [40, 29, 11]
bool balanceSmallTwoRootsParallel(int rank, int size) {
  using Cell2D = Cell<2,2>;
  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);
  auto B = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A}, eB{B};
  eA.setNeighbor(1, B);          // A +x -> B
  eB.setNeighbor(0, A);          // B -x -> A
  std::vector< RootCellEntry<Cell2D> > entries { eA, eB };

  // Construction of the tree
  int min_level = 2, max_level = 3;
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Split first 4 cells in process 0
  for (const auto &child: A->getChildCells())
    if (child->belongToThisProc())
      for (const auto &gc: child->getChildCells())
        if (gc->belongToThisProc())
          gc->split(max_level);

  // Load balance the tree
  tree.loadBalance();

  // Count number of leaf cells
  TreeIterator<Cell2D> iterator(tree.getRootCells(), tree.getMaxLevel());
  int number_leaf_cells = 1;
  iterator.toOwnedBegin();
  while (iterator.ownedNext())
    ++number_leaf_cells;

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
  return all_passed;
}
