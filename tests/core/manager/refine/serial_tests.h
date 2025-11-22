#include <cmath>
#include <memory>
#include <vector>
#include <core/Cell.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>
#include <UnitTestRegistry.h>

bool refineTreeOneRootSerial();
bool refineTreeTwoRootsSerial();

void registerCoreManagerRefineSerialTests() {
  UnitTestRegistry::registerSerialTest("Refine tree (one root, serial)", refineTreeOneRootSerial, "core/manager/refine");
  UnitTestRegistry::registerSerialTest("Refine tree (two roots, serial)", refineTreeTwoRootsSerial, "core/manager/refine");
}

// Refine and count number of leaf cells (one root)
//  ________________           _______________           _______________ 
//  |       |       |         |       |   |   |         |       |   |   |
//  |       |   R   |    R    |       |___|___|    R    |       |___|___|
//  |       |       |    E    |       |   |   |    E    |       |   |   |
//  |_______|_______| -> F -> |_______|___|___| -> F -> |_______|___|___|
//  |       |       |    I    |   |   |       |    I    |   |   |       |
//  |   R   |       |    N    |___|___|       |    N    |___|___|       |
//  |       |       |    E    | R | R |       |    E    |_|_|_|_|       |
//  |_______|_______|         |___|___|_______|         |_|_|_|_|_______|
// R show the leaf cells marked to be refined at next refinement
bool refineTreeOneRootSerial() {
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
  bool passed = number_leaf_cells == 10;

  // Set flags for cells to refine
  A->getChildCell(0)->getChildCell(0)->setToRefine();
  A->getChildCell(0)->getChildCell(1)->setToRefine();

  // Refine the tree
  tree.refine();

  // Count number of leaf cells
  number_leaf_cells = A->countLeaves();

  // Verify that number of leaf cells is right
  passed &= number_leaf_cells == 19;
  return passed;
}

// Refine and count number of leaf cells (two roots)
//       root A           root B                      root A           root B
//  ________________  _______________             _______________  _______________ 
//  |       |       ||       |       |           |       |       ||       |       |
//  |       |       ||       |       |     R     |       |       ||       |       |
//  |       |       ||       |       |     E     |       |       ||       |       |
//  |_______|_______||_______|_______|  -> F ->  |_______|_______||_______|_______|
//  |       |       ||   |   |       |     I     |       |   |   ||   |   |   |   |
//  |       |       ||___|___|       |     N     |       |___|___||___|___|___|___|
//  |       |       || R | R |       |     E     |       |   |   ||_|_|_|_|   |   |
//  |_______|_______||___|___|_______|           |_______|___|___||_|_|_|_|___|___|
// R show the leaf cells marked to be refined at next refinement
bool refineTreeTwoRootsSerial() {
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
  bool passed = number_leaf_cells == 23;
  return passed;
}
