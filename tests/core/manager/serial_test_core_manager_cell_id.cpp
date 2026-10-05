#include <doctest.h>

#include <cmath>
#include <memory>
#include <vector>

#include <core/Cell.h>
#include <core/manager/CellIdManager.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>

// Cell ID computation and inversion (2x2)
//                ┌───┬───┬───────┐
//                │   │ X │       │
//                ├───┼───┤       │
//                │   │   │       │
// structure  ->  ├───┴───┼───────┤
//                │       │       │
//                │       │       │
//                │       │       │
//                └───────┴───────┘
// X is the cell that we test
TEST_CASE("[core][manager][cell_id] Cell ID computation and inversion (2x2)") {
  using Cell2D = Cell<2,2>;
  // Create 4 root cells
  auto A = std::make_shared<Cell2D>(nullptr);

  //Splitting some cells
  unsigned max_level{2};
  A->splitRoot(max_level, A);
  A->getChildCell(2)->split(max_level);

  // Initialize iterator
  std::vector<std::shared_ptr<Cell2D>> roots = { A };
  MortonIterator<Cell2D, 123> iterator(roots, max_level);

  // Go to target cell
  iterator.toBegin(max_level);
  for (int i{0}; i<5; ++i)
    iterator.next(max_level);
  const std::vector<unsigned> index_path = iterator.getIndexPath();
  const std::vector<unsigned> index_path_value = {0, 2, 3};

  CHECK(iterator.getIndexPath() == index_path_value);
  CHECK(iterator.getCellId() == iterator.indexPathToId(index_path_value));
  CHECK(iterator.idToOrderPath(iterator.getCellId()) == iterator.getOrderPath());
}

// Cell ID computation and inversion (3x2 deep)
//                ┌───────────┬───────────┬───────────┐
//                │           │           │           │
//                │           │           │           │
//                │           │           │           │
// structure  ->  ├───┬───┬───┼───────────┼───────────┤
//                │   │   │   │           │           │
//                ├───┼───┼───┤           │           │
//                │ X │   │   │           │           │
//                └───┴───┴───┴───────────┴───────────┘
// Recursively split first child of leaf marked X untile level 20
TEST_CASE("[core][manager][cell_id] Cell ID computation and inversion (3x2 deep)") {
  using Cell2D = Cell<3,2>;
  // Create 4 root cells
  auto A = std::make_shared<Cell2D>(nullptr);

  //Splitting some cells
  unsigned max_level{20};
  auto child_cells = A->splitRoot(max_level, A);
  for (unsigned i{0}; i<(max_level-1); ++i)
    child_cells = child_cells[0]->split(max_level);

  // Initialize iterator
  std::vector<std::shared_ptr<Cell2D>> roots = { A };
  MortonIterator<Cell2D, 123> iterator(roots, max_level);

  // Go to target cell
  iterator.toBegin(max_level);
  CHECK(iterator.getIndexPath() == iterator.idToOrderPath(iterator.getCellId()));
  while (iterator.next())
    CHECK(iterator.getIndexPath() == iterator.idToOrderPath(iterator.getCellId()));
}

// Equal size partition computation
TEST_CASE("[core][manager][cell_id] Equal size partition computation does not throw") {
  using Cell2D = Cell<2,2>;
  CellIdManager<Cell2D> cell_id_manager(1, 3);

  bool exception_thrown = false;
  try {
    cell_id_manager.getEqualPartitions(2, 3);
  } catch (const std::exception &e) {
    exception_thrown = true;
  }

  CHECK(!exception_thrown);
}

// A coarse cell ends after an inner descendant, unless that descendant is
// on the last-child branch all the way to the coarse cell's end.
TEST_CASE("[core][manager][cell_id] cellIdLte compares ancestor to descendant ends") {
  using Cell2D = Cell<2,2>;
  CellIdManager<Cell2D> cell_id_manager(1, 3);

  const auto ancestor = cell_id_manager.orderPathToId({0, 1});
  const auto inner_child = cell_id_manager.orderPathToId({0, 1, 2});
  const auto inner_grandchild = cell_id_manager.orderPathToId({0, 1, 3, 2});
  const auto last_child = cell_id_manager.orderPathToId({0, 1, 3});
  const auto last_grandchild = cell_id_manager.orderPathToId({0, 1, 3, 3});

  CHECK_FALSE(cell_id_manager.cellIdLte(ancestor, inner_child));
  CHECK_FALSE(cell_id_manager.cellIdLte(ancestor, inner_grandchild));
  CHECK(cell_id_manager.cellIdLte(ancestor, last_child));
  CHECK(cell_id_manager.cellIdLte(ancestor, last_grandchild));
}

// The descendant always ends before or at the end of its ancestor. Use a
// second root to cover the cell ID layout used by multi-root trees too.
TEST_CASE("[core][manager][cell_id] cellIdLte compares descendant to ancestor ends") {
  using Cell2D = Cell<2,2>;
  CellIdManager<Cell2D> cell_id_manager(2, 3);

  const auto ancestor = cell_id_manager.orderPathToId({1, 1});
  const auto inner_child = cell_id_manager.orderPathToId({1, 1, 2});
  const auto inner_grandchild = cell_id_manager.orderPathToId({1, 1, 3, 2});
  const auto last_child = cell_id_manager.orderPathToId({1, 1, 3});
  const auto last_grandchild = cell_id_manager.orderPathToId({1, 1, 3, 3});

  CHECK(cell_id_manager.cellIdLte(inner_child, ancestor));
  CHECK(cell_id_manager.cellIdLte(inner_grandchild, ancestor));
  CHECK(cell_id_manager.cellIdLte(last_child, ancestor));
  CHECK(cell_id_manager.cellIdLte(last_grandchild, ancestor));
}
