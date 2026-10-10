#include <doctest.h>

#include <algorithm>
#include <memory>
#include <vector>

#include <tamra/mesh/Cell.h>
#include <tamra/mesh/Tree.h>
#include <tamra/mesh/distributed/LoadBalanceManager.h>
#include <tamra/mesh/traversal/MortonIterator.h>
#include <tamra/parallel/allgatherv.h>
#include <tamra/parallel/wrapper.h>

namespace {

class FractionalLoadData : public CellData {
 public:
  double getLoad(bool is_leaf, const std::shared_ptr<void> = nullptr) const override {
    return is_leaf ? 0.03125 : 0.;
  }
};

} // namespace

TEST_CASE("[parallel][balance] Equal and zero loads do not request balancing") {
  using CellType = Cell<2>;
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  LoadBalanceManager<CellType, MortonIterator<CellType>> manager(1, 2, rank, size);
  auto root = std::make_shared<CellType>(nullptr);
  root->splitRoot(2, root);

  const auto balanced = manager.evaluateLoadBalance({root}, 0.);
  CHECK_FALSE(balanced.first);
  if (rank == 0)
    CHECK(balanced.second == std::vector<double>(size, 2.));
  else
    CHECK(balanced.second.empty());

  const auto empty = manager.evaluateLoadBalance({}, 0.);
  CHECK_FALSE(empty.first);
  if (rank == 0)
    CHECK(empty.second == std::vector<double>(size, 0.));

  root->markRemoteRecurs();
  const auto remote = manager.evaluateLoadBalance({root}, 0.);
  CHECK_FALSE(remote.first);
  if (rank == 0)
    CHECK(remote.second == std::vector<double>(size, 0.));
}

TEST_CASE("[parallel][balance] Threshold decision is shared by every rank") {
  using CellType = Cell<2>;
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  LoadBalanceManager<CellType, MortonIterator<CellType>> manager(1, 2, rank, size);
  auto root = std::make_shared<CellType>(nullptr);
  root->splitRoot(2, root);
  if (rank != size - 1)
    root->markRemoteRecurs();

  // One rank has load 2; the relative imbalance is exactly size - 1.
  const auto unbalanced = manager.evaluateLoadBalance({root}, 0.);
  CHECK(unbalanced.first == (size > 1));
  if (rank == 0) {
    std::vector<double> expected(size, 0.);
    expected.back() = 2.;
    CHECK(unbalanced.second == expected);
  }
  const auto tolerated = manager.evaluateLoadBalance({root}, static_cast<double>(size - 1));
  CHECK_FALSE(tolerated.first);
  const auto above_threshold = manager.evaluateLoadBalance({root}, static_cast<double>(size));
  CHECK_FALSE(above_threshold.first);
}

namespace {

template<typename DataType>
std::vector<double> migrated_values() {
  using CellType = Cell<2, 2, 0, DataType>;
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  auto root = std::make_shared<CellType>(nullptr);
  Tree<CellType> tree(1, 2, rank, size);
  tree.createRootCells({RootCellEntry<CellType>{root}});
  for (const auto &child : root->getChildCells())
    child->split(2);
  tree.applyToOwnedLeaves([rank, size](const auto &cell, const unsigned index) {
    cell->getCellData().setValue(rank == size - 1 ? static_cast<double>(index) : -99.);
  });
  if (rank != size - 1)
    root->markRemoteRecurs();

  tree.loadBalance();
  std::vector<double> local_values, all_values;
  tree.applyToOwnedLeaves([&local_values](const auto &cell, const unsigned) {
    local_values.push_back(cell->getCellData().getValue());
  });
  vector_allgatherv(local_values, all_values, size);
  std::sort(all_values.begin(), all_values.end());
  std::vector<double> expected;
  for (unsigned i{0}; i < 16; ++i)
    expected.push_back(static_cast<double>(i));
  CHECK(all_values == expected);
  // The existing partition rule permits a one-cell offset at a cut.
  CHECK(local_values.size() + 1 >= 16 / size);
  CHECK(local_values.size() <= (16 + size - 1) / size + 1);

  // Allow that one-cell imbalance and verify that a tolerated partition stays put.
  tree.loadBalance([](const auto &) {}, static_cast<double>(size) / 16.);
  std::vector<double> after;
  tree.applyToOwnedLeaves([&after](const auto &cell, const unsigned) {
    after.push_back(cell->getCellData().getValue());
  });
  CHECK(after == local_values);
  return local_values;
}

} // namespace

TEST_CASE("[parallel][balance] Fractional loads preserve cells and values during migration") {
  // FractionalLoadData gives the entire tree load 0.5. Integer accumulation
  // would erase it; scaling all cell loads must not change the partition.
  const auto fractional = migrated_values<FractionalLoadData>();
  const auto integral = migrated_values<CellData>();
  CHECK(fractional == integral);
}
