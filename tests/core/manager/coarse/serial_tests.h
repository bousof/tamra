#include <cmath>
#include <memory>
#include <vector>
#include <core/Cell.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>
#include <testing/UnitTestRegistry.h>

bool coarsenTreeOneRootSerial();
bool coarsenTreeTwoRootsSerial();

void registerCoreManagerCoarseSerialTests() {
  UnitTestRegistry::registerSerialTest("Coarsen tree (one root, serial)", coarsenTreeOneRootSerial, "core/manager/coarsen");
  UnitTestRegistry::registerSerialTest("Coarsen tree (two roots, serial)", coarsenTreeTwoRootsSerial, "core/manager/coarsen");
}

// Coarsen and count number of leaf cells (one root)
//   _______________________________            _______________________________ 
//  |               |       |       |          |               |       |       |
//  |               |       |       |          |               |       |       |
//  |               |       |       |          |               |       |       |
//  |               |_______|_______|          |               |_______|_______|
//  |               |       | C | C |    C     |               |       |       |
//  |               |       |___|___|    O     |               |       |       |
//  |               |       | C | C |    A     |               |       |       |
//  |_______________|_______|___|___| -> R ->  |_______________|_______|_______|
//  |       |       |       |       |    S     |       |       |               |
//  |       |       |   C   |   C   |    E     |       |       |               |
//  |       |       |       |       |    N     |       |       |               |
//  |_______|_______|_______|_______|          |_______|_______|               |
//  |       |       |       |       |          |       |       |               |
//  |       |       |   C   |   C   |          |       |       |               |
//  |       |       |       |       |          |       |       |               |
//  |_______|_______|_______|_______|          |_______|_______|_______________|
// C show the leaf cells marked to be coarsened at next coarsening
bool coarsenTreeOneRootSerial() {
  using Cell2D = Cell<2,2>;
  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector<RootCellEntry<Cell2D>> entries{eA};

  // Construction of the tree
  int min_level = 1, max_level = 3;
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
  bool passed = number_leaf_cells==10;
  return passed;
}

// Coarsen and count number of leaf cells (one root)
//               root A                           root B
//   _______________________________  _______________________________ 
//  |               |       |       ||       |       |               |
//  |               |       |   C   ||   C   |   C   |               |
//  |               |       |       ||       |       |               |
//  |               |_______|_______||_______|_______|               |
//  |               |       | C | C ||       |       |               |
//  |               |       |___|___||   C   |   C   |               |
//  |               |       | C | C ||       |       |               |
//  |_______________|_______|___|___||_______|_______|_______________|
//  |               |       |       ||               |               |
//  |               |   C   |   C   ||               |               |
//  |               |       |       ||               |               |
//  |               |_______|_______||               |               |
//  |               |       |       ||               |               |
//  |               |   C   |   C   ||               |               |
//  |               |       |       ||               |               |
//  |_______________|_______|_______||_______________|_______________|
// C show the leaf cells marked to be coarsened at next coarsening
//                                   |
//                                   | COARSEN
//               root A              V            root B
//   _______________________________  _______________________________ 
//  |               |       |       ||               |               |
//  |               |       |       ||               |               |
//  |               |       |       ||               |               |
//  |               |_______|_______||               |               |
//  |               |       |       ||               |               |
//  |               |       |       ||               |               |
//  |               |       |       ||               |               |
//  |_______________|_______|_______||_______________|_______________|
//  |               |               ||               |               |
//  |               |               ||               |               |
//  |               |               ||               |               |
//  |               |               ||               |               |
//  |               |               ||               |               |
//  |               |               ||               |               |
//  |               |               ||               |               |
//  |_______________|_______________||_______________|_______________|
bool coarsenTreeTwoRootsSerial() {
  using Cell2D = Cell<2,2>;
  // Create 2 root cells
  auto A = std::make_shared<Cell2D>(nullptr);
  auto B = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A}, eB{B};
  eA.setNeighbor(1, B);          // A +x -> B
  eB.setNeighbor(0, A);          // B -x -> A
  std::vector< RootCellEntry<Cell2D> > entries { eA, eB };

  // Construction of the tree
  int min_level = 1, max_level = 3;
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
  bool passed = number_leaf_cells==11;
  return passed;
}
