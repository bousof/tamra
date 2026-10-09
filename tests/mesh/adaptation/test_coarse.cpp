#include <doctest.h>

#include <memory>
#include <vector>

#include <tamra/mesh/Cell.h>
#include <tamra/mesh/Tree.h>
#include <tamra/mesh/RootCellEntry.h>


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
  auto cell_a = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> entry_a{cell_a};
  std::vector<RootCellEntry<Cell2D>> entries{ entry_a };

  // Construction of the tree
  unsigned min_level{1}, max_level{3};
  Tree<Cell2D> tree(min_level, max_level);
  tree.createRootCells(entries);

  // Split cells
  cell_a->getChildCell(0)->split(tree.getMaxLevel());
  cell_a->getChildCell(1)->split(tree.getMaxLevel());
  cell_a->getChildCell(3)->split(tree.getMaxLevel());
  cell_a->getChildCell(3)->getChildCell(1)->split(tree.getMaxLevel());

  // Set flags for cells to coarsen
  cell_a->getChildCell(1)->markForCoarseningRecurs();
  cell_a->getChildCell(3)->getChildCell(1)->markForCoarseningRecurs();

  // Coarsen the tree
  tree.coarsen();

  // Count number of owned leaf cells
  int leaf_cell_count = cell_a->countOwnedLeaves();

  // Verify that number of leaf cells is right
  CHECK(leaf_cell_count == 10);
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

  // Split cells
  cell_a->getChildCell(1)->split(tree.getMaxLevel());
  cell_a->getChildCell(3)->split(tree.getMaxLevel());
  cell_b->getChildCell(2)->split(tree.getMaxLevel());
  cell_a->getChildCell(3)->getChildCell(1)->split(tree.getMaxLevel());

  // Set flags for cells to coarsen
  cell_a->getChildCell(1)->markForCoarseningRecurs();
  cell_a->getChildCell(3)->getChildCell(1)->markForCoarseningRecurs();
  cell_b->getChildCell(2)->markForCoarseningRecurs();

  // Coarse the tree
  tree.coarsen();

  // Count number of owned leaf cells
  int leaf_cell_count = cell_a->countOwnedLeaves() + cell_b->countOwnedLeaves();

  // Verify that number of leaf cells is right
  CHECK(leaf_cell_count == 11);
}
