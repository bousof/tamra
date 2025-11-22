#include <iostream>
#include <vector>
#include <core/Cell.h>
#include <core/CellData.h>
#include <core/Oct.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>
#include <UnitTestRegistry.h>

bool testCoreTemplateClasses();
bool testCellAndOctBasicBehavior();
bool testCellSplitting();
bool testRecursiveSplitting();
bool testRecursiveSplittingPropagation1D();
bool testRecursiveSplittingPropagation2DDirect();
bool testRecursiveSplittingPropagation2DPlane();
bool testCoarsening();
bool testSplitBeyondMaxLevel();
bool testGetSiblingNumber();

void registerCoreCellOctSerialTests() {
  UnitTestRegistry::registerSerialTest("Test instanciation of core template classes", testCoreTemplateClasses, "core/cell_oct");
  UnitTestRegistry::registerSerialTest("Test Cell and Oct basic structure", testCellAndOctBasicBehavior, "core/cell_oct");
  UnitTestRegistry::registerSerialTest("Test Cell splitting with Oct", testCellSplitting, "core/cell_oct");
  UnitTestRegistry::registerSerialTest("Test recursive splitting to deeper levels", testRecursiveSplitting, "core/cell_oct");
  UnitTestRegistry::registerSerialTest("Test recursive splitting propagation to neighbors (1D)", testRecursiveSplittingPropagation1D, "core/cell_oct");
  UnitTestRegistry::registerSerialTest("Test recursive splitting propagation to neighbors (2D direct)", [=]() { return testRecursiveSplittingPropagation2DDirect(); }, "core/cell_oct");
  UnitTestRegistry::registerSerialTest("Test recursive splitting propagation to neighbors (2D plane)", [=]() { return testRecursiveSplittingPropagation2DPlane(); }, "core/cell_oct");
  UnitTestRegistry::registerSerialTest("Test coarsening a cell", testCoarsening, "core/cell_oct");
  UnitTestRegistry::registerSerialTest("Test split beyond max level throws", testSplitBeyondMaxLevel, "core/cell_oct");
  UnitTestRegistry::registerSerialTest("Test sibling number consistency", testGetSiblingNumber, "core/cell_oct");
}

// Test instanciation of core template classes
bool testCoreTemplateClasses() {
  Cell<2> cell2;
  Cell<2, 2> cell22;
  Cell<2, 2, 2> cell222;
  Oct<Cell<2>> oct2;
  Oct<Cell<2, 2>> oct22;
  Oct<Cell<2, 2, 2>> oct222;
  CellData cellData;

  bool passed = true;
  // Number of dimensions
  passed &= cell2.number_dimensions == 1;
  passed &= cell22.number_dimensions == 2;
  passed &= cell222.number_dimensions == 3;
  // Number of children
  passed &= cell2.number_children == 2;
  passed &= cell22.number_children == 4;
  passed &= cell222.number_children == 8;
  passed &= oct2.number_children == 2;
  passed &= oct22.number_children == 2*2;
  passed &= oct222.number_children == 2*2*2;
  // Number of neighbors
  passed &= cell2.number_neighbors == 2;
  passed &= cell22.number_neighbors == 4;
  passed &= cell222.number_neighbors == 6;
  // Number of 2D neighbors
  passed &= cell2.number_plane_neighbors == 2;
  passed &= cell22.number_plane_neighbors == 8;
  passed &= cell222.number_plane_neighbors == 18;
  // Number of 3D neighbors
  passed &= cell2.number_volume_neighbors == 2;
  passed &= cell22.number_volume_neighbors == 8;
  passed &= cell222.number_volume_neighbors == 26;
  return passed;
}

// Test Cell and Oct basic structure
bool testCellAndOctBasicBehavior() {
  using CellType = Cell<2,2>; // 2D quadtree
  Oct<CellType> oct;
  std::shared_ptr<CellType> dummy_parent = std::make_shared<CellType>();
  oct.init(dummy_parent, 1);
  auto children = oct.getChildCells();

  bool passed = children.size() == CellType::number_children;
  // All child cells should be nullptr by default
  for (const auto &ptr : children)
    passed &= (ptr == nullptr);

  // Clear oct
  oct.clear();

  passed &= (oct.getParentCell() == nullptr);
  passed &= (oct.getLevel() == 0);
  return passed;
}

// Test Cell splitting with Oct
bool testCellSplitting() {
  using CellType = Cell<2,2>; // 2D quadtree

  auto root = std::make_shared<CellType>(nullptr);
  auto children = root->splitRoot(2, root);  // Split with max_level = 2

  bool passed = children.size() == CellType::number_children;

  for (const auto &child : children) {
    passed &= child->getLevel() == 1;
    passed &= child->isLeaf();
  }

  return passed;
}

