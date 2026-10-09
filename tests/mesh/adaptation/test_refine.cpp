#include <doctest.h>

#include <memory>
#include <vector>

#include <tamra/mesh/Cell.h>
#include <tamra/mesh/Tree.h>
#include <tamra/mesh/RootCellEntry.h>


// Refine and count number of leaf cells (one root)
//  ┌───────┬───────┐         ┌───────┬───┬───┐         ┌───────┬───┬───┐
//  │       │       │         │       │   │   │         │       │   │   │
//  │       │   R   │    R    │       ├───┼───┤    R    │       ├───┼───┤
//  │       │       │    E    │       │   │   │    E    │       │   │   │
//  ├───────┼───────┤ ─> F ─> ├───┬───┼───┴───┤ ─> F ─> ├───┬───┼───┼───┤
//  │       │       │    I    │   │   │       │    I    │   │   │   │   │
//  │   R   │       │    N    ├───┼───┤       │    N    ├─┬─┼─┬─┼───┼───┤
//  │       │       │    E    │ R │ R │       │    E    ├─┼─┼─┼─┤   │   │
//  └───────┴───────┘         └───┴───┴───────┘         └─┴─┴─┴─┴───┴───┘
// R show the leaf cells marked to be refined at next refinement
TEST_CASE("[core][manager][refine] Refine tree (one root, serial)") {
  using Cell2D = Cell<2,2>;
  // Create root cell
  auto cell_a = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> entry_a{cell_a};
  std::vector<RootCellEntry<Cell2D>> entries{ entry_a };

  // Construction of the tree
  unsigned min_level{1}, max_level{3};
  Tree<Cell2D> tree(min_level, max_level);
  tree.createRootCells(entries);

  // Set flags for cells to refine
  cell_a->getChildCell(0)->markForRefinement();
  cell_a->getChildCell(3)->markForRefinement();

  // Refine the tree
  tree.refine();

  // Count number of leaf cells
  int leaf_cell_count = cell_a->countLeaves();

  // Verify that number of leaf cells is right
  CHECK(leaf_cell_count == 10);

  // Set flags for cells to refine
  cell_a->getChildCell(0)->getChildCell(0)->markForRefinement();
  cell_a->getChildCell(0)->getChildCell(1)->markForRefinement();

  // Refine the tree
  tree.refine();

  // Count number of leaf cells
  leaf_cell_count = cell_a->countLeaves();

  // Verify that number of leaf cells is right
  CHECK(leaf_cell_count == 19);
}

// Refine and count number of leaf cells (two roots)
//       root A           root B                      root A           root B
//  ┌───────┬───────┐┌───────┬───────┐           ┌───────┬───────┐┌───────┬───────┐
//  │       │       ││       │       │           │       │       ││       │       │
//  │       │       ││       │       │     R     │       │       ││       │       │
//  │       │       ││       │       │     E     │       │       ││       │       │
//  ├───────┼───────┤├───┬───┼───────┤  ─> F ─>  ├───────┼───┬───┤├───┬───┼───┬───┤
//  │       │       ││   │   │       │     I     │       │   │   ││   │   │   │   │
//  │       │       │├───┼───┤       │     N     │       ├───┼───┤├─┬─┼─┬─┼───┼───┤
//  │       │       ││ R │ R │       │     E     │       │   │   │├─┼─┼─┼─┤   │   │
//  └───────┴───────┘└───┴───┴───────┘           └───────┴───┴───┘└─┴─┴─┴─┴───┴───┘
// R show the leaf cells marked to be refined at next refinement
TEST_CASE("[core][manager][refine] Refine tree (two roots, serial)") {
  using Cell2D = Cell<2,2>;
  // Create 2 root cells
  auto cell_a = std::make_shared<Cell2D>(nullptr);
  auto cell_b = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> entry_a{cell_a}, entry_b{cell_b};
  entry_a.setNeighbor(1, cell_b);          // A +x -> B
  entry_b.setNeighbor(0, cell_a);          // B -x -> A
  std::vector<RootCellEntry<Cell2D>> entries { entry_a, entry_b };

  // Construction of the tree
  unsigned min_level{1}, max_level{3};
  Tree<Cell2D> tree(min_level, max_level);
  tree.createRootCells(entries);

  // Refine the first child cell of root B
  cell_b->getChildCell(0)->split(tree.getMaxLevel());

  // Set flags for cells to refine
  cell_b->getChildCell(0)->getChildCell(0)->markForRefinement();
  cell_b->getChildCell(0)->getChildCell(1)->markForRefinement();

  // Refine the tree
  tree.refine();

  // Count number of leaf cells
  int leaf_cell_count = cell_a->countLeaves() + cell_b->countLeaves();

  // Verify that number of leaf cells is right
  CHECK(leaf_cell_count == 23);
}
