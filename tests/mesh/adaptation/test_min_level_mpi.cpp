#include <doctest.h>

#include <memory>
#include <vector>

#include <tamra/mesh/Cell.h>
#include <tamra/mesh/RootCellEntry.h>
#include <tamra/mesh/Tree.h>
#include <tamra/parallel/allreduce.h>
#include <tamra/parallel/wrapper.h>


// Mesh at min level and count number of leaf cells belong to this process
// Example for size==3
//                      rank 0               rank 1               rank 2
//                ┌───────┬───────┐    ┌───┬───┬───────┐    ┌───┬───┬───┬───┐
//                │       │       │    │   │   │       │    │ X │ X │ X │ X │
//                │       │       │    ├───┼───┤       │    ├───┼───┼───┼───┤
//                │       │       │    │ X │ X │       │    │   │   │ X │ X │
// structure  ->  ├───┬───┼───┬───┤    ├───┴───┼───┬───┤    ├───┴───┼───┴───┤
//                │ X │ X │   │   │    │       │ X │ X │    │       │       │
//                ├───┼───┼───┼───┤    │       ├───┼───┤    │       │       │
//                │ X │ X │ X │   │    │       │   │ X │    │       │       │
//                └───┴───┴───┴───┘    └───────┴───┴───┘    └───────┴───────┘
// X show the leaf cells that belong to each process
TEST_CASE("[core][manager][min_level][mpi] Meshing at min level (one root, parallel)") {
  using Cell2D = Cell<2,2>;
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  // Create root cell
  auto cell_a = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> entry_a{cell_a};
  std::vector<RootCellEntry<Cell2D>> entries { entry_a };

  // Construction of the tree
  unsigned min_level{2}, max_level{3};
  Tree<Cell2D, MortonIterator<Cell2D, 213>> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Count number of owned leaf cells
  unsigned leaf_cell_count = cell_a->countOwnedLeaves();

  // Compute the number of cells on each process
  unsigned total_cells_min_level = (unsigned)(pow(Cell2D::child_count, min_level));

  // The sum of all the leaf cells owned should be total_cells_min_level
  unsigned total_leaf_cells;
  scalar_sum_allreduce<unsigned>(leaf_cell_count, total_leaf_cells);
  bool passed = total_leaf_cells == total_cells_min_level;

  // Also the number of cells should be equally distributed
  passed &= leaf_cell_count >= total_cells_min_level/size;
  passed &= leaf_cell_count <= (total_cells_min_level/size+1);

  // Test should pass on all processes
  bool all_passed;
  bool_and_allreduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}

