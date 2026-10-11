#include <doctest.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <tamra/mesh/Cell.h>
#include <tamra/mesh/RootCellEntry.h>
#include <tamra/mesh/Tree.h>
#include <tamra/mesh/traversal/HilbertIterator.h>
#include <tamra/mesh/traversal/MortonIterator.h>

namespace {

template<typename CellType, typename IteratorType>
void check_traversal(const std::shared_ptr<CellType> &root, IteratorType &iterator) {
  const unsigned expected_count = 2 * CellType::child_count - 1;
  std::vector<std::shared_ptr<CellType>> forward, reverse, expected_owned, owned;
  iterator.toBegin();
  do {
    const auto cell = iterator.getCell();
    CHECK(cell->isLeaf());
    forward.push_back(cell);
  } while (forward.size() <= expected_count && iterator.next());
  CHECK(forward.size() == expected_count);
  const std::set<std::shared_ptr<CellType>> unique_cells(forward.begin(), forward.end());
  CHECK(unique_cells.size() == expected_count);

  iterator.toLast();
  do {
    reverse.push_back(iterator.getCell());
  } while (reverse.size() <= expected_count && iterator.prev());
  std::reverse(reverse.begin(), reverse.end());
  CHECK(reverse == forward);

  for (const auto &cell : forward) {
    const auto id = iterator.getCellId(cell);
    iterator.toCellId(id);
    CHECK(iterator.getCell() == cell);
    CHECK(iterator.getCellId() == id);
  }

  // Ownership partitions are contiguous in this iterator's curve order.
  REQUIRE(forward.size() == expected_count);
  forward.front()->markRemote();
  forward.back()->markRemote();
  expected_owned.assign(forward.begin() + 1, forward.end() - 1);
  if (iterator.toOwnedBegin()) {
    do {
      owned.push_back(iterator.getCell());
    } while (owned.size() <= expected_count && iterator.ownedNext());
  }
  CHECK(owned == expected_owned);
  owned.clear();
  if (iterator.toOwnedEnd()) {
    do {
      owned.push_back(iterator.getCell());
    } while (owned.size() <= expected_count && iterator.ownedPrev());
  }
  std::reverse(owned.begin(), owned.end());
  CHECK(owned == expected_owned);

  root->markRemoteRecurs();
  CHECK_FALSE(iterator.toOwnedBegin());
  CHECK_FALSE(iterator.toOwnedEnd());
  root->markOwnedRecurs();
}

} // namespace

using TraversalCell1D = Cell<2>;
using TraversalCell2D = Cell<2, 2>;
using TraversalCell3D = Cell<2, 2, 2>;

TEST_CASE_TEMPLATE("[core][traversal] Morton adaptive traversal and ownership", CellType,
                   TraversalCell1D, TraversalCell2D, TraversalCell3D) {
  auto root = std::make_shared<CellType>(nullptr);
  Tree<CellType> tree(1, 2);
  tree.createRootCells({RootCellEntry<CellType>{root}});
  root->getChildCell(0)->split(2);
  MortonIterator<CellType> iterator(tree.getRootCells(), 2);
  check_traversal(root, iterator);
}

TEST_CASE_TEMPLATE("[core][traversal] Hilbert adaptive traversal and ownership", CellType,
                   TraversalCell1D, TraversalCell2D, TraversalCell3D) {
  auto root = std::make_shared<CellType>(nullptr);
  Tree<CellType> tree(1, 2);
  tree.createRootCells({RootCellEntry<CellType>{root}});
  root->getChildCell(0)->split(2);
  HilbertIterator<CellType> iterator(tree.getRootCells(), 2);
  check_traversal(root, iterator);
}

TEST_CASE_TEMPLATE("[core][traversal] Hilbert uniform traversal connects face neighbors", CellType,
                   TraversalCell1D, TraversalCell2D, TraversalCell3D) {
  auto root = std::make_shared<CellType>(nullptr);
  Tree<CellType> tree(1, 2);
  tree.createRootCells({RootCellEntry<CellType>{root}});
  for (const auto &child : root->getChildCells())
    child->split(2);

  // Build positions directly from topology, independently of iterator IDs.
  std::map<const CellType *, std::array<int, 3>> coordinates;
  for (const auto &child : root->getChildCells())
    for (const auto &grandchild : child->getChildCells()) {
      std::array<int, 3> position{};
      for (unsigned dimension{0}; dimension < CellType::dimension_count; ++dimension)
        position[dimension] = 2 * ((child->getSiblingNumber() >> dimension) & 1u)
                              + ((grandchild->getSiblingNumber() >> dimension) & 1u);
      coordinates.emplace(grandchild.get(), position);
    }

  HilbertIterator<CellType> iterator(tree.getRootCells(), 2);
  std::set<std::array<int, 3>> positions;
  std::array<int, 3> previous{};
  iterator.toBegin();
  const unsigned expected_count = CellType::child_count * CellType::child_count;
  unsigned visited = 0;
  do {
    const auto position = coordinates.at(iterator.getCell().get());
    if (visited != 0) {
      int distance = 0;
      for (unsigned dimension{0}; dimension < CellType::dimension_count; ++dimension)
        distance += std::abs(position[dimension] - previous[dimension]);
      CHECK(distance == 1);
    }
    CHECK(positions.insert(position).second);
    previous = position;
    ++visited;
  } while (visited <= expected_count && iterator.next());
  CHECK(visited == expected_count);
}