// Test recursive splitting to deeper levels
bool testRecursiveSplitting() {
  using CellType = Cell<2,2>; // 2D
  const unsigned max_level{3};

  auto root = std::make_shared<CellType>(nullptr);
  auto children = root->splitRoot(max_level, root);

  bool passed = root->isRoot();
  for (auto &child : children) {
    child->split(max_level);
    auto grandchildren = child->getChildOct()->getChildCells();
    passed &= child->getLevel() == 1;
    passed &= child->getParentOct() == root->getChildOct();
    passed &= grandchildren.size() == CellType::number_children;
    for (auto &gc : grandchildren) {
      passed &= gc->getLevel() == 2;
      passed &= gc->getParentOct() == child->getChildOct();
    }
  }

  return passed;
}

// Test recursive splitting propagation
//                      This cell is splitted
//                      | should propagate split to its right level 1 cell first
//                      |
// level      ->    2   2     1
// structure  ->  |___|___|_______|
bool testRecursiveSplittingPropagation1D() {
  using CellType = Cell<2>; // 1D
  const unsigned max_level{3};

  // Create initial tree
  auto root = std::make_shared<CellType>(nullptr);
  auto children = root->splitRoot(max_level, root);
  auto grandchildren = children[0]->split(max_level);

  // The level 1 cell should be leaf
  bool passed = children[1]->isLeaf();

  // We now split the grandchild
  grandchildren[1]->split(max_level);

  // The level 1 cell should not be a leaf anymore
  passed &= !children[1]->isLeaf();

  return passed;
}

// Test recursive splitting propagation in 2D to plane neighbors
// Cell X is splitted should propagate split to its right level 1 cell Y first
//                 _______________
//                |       |       |
//                |       |       |
//                |       |       |
//                |_______|_______|
//                |   |   |       |
//                |___|___|   Y   |
//                |   | X |       |
//                |___|___|_______|
bool testRecursiveSplittingPropagation2DPlane() {
  using CellType = Cell<2, 2>; // 2D
  const unsigned max_level{3};

  // Create initial tree
  auto root = std::make_shared<CellType>(nullptr);
  auto children = root->splitRoot(max_level, root);
  auto grandchildren = children[0]->split(max_level);

  // The level 1 cell should be leaf
  bool passed = children[1]->isLeaf();

  // We now split the grandchild
  grandchildren[1]->split(max_level);

  // The level 1 right cell should not be a leaf anymore
  passed &= !children[1]->isLeaf();

  // The other level 1 cells should still be leafs
  passed &= children[2]->isLeaf();
  passed &= children[3]->isLeaf();

  return passed;
}

// Test recursive splitting propagation in 2D to plane neighbors
// Cell X is splitted should propagate split to its left and top level 1 cells Y and Z first
//                 _______________
//                |       |       |
//                |       |   Z   |
//                |       |       |
//                |_______|_______|
//                |       | X |   |
//                |   Y   |___|___|
//                |       |   |   |
//                |_______|___|___|
bool testRecursiveSplittingPropagation2DDirect() {
  using CellType = Cell<2, 2>; // 2D
  const unsigned max_level{3};

  // Create initial tree
  auto root = std::make_shared<CellType>(nullptr);
  auto children = root->splitRoot(max_level, root);
  auto grandchildren = children[1]->split(max_level);

  // The level 1 cell should be leaf
  bool passed = children[0]->isLeaf();
  passed &= children[3]->isLeaf();

  // We now split the grandchild
  grandchildren[2]->split(max_level);

  // The level 1 right cell should not be a leaf anymore
  passed &= !children[0]->isLeaf();
  passed &= !children[3]->isLeaf();

  // The other level 1 cells should still be leafs
  passed &= children[2]->isLeaf();

  return passed;
}

// Test coarsening a cell
bool testCoarsening() {
  using CellType = Cell<2,2>; // 2D
  const unsigned min_level{0};

  auto root = std::make_shared<CellType>(nullptr);
  root->splitRoot(2, root);
  bool passed = !root->isLeaf();

  bool is_coarsened = root->coarsen(min_level); // Should remove child oct
  passed &= is_coarsened;
  passed &= root->isLeaf();

  return passed;
}

// Test split beyond max level throws
bool testSplitBeyondMaxLevel() {
  using CellType = Cell<2,2>;

  auto root = std::make_shared<CellType>(nullptr);

  bool exception_thrown = false;
  try {
    root->splitRoot(0, root); // root is already at level 0, shouldn't allow
  } catch (const std::exception &e) {
    exception_thrown = true;
  }
  bool passed = exception_thrown;

  auto children = root->splitRoot(1, root);

  exception_thrown = false;
  try {
    children[0]->split(1); // child is already at level 1, shouldn't allow
  } catch (const std::exception &e) {
    exception_thrown = true;
  }
  passed &= exception_thrown;

  return passed;
}

// Test sibling number consistency
bool testGetSiblingNumber() {
  using CellType = Cell<2,2>;

  auto root = std::make_shared<CellType>(nullptr);
  auto children = root->splitRoot(1, root);

  auto oct = root->getChildOct();

  bool passed = true;
  for (size_t i{0}; i<children.size(); ++i) {
    auto actual_index = oct->getSiblingNumber(children[i].get());
    passed &= (i == actual_index);
  }

  return passed;
}