// Mesh at min level and count number of leaf cells belong to this process (2 root cells)
// Example for size==3
//                  root A           root B
//             ┌───┬───┬───┬───┐┌───┬───┬───┬───┐
//             │   │   │   │   ││   │   │   │   │
//             ├───┼───┼───┼───┤├───┼───┼───┼───┤
//             │ X │ X │   │   ││   │   │   │   │
// rank 0  ->  ├───┼───┼───┼───┤├───┼───┼───┼───┤
//             │ X │ X │ X │ X ││   │   │   │   │
//             ├───┼───┼───┼───┤├───┼───┼───┼───┤
//             │ X │ X │ X │ X ││   │   │   │   │
//             └───┴───┴───┴───┘└───┴───┴───┴───┘
//             ┌───┬───┬───┬───┐┌───┬───┬───┬───┐
//             │ X │ X │ X │ X ││   │   │   │   │
//             ├───┼───┼───┼───┤├───┼───┼───┼───┤
//             │   │   │ X │ X ││   │   │   │   │
// rank 1  ->  ├───┼───┼───┼───┤├───┼───┼───┼───┤
//             │   │   │   │   ││ X │ X │   │   │
//             ├───┼───┼───┼───┤├───┼───┼───┼───┤
//             │   │   │   │   ││ X │ X │ X │   │
//             └───┴───┴───┴───┘└───┴───┴───┴───┘
//             ┌───┬───┬───┬───┐┌───┬───┬───┬───┐
//             │   │   │   │   ││ X │ X │ X │ X │
//             ├───┼───┼───┼───┤├───┼───┼───┼───┤
//             │   │   │   │   ││ X │ X │ X │ X │
// rank 2  ->  ├───┼───┼───┼───┤├───┼───┼───┼───┤
//             │   │   │   │   ││   │   │ X │ X │
//             ├───┼───┼───┼───┤├───┼───┼───┼───┤
//             │   │   │   │   ││   │   │   │ X │
//             └───┴───┴───┴───┘└───┴───┴───┴───┘
// X show the leaf cells that belong to each process
TEST_CASE("[core][manager][min_level][mpi] Meshing at min level (two roots, parallel)") {
  using Cell2D = Cell<2,2>;
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  // Create 2 root cells
  auto cell_a = std::make_shared<Cell2D>(nullptr);
  auto cell_b = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> entry_a{cell_a}, entry_b{cell_b};
  entry_a.setNeighbor(1, cell_b);          // A +x -> B
  entry_b.setNeighbor(0, cell_a);          // B -x -> A
  std::vector<RootCellEntry<Cell2D>> entries { entry_a, entry_b };

  // Construction of the tree
  unsigned min_level{2}, max_level{3};
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Count number of owned leaf cells
  unsigned leaf_cell_count = cell_a->countOwnedLeaves() + cell_b->countOwnedLeaves();

  // Compute the number of cells on each process
  unsigned total_cells_min_level = tree.getRootCells().size() * (unsigned)(pow(Cell2D::child_count, min_level));

  // The sum of all the leaf cells owned should be total_cells_min_level
  unsigned total_leaf_cells;
  scalar_sum_allreduce<unsigned>(leaf_cell_count, total_leaf_cells);
  bool passed = total_leaf_cells == total_cells_min_level;

  // Also the number of cells should be equally distributed
  passed &= leaf_cell_count >= total_cells_min_level/size;
  passed &= leaf_cell_count <= (total_cells_min_level/size+1);

  // Test should pass on all processes
  bool all_passed;
  bool_and_allreduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}

// Mesh at min level (1 root per process)
// Example for size==3
//              root 0   root1         root (size-1)
//             ┌───┬───┐┌───┬───┐       ┌───┬───┐
//             │ 0 │ 0 ││   │   │       │   │   │
// structure ->├───┼───┤├───┼───┤ . . . ├───┼───┤
//             │ 0 │ 0 ││   │   │       │   │   │
//             └───┴───┘└───┴───┘       └───┴───┘
// root i should be the partition of process i
TEST_CASE("[core][manager][min_level][mpi] Mesh at min level (1 root per process)") {
  using Cell2D = Cell<2,2>;
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  // Create 2 root cells
  auto cell_a = std::make_shared<Cell2D>(nullptr);
  auto cell_b = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> entry_a{cell_a}, entry_b{cell_b};
  entry_a.setNeighbor(1, cell_b);          // A +x -> B
  entry_b.setNeighbor(0, cell_a);          // B -x -> A
  std::vector<RootCellEntry<Cell2D>> entries { entry_a, entry_b };

  // Construction of the tree
  unsigned min_level{2}, max_level{3};
  Tree<Cell2D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Count number of owned leaf cells
  unsigned leaf_cell_count = cell_a->countOwnedLeaves() + cell_b->countOwnedLeaves();

  // Compute the number of cells on each process
  unsigned total_cells_min_level = tree.getRootCells().size() * (unsigned)(pow(Cell2D::child_count, min_level));

  // The sum of all the leaf cells owned should be total_cells_min_level
  unsigned total_leaf_cells;
  scalar_sum_allreduce<unsigned>(leaf_cell_count, total_leaf_cells);
  bool passed = total_leaf_cells == total_cells_min_level;

  // Also the number of cells should be equally distributed
  passed &= leaf_cell_count >= total_cells_min_level/size;
  passed &= leaf_cell_count <= (total_cells_min_level/size+1);

  // Test should pass on all processes
  bool all_passed;
  bool_and_allreduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}
