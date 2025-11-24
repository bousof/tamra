#include <doctest.h>
#include <test_macros.h>

#include <cmath>
#include <memory>
#include <vector>

#include <core/Cell.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>
#include <UnitTestRegistry.h>

// RootCellEntry basic wiring (1D)
//
//                │   A   │   B   │
// structure  ->  └───┴───┴───┴───┘
TEST_CASE("[core][tree] RootCellEntry basic wiring (1D)") {
  using Cell1D = Cell<2>;
  // Create 2 root cells
  auto A = std::make_shared<Cell1D>(nullptr);
  auto B = std::make_shared<Cell1D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell1D> eA{A}, eB{B};
  eA.setNeighbor(1, B);   // A(+x) = B
  eB.setNeighbor(0, A);   // B(‑x) = A

  CHECK(eA.getNeighbor(0) == nullptr);
  CHECK(eA.getNeighbor(1) == B);
  CHECK(eB.getNeighbor(0) == A);
  CHECK(eB.getNeighbor(1) == nullptr);
}

// Simple Tree (1D)
//
//                │   A   ││   B   │
// structure  ->  └───┴───┘└───┴───┘
TEST_CASE("[core][tree] Simple Tree (1D)") {
  using Cell1D = Cell<2>;
  // Create 2 root cells
  auto A = std::make_shared<Cell1D>(nullptr);
  auto B = std::make_shared<Cell1D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell1D> eA{A}, eB{B};
  eA.setNeighbor(1, B);   // A +x -> B
  eB.setNeighbor(0, A);   // B ‑x -> A

  // Create root cell entry vector and tree
  std::vector<RootCellEntry<Cell1D>> entries { eA, eB };
  Tree<Cell1D> tree;

  // Roots should be leaf at this point
  CHECK(A->isLeaf());
  CHECK(B->isLeaf());

  // Create the tree root cells should split them at least to level 1
  tree.createRootCells(entries);

  // Roots should not be leaf and neighbors set properly
  CHECK(!A->isLeaf());
  CHECK(!B->isLeaf());
  CHECK(A->getNeighborCell(1) == B);
  CHECK(B->getNeighborCell(0) == A);
}

// Simple Tree (2D)
//                ┌───────┐┌───────┐
//                │       ││       │
//                │   C   ││   D   │
// structure  ->  │_______││_______│
//                ┌───────┐┌───────┐
//                │       ││       │
//                │   A   ││   B   │
//                │_______││_______│
TEST_CASE("[core][tree] Simple Tree (2D)") {
  using Cell2D = Cell<2,2>;
  // Create 4 root cells
  auto A = std::make_shared<Cell2D>(nullptr);
  auto B = std::make_shared<Cell2D>(nullptr);
  auto C = std::make_shared<Cell2D>(nullptr);
  auto D = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A}, eB{B}, eC{C}, eD{D};
  eA.setNeighbor(1, B);   // A +x -> B
  eA.setNeighbor(3, C);   // A +y -> C
  eB.setNeighbor(0, A);   // B -x -> A
  eB.setNeighbor(3, D);   // B +y -> D
  eC.setNeighbor(1, D);   // C +x -> D
  eC.setNeighbor(2, A);   // C -y -> A
  eD.setNeighbor(0, C);   // D -x -> C
  eD.setNeighbor(2, B);   // D -y -> B
  std::vector<RootCellEntry<Cell2D>> entries { eA, eB, eC, eD };

  Tree<Cell2D> tree;
  tree.createRootCells(entries);

  CHECK(A->getNeighborCell(1) == B);
  CHECK(A->getNeighborCell(3) == C);
  CHECK(B->getNeighborCell(0) == A);
  CHECK(B->getNeighborCell(3) == D);
  CHECK(C->getNeighborCell(1) == D);
  CHECK(C->getNeighborCell(2) == A);
  CHECK(D->getNeighborCell(0) == C);
  CHECK(D->getNeighborCell(2) == B);
}
