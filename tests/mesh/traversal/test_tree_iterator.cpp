#include <doctest.h>

#include <memory>
#include <vector>

#include <tamra/mesh/Cell.h>
#include <tamra/mesh/Tree.h>
#include <tamra/mesh/RootCellEntry.h>


// Iterator leaf cell counting (1 root cell)
//                ┌───┬───┬───────┐
//                │   │   │       │
//                │___│___│       │
//                │   │   │       │
// structure  ->  │___│___│_______│
//                │       │   │   │
//                │       │___│___│
//                │       │   │   │
//                │_______│___│___│
// Number of leaf cells should be 10
TEST_CASE("[core][tree_iterator] Iterator leaf cell counting (1 root cell)") {
  using Cell2D = Cell<2,2>;
  // Create 4 root cells
  auto cell_a = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> entry_a{cell_a};
  std::vector<RootCellEntry<Cell2D>> entries { entry_a };

  // Construction of the tree
  unsigned min_level{1}, max_level{2};
  Tree<Cell2D> tree(min_level, max_level);
  tree.createRootCells(entries);

  //Splitting some cells
  cell_a->getChildCell(1)->split(max_level);
  cell_a->getChildCell(2)->split(max_level);

  // Count number of leaf cells
  int leaf_cell_count = cell_a->countLeaves();

  CHECK(leaf_cell_count == 10);
}

// Iterator leaf cell counting (2 root cells)
//                      root A           root B
//                ┌───────┬───────┐┌───────┬───┬───┐
//                │       │       ││       │   │   │
//                │       │       ││       │___│___│
//                │       │       ││       │   │   │
// structure  ->  │_______│_______││_______│___│___│
//                │       │   │   ││       │       │
//                │       │___│___││       │       │
//                │       │   │   ││       │       │
//                │_______│___│___││_______│_______│
// Number of leaf cells should be 10
TEST_CASE("[core][tree_iterator] Iterator leaf cell counting (2 root cells)") {
  using Cell2D = Cell<2,2>;
  // Create 4 root cells
  auto cell_a = std::make_shared<Cell2D>(nullptr);
  auto cell_b = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> entry_a{cell_a}, entry_b{cell_b};
  entry_a.setNeighbor(1, cell_b);          // A +x -> B
  entry_b.setNeighbor(0, cell_a);          // B -x -> A
  std::vector<RootCellEntry<Cell2D>> entries { entry_a, entry_b };

  // Construction of the tree
  unsigned min_level{1}, max_level{2};
  Tree<Cell2D> tree(min_level, max_level);
  tree.createRootCells(entries);

  //Splitting some cells
  cell_a->getChildCell(1)->split(max_level);
  cell_b->getChildCell(3)->split(max_level);

  // Count number of leaf cells
  int leaf_cell_count = cell_a->countLeaves() + cell_b->countLeaves();

  CHECK(leaf_cell_count == 14);
}
