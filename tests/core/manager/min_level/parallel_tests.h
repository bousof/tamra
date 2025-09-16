#include <mpi.h>

#include <core/Cell.h>
#include <core/RootCellEntry.h>
#include <core/Tree.h>
#include <memory>
#include <parallel/allreduce.h>
#include <testing/UnitTestRegistry.h>
#include <vector>

bool meshTreeToMinLevelOneRootParallel(int rank, int size);
bool meshTreeToMinLevelTwoRootsParallel(int rank, int size);
bool meshTreeToMinLevelOneRootPerProcParallel(int rank, int size);

void registerCoreManagerMinLevelParallelTests() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  UnitTestRegistry::label = "P_" + std::to_string(rank) + ": ";
  UnitTestRegistry::registerParallelTest("Meshing at min level (one root, parallel)", [=]() { return meshTreeToMinLevelOneRootParallel(rank, size); }, "core/manager/min_level");
  UnitTestRegistry::registerParallelTest("Meshing at min level (two roots, parallel)", [=]() { return meshTreeToMinLevelTwoRootsParallel(rank, size); }, "core/manager/min_level");
  UnitTestRegistry::registerParallelTest("Mesh at min level (1 root per process)", [=]() { return meshTreeToMinLevelOneRootPerProcParallel(rank, size); }, "core/manager/min_level");
}

// Mesh at min level and count number of leaf cells belong to this process
// Example for size==3
//                      rank 0               rank 1               rank 2
//                 _______________      _______________      _______________ 
//                |       |       |    |   |   |       |    | X | X | X | X |
//                |       |       |    |___|___|       |    |___|___|___|___|
//                |       |       |    | X | X |       |    |   |   | X | X |
// structure  ->  |_______|_______|    |___|___|_______|    |___|___|___|___|
//                | X | X |   |   |    |       | X | X |    |       |       |
//                |___|___|___|___|    |       |___|___|    |       |       |
//                | X | X | X |   |    |       |   | X |    |       |       |
//                |___|___|___|___|    |_______|___|___|    |_______|_______|
// X show the leaf cells that belong to each process
bool meshTreeToMinLevelOneRootParallel(int rank, int size) {
  using Cell2D = Cell<2,2>;
  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector<RootCellEntry<Cell2D>> entries { eA };

  // Construction of the tree
  int min_level = 2, max_level = 3;
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Count number of owned leaf cells
  int number_leaf_cells = A->countOwnedLeaves();

  // Compute the number of cells on each process
  int total_cells_min_level = (int)(pow(Cell2D::number_children, min_level));

  // The sum of all the leaf cells owned should be total_cells_min_level
  unsigned total_leaf_cells;
  unsignedSumAllReduce(number_leaf_cells, total_leaf_cells);
  bool passed = total_leaf_cells == total_cells_min_level;

  // Also the number of cells should be equally distributed
  passed &= number_leaf_cells >= total_cells_min_level/size;
  passed &= number_leaf_cells <= (total_cells_min_level/size+1);

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}

// Mesh at min level and count number of leaf cells belong to this process (2 root cells)
// Example for size==3
//                  root A           root B
//              _______________  _______________
//             |   |   |   |   ||   |   |   |   |
//             |___|___|___|___||___|___|___|___|
//             | X | X |   |   ||   |   |   |   |
// rank 0  ->  |___|___|___|___||___|___|___|___|
//             | X | X | X | X ||   |   |   |   |
//             |___|___|___|___||___|___|___|___|
//             | X | X | X | X ||   |   |   |   |
//             |___|___|___|___||___|___|___|___|
//              _______________  _______________
//             | X | X | X | X ||   |   |   |   |
//             |___|___|___|___||___|___|___|___|
//             |   |   | X | X ||   |   |   |   |
// rank 1  ->  |___|___|___|___||___|___|___|___|
//             |   |   |   |   || X | X |   |   |
//             |___|___|___|___||___|___|___|___|
//             |   |   |   |   || X | X | X |   |
//             |___|___|___|___||___|___|___|___|
//              _______________  _______________
//             |   |   |   |   || X | X | X | X |
//             |___|___|___|___||___|___|___|___|
//             |   |   |   |   || X | X | X | X |
// rank 2  ->  |___|___|___|___||___|___|___|___|
//             |   |   |   |   ||   |   | X | X |
//             |___|___|___|___||___|___|___|___|
//             |   |   |   |   ||   |   |   | X |
//             |___|___|___|___||___|___|___|___|
// X show the leaf cells that belong to each process
bool meshTreeToMinLevelTwoRootsParallel(int rank, int size) {
  using Cell2D = Cell<2,2>;
  // Create 2 root cells
  auto A = std::make_shared<Cell2D>(nullptr);
  auto B = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A}, eB{B};
  eA.setNeighbor(1, B);          // A +x -> B
  eB.setNeighbor(0, A);          // B -x -> A
  std::vector<RootCellEntry<Cell2D>> entries { eA, eB };

  // Construction of the tree
  int min_level = 2, max_level = 3;
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Count number of owned leaf cells
  int number_leaf_cells = A->countOwnedLeaves() + B->countOwnedLeaves();

  // Compute the number of cells on each process
  int total_cells_min_level = tree.getRootCells().size() * (int)(pow(Cell2D::number_children, min_level));

  // The sum of all the leaf cells owned should be total_cells_min_level
  unsigned total_leaf_cells;
  unsignedSumAllReduce(number_leaf_cells, total_leaf_cells);
  bool passed = total_leaf_cells == total_cells_min_level;

  // Also the number of cells should be equally distributed
  passed &= number_leaf_cells >= total_cells_min_level/size;
  passed &= number_leaf_cells <= (total_cells_min_level/size+1);

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}

// Mesh at min level (1 root per process)
// Example for size==3
//              root 0   root1         root (size-1)
//              _______  _______         _______
//             | 0 | 0 ||   |   |       |   |   |
// structure ->|___|___||___|___| . . . |___|___|
//             | 0 | 0 ||   |   |       |   |   |
//             |___|___||___|___|       |___|___|
// root i should be the partition of process i
bool meshTreeToMinLevelOneRootPerProcParallel(int rank, int size) {
  using Cell2D = Cell<2,2>;
  // Create 2 root cells
  auto A = std::make_shared<Cell2D>(nullptr);
  auto B = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A}, eB{B};
  eA.setNeighbor(1, B);          // A +x -> B
  eB.setNeighbor(0, A);          // B -x -> A
  std::vector<RootCellEntry<Cell2D>> entries { eA, eB };

  // Construction of the tree
  int min_level = 2, max_level = 3;
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Count number of owned leaf cells
  int number_leaf_cells = A->countOwnedLeaves() + B->countOwnedLeaves();

  // Compute the number of cells on each process
  unsigned total_cells_min_level = tree.getRootCells().size() * (int)(pow(Cell2D::number_children, min_level));

  // The sum of all the leaf cells owned should be total_cells_min_level
  unsigned total_leaf_cells;
  unsignedSumAllReduce(number_leaf_cells, total_leaf_cells);
  bool passed = total_leaf_cells == total_cells_min_level;

  // Also the number of cells should be equally distributed
  passed &= number_leaf_cells >= total_cells_min_level/size;
  passed &= number_leaf_cells <= (total_cells_min_level/size+1);

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}
