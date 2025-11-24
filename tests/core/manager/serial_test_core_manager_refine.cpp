#include <doctest.h>
#include <test_macros.h>

#include <cmath>
#include <memory>
#include <vector>

#include <core/Cell.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>

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
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector<RootCellEntry<Cell2D>> entries{ eA };

  // Construction of the tree
  unsigned min_level{1}, max_level{3};
  Tree<Cell2D> tree(min_level, max_level);
  tree.createRootCells(entries);

  // Set flags for cells to refine
  A->getChildCell(0)->setToRefine();
  A->getChildCell(3)->setToRefine();

  // Refine the tree
  tree.refine();

  // Count number of leaf cells
  int number_leaf_cells = A->countLeaves();

  // Verify that number of leaf cells is right
  CHECK(number_leaf_cells == 10);

  // Set flags for cells to refine
  A->getChildCell(0)->getChildCell(0)->setToRefine();
  A->getChildCell(0)->getChildCell(1)->setToRefine();

  // Refine the tree
  tree.refine();

  // Count number of leaf cells
  number_leaf_cells = A->countLeaves();

  // Verify that number of leaf cells is right
  CHECK(number_leaf_cells == 19);
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
  auto A = std::make_shared<Cell2D>(nullptr);
  auto B = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A}, eB{B};
  eA.setNeighbor(1, B);          // A +x -> B
  eB.setNeighbor(0, A);          // B -x -> A
  std::vector<RootCellEntry<Cell2D>> entries { eA, eB };

  // Construction of the tree
  unsigned min_level{1}, max_level{3};
  Tree<Cell2D> tree(min_level, max_level);
  tree.createRootCells(entries);

  // Refine the first child cell of root B
  B->getChildCell(0)->split(tree.getMaxLevel());

  // Set flags for cells to refine
  B->getChildCell(0)->getChildCell(0)->setToRefine();
  B->getChildCell(0)->getChildCell(1)->setToRefine();

  // Refine the tree
  tree.refine();

  // Count number of leaf cells
  int number_leaf_cells = A->countLeaves() + B->countLeaves();

  // Verify that number of leaf cells is right
  CHECK(number_leaf_cells == 23);
}
