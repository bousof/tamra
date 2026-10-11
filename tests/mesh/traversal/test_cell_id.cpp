#include <doctest.h>

#include <memory>
#include <vector>

#include <tamra/mesh/Cell.h>
#include <tamra/mesh/RootCellEntry.h>
#include <tamra/mesh/traversal/CellIdManager.h>
#include <tamra/mesh/Tree.h>


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
  auto cell_a = std::make_shared<Cell2D>(nullptr);

  //Splitting some cells
  unsigned max_level{2};
  cell_a->splitRoot(max_level, cell_a);
  cell_a->getChildCell(2)->split(max_level);

  // Initialize iterator
  std::vector<std::shared_ptr<Cell2D>> roots = { cell_a };
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
  auto cell_a = std::make_shared<Cell2D>(nullptr);

  //Splitting some cells
  unsigned max_level{20};
  auto child_cells = cell_a->splitRoot(max_level, cell_a);
  for (unsigned i{0}; i<(max_level-1); ++i)
    child_cells = child_cells[0]->split(max_level);

  // Initialize iterator
  std::vector<std::shared_ptr<Cell2D>> roots = { cell_a };
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

using PartitionCell1D = Cell<2>;
using PartitionCell2D = Cell<3, 2>;
using PartitionCell3D = Cell<2, 2, 2>;

TEST_CASE_TEMPLATE("[core][manager][cell_id] Equal partitions cover roots and uneven rank counts", CellType,
                   PartitionCell1D, PartitionCell2D, PartitionCell3D) {
  for (unsigned root_count : {1u, 2u, 9u}) {
    CellIdManager<CellType> manager(root_count, 2);
    for (unsigned level : {0u, 1u, 2u}) {
      unsigned cells_per_root = 1;
      for (unsigned l{0}; l < level; ++l)
        cells_per_root *= CellType::child_count;
      const unsigned total_cells = root_count * cells_per_root;
      for (unsigned size : {1u, 2u, 3u, 5u, 17u}) {
        INFO("roots = " << root_count << ", level = " << level << ", ranks = " << size);
        const auto partitions = manager.getEqualPartitions(level, size);
        REQUIRE(partitions.size() == size);
        for (unsigned rank{0}; rank < size; ++rank) {
          const auto path = manager.idToOrderPath(partitions[rank]);
          REQUIRE(path.size() == level + 1);
          CHECK(path[0] < root_count);
          unsigned ordinal = path[0];
          for (unsigned l{1}; l <= level; ++l) {
            CHECK(path[l] < CellType::child_count);
            ordinal = ordinal * CellType::child_count + path[l];
          }
          CHECK(ordinal == rank * total_cells / size);
        }
      }
    }
  }
}

TEST_CASE("[core][manager][cell_id] Deep partition cuts retain exact digits") {
  CellIdManager<PartitionCell1D> manager(1, 80);
  const auto partitions = manager.getEqualPartitions(80, 3);
  const auto one_third = manager.idToOrderPath(partitions[1]);
  const auto two_thirds = manager.idToOrderPath(partitions[2]);
  // Binary expansions 0.010101... and 0.101010... extend beyond double precision.
  for (unsigned level{1}; level <= 80; ++level) {
    CHECK(one_third[level] == (level + 1) % 2);
    CHECK(two_thirds[level] == level % 2);
  }
  CHECK_THROWS_AS(manager.getEqualPartitions(2, 0), std::invalid_argument);
}
