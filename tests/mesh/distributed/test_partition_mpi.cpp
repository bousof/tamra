#include <doctest.h>

#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

#include <tamra/mesh/Cell.h>
#include <tamra/mesh/RootCellEntry.h>
#include <tamra/mesh/Tree.h>
#include <tamra/mesh/traversal/HilbertIterator.h>
#include <tamra/mesh/traversal/MortonIterator.h>
#include <tamra/parallel/allgatherv.h>
#include <tamra/parallel/wrapper.h>

namespace {

using Partition1D = Tree<Cell<2>>;
using Partition2D = Tree<Cell<2, 2>>;
using Partition3D = Tree<Cell<2, 2, 2>>;
using HilbertPartition1D = Tree<Cell<2>, HilbertIterator<Cell<2>>>;
using HilbertPartition2D = Tree<Cell<2, 2>, HilbertIterator<Cell<2, 2>>>;
using HilbertPartition3D = Tree<Cell<2, 2, 2>, HilbertIterator<Cell<2, 2, 2>>>;

} // namespace

TEST_CASE_TEMPLATE("[core][partition][mpi] Partitions cover every cell once and exchange ghost values", TreeType,
                   Partition1D, Partition2D, Partition3D,
                   HilbertPartition1D, HilbertPartition2D, HilbertPartition3D) {
  using CellType = typename TreeType::CellType;
  using IteratorType = typename TreeType::TreeIteratorType;
  const unsigned rank = mpi_rank(), size = mpi_size();
  for (unsigned root_count : {1u, size + 1}) {
    for (unsigned min_level : {1u, 2u}) {
      INFO("rank = " << rank << ", roots = " << root_count << ", level = " << min_level);
      std::vector<RootCellEntry<CellType>> entries;
      for (unsigned r{0}; r < root_count; ++r)
        entries.push_back(RootCellEntry<CellType>{std::make_shared<CellType>(nullptr)});
      for (unsigned r{1}; r < root_count; ++r) {
        entries[r-1].setNeighbor(1, entries[r].cell);
        entries[r].setNeighbor(0, entries[r-1].cell);
      }
      TreeType tree(min_level, min_level, rank, size);
      tree.createRootCells(entries);
      tree.meshAtMinLevel();

      unsigned total_cells = root_count;
      for (unsigned l{0}; l < min_level; ++l)
        total_cells *= CellType::child_count;
      const unsigned begin = rank * total_cells / size;
      const unsigned end = (rank + 1) * total_cells / size;
      std::vector<unsigned> expected_local(end - begin);
      std::iota(expected_local.begin(), expected_local.end(), begin);

      IteratorType iterator(tree.getRootCells(), tree.getMaxLevel());
      auto cell_ordinal = [&iterator](const auto &cell) {
        const auto path = iterator.idToOrderPath(iterator.getCellId(cell));
        unsigned ordinal = path[0];
        for (std::size_t l{1}; l < path.size(); ++l)
          ordinal = ordinal * CellType::child_count + path[l];
        return ordinal;
      };
      std::vector<unsigned> local_cells, all_cells;
      tree.applyToOwnedLeaves([&](const auto &cell, unsigned) {
        CHECK(cell->getLevel() == min_level);
        const unsigned ordinal = cell_ordinal(cell);
        local_cells.push_back(ordinal);
        cell->getCellData().setValue(ordinal + 1.);
      });
      CHECK(local_cells == expected_local);
      CHECK(tree.countOwnedLeaves() == expected_local.size());
      CHECK(tree.isEmpty() == expected_local.empty());
      vector_allgatherv(local_cells, all_cells, size);
      std::sort(all_cells.begin(), all_cells.end());
      std::vector<unsigned> expected_all(total_cells);
      std::iota(expected_all.begin(), expected_all.end(), 0u);
      CHECK(all_cells == expected_all);

      auto task = tree.buildGhostLayer();
      CHECK(task.is_finished);
      tree.exchangeGhostValues(task);
      for (const auto &ghost : task.getCellsToRecv()) {
        CHECK(ghost->isRemote());
        CHECK(ghost->getLevel() == min_level);
        CHECK(ghost->getCellData().getValue() == cell_ordinal(ghost) + 1.);
      }
      CHECK(tree.countOwnedLeaves() == expected_local.size());
      if (expected_local.empty())
        CHECK(task.getCellsToRecv().empty());
    }
  }
}
