#include <doctest.h>

#include <memory>
#include <vector>

#include <tamra/mesh/Cell.h>
#include <tamra/mesh/Tree.h>
#include <tamra/mesh/RootCellEntry.h>


// RootCellEntry basic wiring (1D)
//
//                │   A   │   B   │
// structure  ->  └───┴───┴───┴───┘
TEST_CASE("[core][tree] RootCellEntry basic wiring (1D)") {
  using Cell1D = Cell<2>;
  // Create 2 root cells
  auto cell_a = std::make_shared<Cell1D>(nullptr);
  auto cell_b = std::make_shared<Cell1D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell1D> entry_a{cell_a}, entry_b{cell_b};
  entry_a.setNeighbor(1, cell_b);   // A(+x) = B
  entry_b.setNeighbor(0, cell_a);   // B(‑x) = A

  CHECK(entry_a.getNeighbor(0) == nullptr);
  CHECK(entry_a.getNeighbor(1) == cell_b);
  CHECK(entry_b.getNeighbor(0) == cell_a);
  CHECK(entry_b.getNeighbor(1) == nullptr);
}

// Simple Tree (1D)
//
//                │   A   ││   B   │
// structure  ->  └───┴───┘└───┴───┘
TEST_CASE("[core][tree] Simple Tree (1D)") {
  using Cell1D = Cell<2>;
  // Create 2 root cells
  auto cell_a = std::make_shared<Cell1D>(nullptr);
  auto cell_b = std::make_shared<Cell1D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell1D> entry_a{cell_a}, entry_b{cell_b};
  entry_a.setNeighbor(1, cell_b);   // A +x -> B
  entry_b.setNeighbor(0, cell_a);   // B ‑x -> A

  // Create root cell entry vector and tree
  std::vector<RootCellEntry<Cell1D>> entries { entry_a, entry_b };
  Tree<Cell1D> tree;

  // Roots should be leaf at this point
  CHECK(cell_a->isLeaf());
  CHECK(cell_b->isLeaf());

  // Create the tree root cells should split them at least to level 1
  tree.createRootCells(entries);

  // Roots should not be leaf and neighbors set properly
  CHECK(!cell_a->isLeaf());
  CHECK(!cell_b->isLeaf());
  CHECK(cell_a->getNeighborCell(1) == cell_b);
  CHECK(cell_b->getNeighborCell(0) == cell_a);
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
  auto cell_a = std::make_shared<Cell2D>(nullptr);
  auto cell_b = std::make_shared<Cell2D>(nullptr);
  auto cell_c = std::make_shared<Cell2D>(nullptr);
  auto cell_d = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> entry_a{cell_a}, entry_b{cell_b}, entry_c{cell_c}, entry_d{cell_d};
  entry_a.setNeighbor(1, cell_b);   // A +x -> B
  entry_a.setNeighbor(3, cell_c);   // A +y -> C
  entry_b.setNeighbor(0, cell_a);   // B -x -> A
  entry_b.setNeighbor(3, cell_d);   // B +y -> D
  entry_c.setNeighbor(1, cell_d);   // C +x -> D
  entry_c.setNeighbor(2, cell_a);   // C -y -> A
  entry_d.setNeighbor(0, cell_c);   // D -x -> C
  entry_d.setNeighbor(2, cell_b);   // D -y -> B
  std::vector<RootCellEntry<Cell2D>> entries { entry_a, entry_b, entry_c, entry_d };

  Tree<Cell2D> tree;
  tree.createRootCells(entries);

  CHECK(cell_a->getNeighborCell(1) == cell_b);
  CHECK(cell_a->getNeighborCell(3) == cell_c);
  CHECK(cell_b->getNeighborCell(0) == cell_a);
  CHECK(cell_b->getNeighborCell(3) == cell_d);
  CHECK(cell_c->getNeighborCell(1) == cell_d);
  CHECK(cell_c->getNeighborCell(2) == cell_a);
  CHECK(cell_d->getNeighborCell(0) == cell_c);
  CHECK(cell_d->getNeighborCell(2) == cell_b);
}
// Merge two adjacent binary tree partitions at the maximum level.
TEST_CASE("[core][tree] Merge adjacent binary partitions") {
  using Cell1D = Cell<2>;
  using BinaryTree = Tree<Cell1D>;

  auto cell_a0 = std::make_shared<Cell1D>(nullptr);
  auto cell_b0 = std::make_shared<Cell1D>(nullptr);
  RootCellEntry<Cell1D> entry_a0{cell_a0}, entry_b0{cell_b0};
  entry_a0.setNeighbor(1, cell_b0);
  entry_b0.setNeighbor(0, cell_a0);
  BinaryTree first_tree(1, 1, 0, 2);
  first_tree.createRootCells({entry_a0, entry_b0});
  cell_a0->markOwnedRecurs();
  cell_b0->markRemoteRecurs();
  cell_a0->getCellData().setValue(10.0);
  cell_b0->getCellData().setValue(20.0);

  auto cell_a1 = std::make_shared<Cell1D>(nullptr);
  auto cell_b1 = std::make_shared<Cell1D>(nullptr);
  RootCellEntry<Cell1D> entry_a1{cell_a1}, entry_b1{cell_b1};
  entry_a1.setNeighbor(1, cell_b1);
  entry_b1.setNeighbor(0, cell_a1);
  BinaryTree second_tree(1, 1, 1, 2);
  second_tree.createRootCells({entry_a1, entry_b1});
  cell_a1->markRemoteRecurs();
  cell_b1->markOwnedRecurs();
  cell_a1->getCellData().setValue(100.0);
  cell_b1->getCellData().setValue(200.0);

  CHECK_THROWS(second_tree.mergeWith(first_tree));

  first_tree.mergeWith(second_tree);

  CHECK(cell_a0->getCellData().getValue() == 10.0);
  CHECK(cell_b0->getCellData().getValue() == 200.0);
  CHECK(first_tree.countOwnedLeaves() == 4);
}

