#include <cmath>
#include <memory>
#include <vector>
#include <core/Cell.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>
#include <UnitTestRegistry.h>

bool meshTreeToMinLevelOneRootSerial();
bool meshTreeToMinLevelTwoRootsSerial();

void registerCoreManagerMinLevelSerialTests() {
  UnitTestRegistry::registerSerialTest("Mesh at min level (one root, serial)", meshTreeToMinLevelOneRootSerial, "core/manager/min_level");
  UnitTestRegistry::registerSerialTest("Mesh at min level (two roots, serial)", meshTreeToMinLevelTwoRootsSerial, "core/manager/min_level");
}

// Mesh at min level and count number of leaf cells (one root)
bool meshTreeToMinLevelOneRootSerial() {
  using Cell2D = Cell<2,3>;
  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector<RootCellEntry<Cell2D>> entries { eA };

  // Construction of the tree
  int min_level = 2, max_level = 3;
  Tree<Cell2D> tree(min_level, max_level);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Count number of leaf cells
  int number_leaf_cells = A->countLeaves();

  bool passed = number_leaf_cells == (int)(pow(Cell2D::number_children, min_level));
  return passed;
}

// Mesh at min level and count number of leaf cells (two roots)
bool meshTreeToMinLevelTwoRootsSerial() {
  using Cell2D = Cell<2,3>;
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
  Tree<Cell2D> tree(min_level, max_level);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Count number of leaf cells
  int number_leaf_cells = A->countLeaves() + B->countLeaves();

  bool passed = number_leaf_cells == tree.getRootCells().size() * (int)(pow(Cell2D::number_children, min_level));
  return passed;
}
