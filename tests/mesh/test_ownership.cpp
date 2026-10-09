#include <doctest.h>

#include <memory>

#include <tamra/mesh/Cell.h>
#include <tamra/mesh/RootCellEntry.h>
#include <tamra/mesh/Tree.h>

using OwnershipCell1D = Cell<2>;
using OwnershipCell2D = Cell<2, 2>;
using OwnershipCell3D = Cell<2, 2, 2>;

TEST_CASE_TEMPLATE("[core][ownership] Ghost counts include remote leaves below owned parents", CellType,
                   OwnershipCell1D, OwnershipCell2D, OwnershipCell3D) {
  auto root = std::make_shared<CellType>(nullptr);
  auto remote_root = std::make_shared<CellType>(nullptr);
  Tree<CellType> tree(1, 2);
  tree.createRootCells({RootCellEntry<CellType>{root}, RootCellEntry<CellType>{remote_root}});
  root->getChildCell(0)->split(2);
  root->getChildCell(0)->getChildCell(0)->markRemote();
  root->getChildCell(1)->markRemote();
  remote_root->markRemoteRecurs();

  CHECK(root->isOwned());
  CHECK(root->countGhostLeaves() == 2); // FAIL HERE
  CHECK(root->countOwnedLeaves() == 2 * CellType::child_count - 3);
  CHECK(remote_root->countGhostLeaves() == CellType::child_count);
  CHECK(remote_root->countOwnedLeaves() == 0);
  CHECK(tree.countGhostLeaves() == CellType::child_count + 2);
  CHECK(tree.countOwnedLeaves() == 2 * CellType::child_count - 3);
  CHECK(tree.countGhostLeaves() + tree.countOwnedLeaves()
        == root->countLeaves() + remote_root->countLeaves());

  root->markOwnedRecurs();
  remote_root->markOwnedRecurs();
  CHECK(tree.countGhostLeaves() == 0);
  CHECK(tree.countOwnedLeaves() == root->countLeaves() + remote_root->countLeaves());
}

TEST_CASE("[core][ownership] Leaf flags preserve adaptation and exclude boundary cells") {
  Cell<> leaf(nullptr);
  CHECK(leaf.countOwnedLeaves() == 1);
  CHECK(leaf.countGhostLeaves() == 0);
  leaf.markForRefinement();
  leaf.markRemote();
  CHECK(leaf.isMarkedForRefinement());
  CHECK(leaf.countOwnedLeaves() == 0);
  CHECK(leaf.countGhostLeaves() == 1);
  leaf.markForCoarsening();
  leaf.markOwned();
  CHECK(leaf.isMarkedForCoarsening());
  leaf.clearAdaptationFlag();
  CHECK_FALSE(leaf.isMarkedForCoarsening());
  CHECK_FALSE(leaf.isMarkedForRefinement());
  CHECK(leaf.isOwned());

  Cell<> boundary(nullptr, 6);
  CHECK(boundary.isBoundaryCell());
  CHECK(boundary.countOwnedLeaves() == 0);
  CHECK(boundary.countGhostLeaves() == 0);
}