// Reject a merge when an unowned SFC interval lies between the two partitions.
TEST_CASE("[core][tree] Reject non-contiguous binary partitions") {
  using Cell1D = Cell<2>;
  using BinaryTree = Tree<Cell1D>;

  auto cell_a0 = std::make_shared<Cell1D>(nullptr);
  auto cell_b0 = std::make_shared<Cell1D>(nullptr);
  RootCellEntry<Cell1D> entry_a0{cell_a0}, entry_b0{cell_b0};
  entry_a0.setNeighbor(1, cell_b0);
  entry_b0.setNeighbor(0, cell_a0);
  BinaryTree first_tree(1, 1, 0, 2);
  first_tree.createRootCells({entry_a0, entry_b0});
  cell_a0->markOwnedRecurs();
  cell_b0->markRemoteRecurs();

  auto cell_a1 = std::make_shared<Cell1D>(nullptr);
  auto cell_b1 = std::make_shared<Cell1D>(nullptr);
  RootCellEntry<Cell1D> entry_a1{cell_a1}, entry_b1{cell_b1};
  entry_a1.setNeighbor(1, cell_b1);
  entry_b1.setNeighbor(0, cell_a1);
  BinaryTree second_tree(1, 1, 1, 2);
  second_tree.createRootCells({entry_a1, entry_b1});
  cell_a1->markRemoteRecurs();
  cell_b1->markOwnedRecurs();

  // Create a gape between the two partitions by setting the first cell of the second partition to other proc.
  MortonIterator<Cell1D> second_tree_iterator(second_tree.getRootCells(), second_tree.getMaxLevel());
  REQUIRE(second_tree_iterator.toOwnedBegin());
  second_tree_iterator.getCell()->markRemote();
  CHECK_THROWS(first_tree.mergeWith(second_tree));

  // Create an intersection between the two partitions by setting the first cell of the second partition to other proc.
  second_tree_iterator.getCell()->markOwned();
  second_tree_iterator.prev();
  second_tree_iterator.getCell()->markOwned();
  cell_a1->markOwned();
  CHECK_THROWS(first_tree.mergeWith(second_tree));

  // Remove the gap by setting the last cell of the first partition to other proc.
  MortonIterator<Cell1D> first_tree_iterator(first_tree.getRootCells(), first_tree.getMaxLevel());
  REQUIRE(first_tree_iterator.toOwnedEnd());
  first_tree_iterator.getCell()->markRemote();
  first_tree.mergeWith(second_tree);
  CHECK(first_tree.countOwnedLeaves() == 4);
}

// Merge a partition whose topology is more refined than the destination tree.
TEST_CASE("[core][tree] Merge refined binary partition with extrapolation") {
  using Cell1D = Cell<2>;
  using BinaryTree = Tree<Cell1D>;

  auto cell_a0 = std::make_shared<Cell1D>(nullptr);
  auto cell_b0 = std::make_shared<Cell1D>(nullptr);
  RootCellEntry<Cell1D> entry_a0{cell_a0}, entry_b0{cell_b0};
  entry_a0.setNeighbor(1, cell_b0);
  entry_b0.setNeighbor(0, cell_a0);
  BinaryTree destination(1, 2, 0, 2);
  destination.createRootCells({entry_a0, entry_b0});
  cell_a0->markOwnedRecurs();
  cell_b0->markRemoteRecurs();
  cell_a0->getCellData().setValue(10.0);
  cell_b0->getCellData().setValue(20.0);

  auto cell_a1 = std::make_shared<Cell1D>(nullptr);
  auto cell_b1 = std::make_shared<Cell1D>(nullptr);
  RootCellEntry<Cell1D> entry_a1{cell_a1}, entry_b1{cell_b1};
  entry_a1.setNeighbor(1, cell_b1);
  entry_b1.setNeighbor(0, cell_a1);
  BinaryTree source(1, 2, 1, 2);
  source.createRootCells({entry_a1, entry_b1});
  cell_a1->markRemoteRecurs();
  cell_b1->markOwnedRecurs();
  cell_a1->getCellData().setValue(100.0);
  cell_b1->getCellData().setValue(200.0);
  cell_b1->getChildCell(0)->split(2);
  cell_b1->getChildCell(0)->getChildCell(0)->getCellData().setValue(210.0);
  cell_b1->getChildCell(0)->getChildCell(1)->getCellData().setValue(211.0);

  unsigned extrapolated_cells = 0;
  destination.mergeWith(
    source,
    [&extrapolated_cells](const std::shared_ptr<Cell1D> &) { ++extrapolated_cells; }
  );

  CHECK(cell_a0->getCellData().getValue() == 10.0);
  CHECK(cell_b0->getChildCell(0)->isLeaf() == false);
  CHECK(cell_b0->getChildCell(0)->getChildCell(0)->getCellData().getValue() == 210.0);
  CHECK(cell_b0->getChildCell(0)->getChildCell(1)->getCellData().getValue() == 211.0);
  CHECK(extrapolated_cells > 0);
}
