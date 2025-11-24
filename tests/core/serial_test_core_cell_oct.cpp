#include <doctest.h>
#include <test_macros.h>

#include <iostream>
#include <vector>

#include <core/Cell.h>
#include <core/CellData.h>
#include <core/Oct.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>

// Test instanciation of core template classes
TEST_CASE("[core][cell_oct] Instanciation of core template classes") {
  Cell<2> cell2;
  Cell<2, 2> cell22;
  Cell<2, 2, 2> cell222;
  Oct<Cell<2>> oct2;
  Oct<Cell<2, 2>> oct22;
  Oct<Cell<2, 2, 2>> oct222;
  CellData cellData;

  // Number of dimensions
  CHECK(cell2.number_dimensions == 1);
  CHECK(cell22.number_dimensions == 2);
  CHECK(cell222.number_dimensions == 3);
  // Number of children
  CHECK(cell2.number_children == 2);
  CHECK(cell22.number_children == 4);
  CHECK(cell222.number_children == 8);
  CHECK(oct2.number_children == 2);
  CHECK(oct22.number_children == 2*2);
  CHECK(oct222.number_children == 2*2*2);
  // Number of neighbors
  CHECK(cell2.number_neighbors == 2);
  CHECK(cell22.number_neighbors == 4);
  CHECK(cell222.number_neighbors == 6);
  // Number of 2D neighbors
  CHECK(cell2.number_plane_neighbors == 2);
  CHECK(cell22.number_plane_neighbors == 8);
  CHECK(cell222.number_plane_neighbors == 18);
  // Number of 3D neighbors
  CHECK(cell2.number_volume_neighbors == 2);
  CHECK(cell22.number_volume_neighbors == 8);
  CHECK(cell222.number_volume_neighbors == 26);
}

// Test Cell and Oct basic structure
TEST_CASE("[core][cell_oct] Cell and Oct basic structure") {
  using CellType = Cell<2,2>; // 2D quadtree
  Oct<CellType> oct;
  std::shared_ptr<CellType> dummy_parent = std::make_shared<CellType>();
  oct.init(dummy_parent, 1);
  auto children = oct.getChildCells();

  CHECK(children.size() == CellType::number_children);
  // All child cells should be nullptr by default
  for (const auto &ptr : children)
    CHECK(ptr == nullptr);

  // Clear oct
  oct.clear();

  CHECK(oct.getParentCell() == nullptr);
  CHECK(oct.getLevel() == 0);
}

// Test Cell splitting with Oct
TEST_CASE("[core][cell_oct] Cell splitting with Oct") {
  using CellType = Cell<2,2>; // 2D quadtree

  auto root = std::make_shared<CellType>(nullptr);
  auto children = root->splitRoot(2, root);  // Split with max_level = 2

  CHECK(children.size() == CellType::number_children);

  for (const auto &child : children) {
    CHECK(child->getLevel() == 1);
    CHECK(child->isLeaf());
  }
}

// Test recursive splitting to deeper levels
TEST_CASE("[core][cell_oct] Recursive splitting to deeper levels") {
  using CellType = Cell<2,2>; // 2D
  const unsigned max_level{3};

  auto root = std::make_shared<CellType>(nullptr);
  auto children = root->splitRoot(max_level, root);

  CHECK(root->isRoot());
  for (auto &child : children) {
    child->split(max_level);
    auto grandchildren = child->getChildOct()->getChildCells();
    CHECK(child->getLevel() == 1);
    CHECK(child->getParentOct() == root->getChildOct());
    CHECK(grandchildren.size() == CellType::number_children);
    for (auto &gc : grandchildren) {
      CHECK(gc->getLevel() == 2);
      CHECK(gc->getParentOct() == child->getChildOct());
    }
  }
}

// Test recursive splitting propagation
//                      This cell is splitted
//                      | should propagate split to its right level 1 cell first
//                      |
// level      ->  │ 2 │ 2 │   1.  │
// structure  ->  └───┴───┴───────┘
TEST_CASE("[core][cell_oct] Recursive splitting propagation to neighbors (1D)") {
  using CellType = Cell<2>; // 1D
  const unsigned max_level{3};

  // Create initial tree
  auto root = std::make_shared<CellType>(nullptr);
  auto children = root->splitRoot(max_level, root);
  auto grandchildren = children[0]->split(max_level);

  // The level 1 cell should be leaf
  CHECK(children[1]->isLeaf());

  // We now split the grandchild
  grandchildren[1]->split(max_level);

  // The level 1 cell should not be a leaf anymore
  CHECK(!children[1]->isLeaf());
}

