#include <doctest.h>
#include <test_macros.h>

#include <cmath>
#include <memory>
#include <vector>

#include <core/Cell.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>

// Coarsen and count number of leaf cells (one root)
//  ┌───────────────┬───────┬───────┐          ┌───────────────┬───────┬───────┐
//  │               │       │       │          │               │       │       │
//  │               │       │       │          │               │       │       │
//  │               │       │       │          │               │       │       │
//  │               ├───────┼───┬───┤          │               ├───────┼───────┤
//  │               │       │ C │ C │    C     │               │       │       │
//  │               │       ├───┼───┤    O     │               │       │       │
//  │               │       │ C │ C │    A     │               │       │       │
//  ├───────┬───────┼───────┼───┴───┤ ─> R ─>  ├───────┬───────┼───────┴───────┤
//  │       │       │       │       │    S     │       │       │               │
//  │       │       │   C   │   C   │    E     │       │       │               │
//  │       │       │       │       │    N     │       │       │               │
//  ├───────┼───────┼───────┼───────┤          ├───────┼───────┤               │
//  │       │       │       │       │          │       │       │               │
//  │       │       │   C   │   C   │          │       │       │               │
//  │       │       │       │       │          │       │       │               │
//  └───────┴───────┴───────┴───────┘          └───────┴───────┴───────────────┘
// C show the leaf cells marked to be coarsened at next coarsening
TEST_CASE("[core][manager][coarsen] Coarsen tree (one root, serial)") {
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

  // Split cells
  A->getChildCell(0)->split(tree.getMaxLevel());
  A->getChildCell(1)->split(tree.getMaxLevel());
  A->getChildCell(3)->split(tree.getMaxLevel());
  A->getChildCell(3)->getChildCell(1)->split(tree.getMaxLevel());

  // Set flags for cells to coarse
  A->getChildCell(1)->setToCoarseRecurs();
  A->getChildCell(3)->getChildCell(1)->setToCoarseRecurs();

  // Coarsen the tree
  tree.coarsen();

  // Count number of owned leaf cells
  int number_leaf_cells = A->countOwnedLeaves();

  // Verify that number of leaf cells is right
  CHECK(number_leaf_cells == 10);
}

// Coarsen and count number of leaf cells (one root)
//               root A                           root B
//  ┌───────────────┬───────┬───────┐┌───────┬───────┬───────────────┐
//  │               │       │       ││       │       │               │
//  │               │       │   C   ││   C   │   C   │               │
//  │               │       │       ││       │       │               │
//  │               ├───────┼───┬───┤├───────┼───────┤               │
//  │               │       │ C │ C ││       │       │               │
//  │               │       ├───┼───┤│   C   │   C   │               │
//  │               │       │ C │ C ││       │       │               │
//  ├───────────────┼───────┼───┴───┤├───────┴───────┼───────────────┤
//  │               │       │       ││               │               │
//  │               │   C   │   C   ││               │               │
//  │               │       │       ││               │               │
//  │               ├───────┼───────┤│               │               │
//  │               │       │       ││               │               │
//  │               │   C   │   C   ││               │               │
//  │               │       │       ││               │               │
//  └───────────────┴───────┴───────┘└───────────────┴───────────────┘
// C show the leaf cells marked to be coarsened at next coarsening
//                                   │
//                                   │ COARSEN
//               root A              V            root B
//  ┌───────────────┬───────┬───────┐┌───────────────┬───────────────┐
//  │               │       │       ││               │               │
//  │               │       │       ││               │               │
//  │               │       │       ││               │               │
//  │               ├───────┼───────┤│               │               │
//  │               │       │       ││               │               │
//  │               │       │       ││               │               │
//  │               │       │       ││               │               │
//  ├───────────────┼───────┴───────┤├───────────────┼───────────────┤
//  │               │               ││               │               │
//  │               │               ││               │               │
//  │               │               ││               │               │
//  │               │               ││               │               │
//  │               │               ││               │               │
//  │               │               ││               │               │
//  │               │               ││               │               │
//  └───────────────┴───────────────┘└───────────────┴───────────────┘
TEST_CASE("[core][manager][coarsen] Coarsen tree (two roots, serial)") {
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

  // Split cells
  A->getChildCell(1)->split(tree.getMaxLevel());
  A->getChildCell(3)->split(tree.getMaxLevel());
  B->getChildCell(2)->split(tree.getMaxLevel());
  A->getChildCell(3)->getChildCell(1)->split(tree.getMaxLevel());

  // Set flags for cells to coarse
  A->getChildCell(1)->setToCoarseRecurs();
  A->getChildCell(3)->getChildCell(1)->setToCoarseRecurs();
  B->getChildCell(2)->setToCoarseRecurs();

  // Coarse the tree
  tree.coarsen();

  // Count number of owned leaf cells
  int number_leaf_cells = A->countOwnedLeaves() + B->countOwnedLeaves();

  // Verify that number of leaf cells is right
  CHECK(number_leaf_cells == 11);
}
