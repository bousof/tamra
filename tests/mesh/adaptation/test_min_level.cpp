#include <doctest.h>

#include <cmath>
#include <memory>
#include <vector>

#include <tamra/mesh/Cell.h>
#include <tamra/mesh/Tree.h>
#include <tamra/mesh/RootCellEntry.h>


// Mesh at min level and count number of leaf cells (one root)
TEST_CASE("[core][manager][min_level] Mesh at min level (one root, serial)") {
  using Cell2D = Cell<2,3>;
  // Create root cell
  auto cell_a = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> entry_a{cell_a};
  std::vector<RootCellEntry<Cell2D>> entries { entry_a };

  // Construction of the tree
  unsigned min_level{2}, max_level{3};
  Tree<Cell2D> tree(min_level, max_level);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Count number of leaf cells
  unsigned leaf_cell_count = cell_a->countLeaves();

  CHECK(leaf_cell_count == (unsigned)(pow(Cell2D::child_count, min_level)));
}

// Mesh at min level and count number of leaf cells (two roots)
TEST_CASE("[core][manager][min_level] Mesh at min level (two roots, serial)") {
  using Cell2D = Cell<2,3>;
  // Create 2 root cells
  auto cell_a = std::make_shared<Cell2D>(nullptr);
  auto cell_b = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> entry_a{cell_a}, entry_b{cell_b};
  entry_a.setNeighbor(1, cell_b);          // A +x -> B
  entry_b.setNeighbor(0, cell_a);          // B -x -> A
  std::vector<RootCellEntry<Cell2D>> entries { entry_a, entry_b };

  // Construction of the tree
  unsigned min_level{2}, max_level{3};
  Tree<Cell2D> tree(min_level, max_level);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  tree.meshAtMinLevel();

  // Count number of leaf cells
  unsigned leaf_cell_count = cell_a->countLeaves() + cell_b->countLeaves();

  CHECK(leaf_cell_count == tree.getRootCells().size() * (unsigned)(pow(Cell2D::child_count, min_level)));
}