// Test recursive splitting propagation in 2D to plane neighbors
// Cell X is splitted should propagate split to its right level 1 cell Y first
//                ┌───────┬───────┐
//                │       │       │
//                │       │       │
//                │       │       │
//                │_______│_______│
//                │   │   │       │
//                │___│___│   Y   │
//                │   │ X │       │
//                │___│___│_______│
TEST_CASE("[core][cell_oct] Recursive splitting propagation to neighbors (2D direct)") {
  using CellType = Cell<2, 2>; // 2D
  const unsigned max_level{3};

  // Create initial tree
  auto root = std::make_shared<CellType>(nullptr);
  auto children = root->splitRoot(max_level, root);
  auto grandchildren = children[0]->split(max_level);

  // The level 1 cell should be leaf
  CHECK(children[1]->isLeaf());

  // We now split the grandchild
  grandchildren[1]->split(max_level);

  // The level 1 right cell should not be a leaf anymore
  CHECK(!children[1]->isLeaf());

  // The other level 1 cells should still be leafs
  CHECK(children[2]->isLeaf());
  CHECK(children[3]->isLeaf());
}

// Test recursive splitting propagation in 2D to plane neighbors
// Cell X is splitted should propagate split to its left and top level 1 cells Y and Z first
//                ┌───────┬───────┐
//                │       │       │
//                │       │   Z   │
//                │       │       │
//                │_______│_______│
//                │       │ X │   │
//                │   Y   │___│___│
//                │       │   │   │
//                │_______│___│___│
TEST_CASE("[core][cell_oct] Recursive splitting propagation to neighbors (2D plane)") {
  using CellType = Cell<2, 2>; // 2D
  const unsigned max_level{3};

  // Create initial tree
  auto root = std::make_shared<CellType>(nullptr);
  auto children = root->splitRoot(max_level, root);
  auto grandchildren = children[1]->split(max_level);

  // The level 1 cell should be leaf
  CHECK(children[0]->isLeaf());
  CHECK(children[3]->isLeaf());

  // We now split the grandchild
  grandchildren[2]->split(max_level);

  // The level 1 right cell should not be a leaf anymore
  CHECK(!children[0]->isLeaf());
  CHECK(!children[3]->isLeaf());

  // The other level 1 cells should still be leafs
  CHECK(children[2]->isLeaf());
}

// Test coarsening a cell
TEST_CASE("[core][cell_oct] Coarsening a cell") {
  using CellType = Cell<2,2>; // 2D
  const unsigned min_level{0};

  auto root = std::make_shared<CellType>(nullptr);
  root->splitRoot(2, root);
  CHECK(!root->isLeaf());

  bool is_coarsened = root->coarsen(min_level); // Should remove child oct
  CHECK(is_coarsened);
  CHECK(root->isLeaf());
}

// Test split beyond max level throws
TEST_CASE("[core][cell_oct] Split beyond max level throws") {
  using CellType = Cell<2,2>;

  auto root = std::make_shared<CellType>(nullptr);

  bool exception_thrown = false;
  try {
    root->splitRoot(0, root); // root is already at level 0, shouldn't allow
  } catch (const std::exception &e) {
    exception_thrown = true;
  }
  CHECK(exception_thrown);

  auto children = root->splitRoot(1, root);

  exception_thrown = false;
  try {
    children[0]->split(1); // child is already at level 1, shouldn't allow
  } catch (const std::exception &e) {
    exception_thrown = true;
  }
  CHECK(exception_thrown);
}

// Test sibling number consistency
TEST_CASE("[core][cell_oct] Sibling number consistency") {
  using CellType = Cell<2,2>;

  auto root = std::make_shared<CellType>(nullptr);
  auto children = root->splitRoot(1, root);

  auto oct = root->getChildOct();

  for (size_t i{0}; i<children.size(); ++i) {
    auto actual_index = oct->getSiblingNumber(children[i].get());
    CHECK(i == actual_index);
  }
}
