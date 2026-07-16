#include <doctest.h>

#include <cmath>
#include <memory>
#include <vector>

#include <core/Cell.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>

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
// Merge two adjacent binary tree partitions at the maximum level.
TEST_CASE("[core][tree] Merge adjacent binary partitions") {
  using Cell1D = Cell<2>;
  using BinaryTree = Tree<Cell1D>;

  auto A0 = std::make_shared<Cell1D>(nullptr);
  auto B0 = std::make_shared<Cell1D>(nullptr);
  RootCellEntry<Cell1D> eA0{A0}, eB0{B0};
  eA0.setNeighbor(1, B0);
  eB0.setNeighbor(0, A0);
  BinaryTree first_tree(1, 1, 0, 2);
  first_tree.createRootCells({eA0, eB0});
  A0->setToThisProcRecurs();
  B0->setToOtherProcRecurs();
  A0->getCellData().setValue(10.0);
  B0->getCellData().setValue(20.0);

  auto A1 = std::make_shared<Cell1D>(nullptr);
  auto B1 = std::make_shared<Cell1D>(nullptr);
  RootCellEntry<Cell1D> eA1{A1}, eB1{B1};
  eA1.setNeighbor(1, B1);
  eB1.setNeighbor(0, A1);
  BinaryTree second_tree(1, 1, 1, 2);
  second_tree.createRootCells({eA1, eB1});
  A1->setToOtherProcRecurs();
  B1->setToThisProcRecurs();
  A1->getCellData().setValue(100.0);
  B1->getCellData().setValue(200.0);

  CHECK_THROWS(second_tree.mergeWith(first_tree));

  first_tree.mergeWith(second_tree);

  CHECK(A0->getCellData().getValue() == 10.0);
  CHECK(B0->getCellData().getValue() == 200.0);
  CHECK(first_tree.countOwnedLeaves() == 4);
}

// Reject a merge when an unowned SFC interval lies between the two partitions.
TEST_CASE("[core][tree] Reject non-contiguous binary partitions") {
  using Cell1D = Cell<2>;
  using BinaryTree = Tree<Cell1D>;

  auto A0 = std::make_shared<Cell1D>(nullptr);
  auto B0 = std::make_shared<Cell1D>(nullptr);
  RootCellEntry<Cell1D> eA0{A0}, eB0{B0};
  eA0.setNeighbor(1, B0);
  eB0.setNeighbor(0, A0);
  BinaryTree first_tree(1, 1, 0, 2);
  first_tree.createRootCells({eA0, eB0});
  A0->setToThisProcRecurs();
  B0->setToOtherProcRecurs();

  auto A1 = std::make_shared<Cell1D>(nullptr);
  auto B1 = std::make_shared<Cell1D>(nullptr);
  RootCellEntry<Cell1D> eA1{A1}, eB1{B1};
  eA1.setNeighbor(1, B1);
  eB1.setNeighbor(0, A1);
  BinaryTree second_tree(1, 1, 1, 2);
  second_tree.createRootCells({eA1, eB1});
  A1->setToOtherProcRecurs();
  B1->setToThisProcRecurs();

  // Create a gape between the two partitions by setting the first cell of the second partition to other proc.
  MortonIterator<Cell1D> second_tree_iterator(second_tree.getRootCells(), second_tree.getMaxLevel());
  REQUIRE(second_tree_iterator.toOwnedBegin());
  second_tree_iterator.getCell()->setToOtherProc();
  CHECK_THROWS(first_tree.mergeWith(second_tree));

  // Create an intersection between the two partitions by setting the first cell of the second partition to other proc.
  second_tree_iterator.getCell()->setToThisProc();
  second_tree_iterator.prev();
  second_tree_iterator.getCell()->setToThisProc();
  A1->setToThisProc();
  CHECK_THROWS(first_tree.mergeWith(second_tree));

  // Remove the gap by setting the last cell of the first partition to other proc.
  MortonIterator<Cell1D> first_tree_iterator(first_tree.getRootCells(), first_tree.getMaxLevel());
  REQUIRE(first_tree_iterator.toOwnedEnd());
  first_tree_iterator.getCell()->setToOtherProc();
  first_tree.mergeWith(second_tree);
  CHECK(first_tree.countOwnedLeaves() == 4);
}

// Merge a partition whose topology is more refined than the destination tree.
TEST_CASE("[core][tree] Merge refined binary partition with extrapolation") {
  using Cell1D = Cell<2>;
  using BinaryTree = Tree<Cell1D>;

  auto A0 = std::make_shared<Cell1D>(nullptr);
  auto B0 = std::make_shared<Cell1D>(nullptr);
  RootCellEntry<Cell1D> eA0{A0}, eB0{B0};
  eA0.setNeighbor(1, B0);
  eB0.setNeighbor(0, A0);
  BinaryTree destination(1, 2, 0, 2);
  destination.createRootCells({eA0, eB0});
  A0->setToThisProcRecurs();
  B0->setToOtherProcRecurs();
  A0->getCellData().setValue(10.0);
  B0->getCellData().setValue(20.0);

  auto A1 = std::make_shared<Cell1D>(nullptr);
  auto B1 = std::make_shared<Cell1D>(nullptr);
  RootCellEntry<Cell1D> eA1{A1}, eB1{B1};
  eA1.setNeighbor(1, B1);
  eB1.setNeighbor(0, A1);
  BinaryTree source(1, 2, 1, 2);
  source.createRootCells({eA1, eB1});
  A1->setToOtherProcRecurs();
  B1->setToThisProcRecurs();
  A1->getCellData().setValue(100.0);
  B1->getCellData().setValue(200.0);
  B1->getChildCell(0)->split(2);
  B1->getChildCell(0)->getChildCell(0)->getCellData().setValue(210.0);
  B1->getChildCell(0)->getChildCell(1)->getCellData().setValue(211.0);

  unsigned extrapolated_cells = 0;
  destination.mergeWith(
    source,
    [&extrapolated_cells](const std::shared_ptr<Cell1D> &) { ++extrapolated_cells; }
  );

  CHECK(A0->getCellData().getValue() == 10.0);
  CHECK(B0->getChildCell(0)->isLeaf() == false);
  CHECK(B0->getChildCell(0)->getChildCell(0)->getCellData().getValue() == 210.0);
  CHECK(B0->getChildCell(0)->getChildCell(1)->getCellData().getValue() == 211.0);
  CHECK(extrapolated_cells > 0);
}
