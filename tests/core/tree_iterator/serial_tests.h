#include <cmath>
#include <memory>
#include <vector>
#include <core/Cell.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>
#include <testing/UnitTestRegistry.h>

bool countLeafOneRoot();
bool countLeafTwoRoots();

void registerCoreTreeIteratorSerialTests() {
  UnitTestRegistry::registerSerialTest("Iterator leaf cell counting (1 root cell)", countLeafOneRoot, "core/tree_iterator");
  UnitTestRegistry::registerSerialTest("Iterator leaf cell counting (2 root cells)", countLeafTwoRoots, "core/tree_iterator");
}

// Iterator leaf cell counting (1 root cell)
//                 _______________
//                |   |   |       |
//                |___|___|       |
//                |   |   |       |
// structure  ->  |___|___|_______|
//                |       |   |   |
//                |       |___|___|
//                |       |   |   |
//                |_______|___|___|
// Number of leaf cells should be 10
bool countLeafOneRoot() {
  using Cell2D = Cell<2,2>;
  // Create 4 root cells
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector< RootCellEntry<Cell2D> > entries { eA };

  // Construction of the tree
  int min_level = 1, max_level = 2;
  Tree<Cell2D> tree(min_level, max_level);
  tree.createRootCells(entries);

  //Splitting some cells
  A->getChildCell(1)->split(max_level);
  A->getChildCell(2)->split(max_level);

  // Count number of leaf cells
  int number_leaf_cells = A->countLeaves();

  bool passed = number_leaf_cells==10;
  return passed;
}

// Iterator leaf cell counting (2 root cells)
//                      root A           root B
//                 _______________  _______________
//                |       |       ||       |   |   |
//                |       |       ||       |___|___|
//                |       |       ||       |   |   |
// structure  ->  |_______|_______||_______|___|___|
//                |       |   |   ||       |       |
//                |       |___|___||       |       |
//                |       |   |   ||       |       |
//                |_______|___|___||_______|_______|
// Number of leaf cells should be 10
bool countLeafTwoRoots() {
  using Cell2D = Cell<2,2>;
  // Create 4 root cells
  auto A = std::make_shared<Cell2D>(nullptr);
  auto B = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A}, eB{B};
  eA.setNeighbor(1, B);          // A +x -> B
  eB.setNeighbor(0, A);          // B -x -> A
  std::vector< RootCellEntry<Cell2D> > entries { eA, eB };

  // Construction of the tree
  int min_level = 1, max_level = 2;
  Tree<Cell2D> tree(min_level, max_level);
  tree.createRootCells(entries);

  //Splitting some cells
  A->getChildCell(1)->split(max_level);
  B->getChildCell(3)->split(max_level);

  // Count number of leaf cells
  int number_leaf_cells = A->countLeaves() + B->countLeaves();

  bool passed = number_leaf_cells==14;
  return passed;
}
