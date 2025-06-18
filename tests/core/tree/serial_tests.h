#include <cmath>
#include <memory>
#include <vector>
#include <core/Cell.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>
#include <testing/UnitTestRegistry.h>

bool testRootCellEntry1D();
bool testTree1D();
bool testTree2D();
bool countLeafOneRoot();

void registerCoreTreeSerialTests() {
  UnitTestRegistry::registerSerialTest("RootCellEntry basic wiring (1D)", testRootCellEntry1D, "core/tree");
  UnitTestRegistry::registerSerialTest("Simple Tree (1D)", testTree1D, "core/tree");
  UnitTestRegistry::registerSerialTest("Simple Tree (2D)", testTree2D, "core/tree");
}

// RootCellEntry basic wiring (1D)
//
//                |   A   |   B   |
// structure  ->  |___|___|___|___|
bool testRootCellEntry1D() {
  using Cell1D = Cell<2>;
  // Create 2 root cells
  auto A = std::make_shared<Cell1D>(nullptr);
  auto B = std::make_shared<Cell1D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell1D> eA{A}, eB{B};
  eA.setNeighbor(1, B);   // A(+x) = B
  eB.setNeighbor(0, A);   // B(‑x) = A

  bool passed = true;
  passed &= (eA.getNeighbor(0) == nullptr);
  passed &= (eA.getNeighbor(1) == B);
  passed &= (eB.getNeighbor(0) == A);
  passed &= (eB.getNeighbor(1) == nullptr);
  return passed;
}

// Dummy iterator just for building ttree
struct DummyIterator {
};

// Simple Tree (1D)
//
//                |   A   ||   B   |
// structure  ->  |___|___||___|___|
bool testTree1D() {
  using Cell1D = Cell<2>;
  // Create 2 root cells
  auto A = std::make_shared<Cell1D>(nullptr);
  auto B = std::make_shared<Cell1D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell1D> eA{A}, eB{B};
  eA.setNeighbor(1, B);   // A(+x) = B
  eB.setNeighbor(0, A);   // B(‑x) = A

  // Create root cell entry vector and tree
  std::vector< RootCellEntry<Cell1D> > entries { eA, eB };
  Tree<Cell1D> tree;

  // Roots should be leaf at this point
  bool passed = true;
  passed &= A->isLeaf();
  passed &= B->isLeaf();

  tree.createRootCells(entries);

  // Roots should not be leaf and neighbors set properly
  passed &= !A->isLeaf();
  passed &= !B->isLeaf();
  passed &= (A->getNeighborCell(1) == B);
  passed &= (B->getNeighborCell(0) == A);
  return passed;
}

// Simple Tree (2D)
//                 _______  _______
//                |       ||       |
//                |   C   ||   D   |
// structure  ->  |_______||_______|
//                 _______  _______
//                |       ||       |
//                |   A   ||   B   |
//                |_______||_______|
bool testTree2D() {
  using Cell2D = Cell<2,2>;
  // Create 4 root cells
  auto A = std::make_shared<Cell2D>(nullptr);
  auto B = std::make_shared<Cell2D>(nullptr);
  auto C = std::make_shared<Cell2D>(nullptr);
  auto D = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A}, eB{B}, eC{C}, eD{D};
  eA.setNeighbor(1, B);          // A +x -> B
  eA.setNeighbor(3, C);          // A +y -> C
  eB.setNeighbor(0, A);          // B -x -> A
  eB.setNeighbor(3, D);          // B +y -> D
  eC.setNeighbor(1, D);          // C +x -> D
  eC.setNeighbor(2, A);          // C -y -> A
  eD.setNeighbor(0, C);          // D -x -> C
  eD.setNeighbor(2, B);          // D -y -> B
  std::vector< RootCellEntry<Cell2D> > entries { eA, eB, eC, eD };

  Tree<Cell2D> tree;
  tree.createRootCells(entries);

  bool passed = true;
  passed &= (A->getNeighborCell(1) == B);
  passed &= (A->getNeighborCell(3) == C);
  passed &= (B->getNeighborCell(0) == A);
  passed &= (B->getNeighborCell(3) == D);
  passed &= (C->getNeighborCell(1) == D);
  passed &= (C->getNeighborCell(2) == A);
  passed &= (D->getNeighborCell(0) == C);
  passed &= (D->getNeighborCell(2) == B);
  return passed;
}
