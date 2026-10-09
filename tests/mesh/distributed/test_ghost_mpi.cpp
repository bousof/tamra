#include <doctest.h>

#include <memory>
#include <vector>

#include <tamra/mesh/Cell.h>
#include <tamra/mesh/RootCellEntry.h>
#include <tamra/mesh/Tree.h>
#include <tamra/parallel/allreduce.h>
#include <tamra/parallel/wrapper.h>


// 1D Ghost Cells (one root, 1D)
// X show the leaf cells that belong to each process
// Example for size==2
//                      rank 0             rank 1
//                │       │       │  │       |       │
// structure  ->  │ X │ X │       │  │       | X │ X │
//                └───┴───┴───────┘  └───────┴───┴───┘
// Ghost cells creation should lead to the creation of cells Y
//                      rank 0             rank 1
//                │       │       │  │       │       │
// structure  ->  │   │   │ Y │   │  │   │ Y │   │   │
//                └───┴───┴───┴───┘  └───┴───┴───┴───┘
TEST_CASE("[core][manager][ghost][mpi] 1D Ghost Cells creation (one root, 1D)") {
  using Cell1D = Cell<2>;
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  // Create 2 root cells
  auto cell_a = std::make_shared<Cell1D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell1D> entry_a{cell_a};
  std::vector<RootCellEntry<Cell1D>> entries { entry_a };

  // Construction of the tree
  unsigned min_level{1}, max_level{2};
  Tree<Cell1D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  cell_a->markRemoteRecurs();
  if (rank==0 || rank==1)  {
    cell_a->markOwned();
    cell_a->getChildCell(rank)->split(max_level);
    cell_a->getChildCell(rank)->markOwnedRecurs();
    cell_a->getChildCell(rank)->getChildCell(0)->getCellData().setValue(2*rank);
    cell_a->getChildCell(rank)->getChildCell(1)->getCellData().setValue(2*rank+1);
  }

  // Ghost parent cell should be leaf at that point
  bool passed = true;
  if (rank==0 || rank==1)
    passed &= cell_a->getChildCell((rank+1)%2)->isLeaf();

  // Create ghost cells
  Tree<Cell1D>::GhostManagerTaskType task = tree.buildGhostLayer();

  // Exchange ghost values
  tree.exchangeGhostValues(task);

  // Ghost parent cell Y should not be leaf anymore
  if (rank==0 || rank==1) {
    unsigned other_rank = (rank+1) % 2;
    passed &= !cell_a->getChildCell(other_rank)->isLeaf();
    // Check value
    passed &= cell_a->getChildCell(other_rank)->getChildCell(rank)->getCellData().getValue() == (2*other_rank+rank);
  }

  // Test should pass on all processes
  bool all_passed;
  bool_and_allreduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}

// 1D Ghost Cells leading to extrapolate conflict (one root, 1D)
// X show the leaf cells that belong to each process
// Example for size==2
//                              rank 0                             rank 1
//                │       │       │       │       │  │               │               │
// structure  ->  │   X   │ X │ X │       │       │  │               │       X       │
//                └───────┴───┴───┴───────┴───────┘  └───────────────┴───────────────┘
//
// In process 0:
// - Cell Y received from process 1 is found splitted.
// - > Ghost cell extrapolation conflict.
//                              rank 0
//                │       │       │       │       │
// structure  ->  │   X   │ X │ X │   Y   │   Y   │
//                └───────┴───┴───┴───────┴───────┘
//
// In process 1:
// - Cell Y received from process 0 result in a splitting of cells Z to a conflict.
// - > Owned cell extrapolation conflict.
//                              rank 1
//                │       │       │       │       │
// structure  ->  │       │   │ Y │   Z   │   Z   │
//                └───────┴───┴───┴───────┴───────┘
TEST_CASE("[core][manager][ghost][mpi] 1D Ghost Cells leading to extrapolate conflict (one root, 1D)") {
  using Cell1D = Cell<2>;
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  // Create 2 root cells
  auto cell_a = std::make_shared<Cell1D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell1D> entry_a{cell_a};
  std::vector<RootCellEntry<Cell1D>> entries { entry_a };

  // Construction of the tree
  unsigned min_level{1}, max_level{3};
  Tree<Cell1D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Create the tree structure
  cell_a->markRemoteRecurs();
  if (rank == 0)  {
    cell_a->markOwned();
    cell_a->getChildCell(0)->split(max_level);
    cell_a->getChildCell(0)->getChildCell(1)->split(max_level);
    cell_a->getChildCell(0)->markOwnedRecurs();
    cell_a->getChildCell(0)->getChildCell(1)->getChildCell(1)->getCellData().setValue(rank);
  }
  if (rank == 1)  {
    cell_a->markOwned();
    cell_a->getChildCell(1)->markOwned();
    cell_a->getChildCell(1)->markOwnedRecurs();
    cell_a->getChildCell(1)->getCellData().setValue(rank);
  }

  // Create ghost cells
  Tree<Cell1D>::GhostManagerTaskType task = tree.buildGhostLayer();

  // Task should not be finished because of conflicts
  bool passed = !task.is_finished;

  // Set up the task conflict resolution strategy that consist nin copying value to child cells
  auto copy_parent_to_children = [](const std::shared_ptr<Cell1D> &parent_cell) -> bool {
    // Example: simply duplicate the parent value for each child (placeholder logic)
    for (const auto &child : parent_cell->getChildCells())
      child->getCellData().setValue(parent_cell->getCellData().getValue());
    return true;
  };
  task.setOwnedExtrapolationFunction(copy_parent_to_children);
  task.setGhostExtrapolationFunction(copy_parent_to_children);
  task.setOwnedConflictResolutionStrategy({ OwnedConflictResolutionStrategy::EXTRAPOLATE });
  task.setGhostConflictResolutionStrategy({ GhostConflictResolutionStrategy::EXTRAPOLATE });

  // Exchange ghost values and solve conflicts
  tree.exchangeGhostValues(task);

  // Task should be finished because of conflicts
  passed &= task.is_finished;

  // Ghost parent cell Y should not be leaf anymore
  if (rank == 0) {
    unsigned other_rank = 1;
    passed &= !cell_a->getChildCell(1)->isLeaf();
    // Check value
    passed &= cell_a->getChildCell(1)->getChildCell(0)->getCellData().getValue() == (other_rank);
  }
  if (rank == 1) {
    unsigned other_rank = 0;
    passed &= !cell_a->getChildCell(0)->isLeaf();
    passed &= !cell_a->getChildCell(0)->getChildCell(1)->isLeaf();
    // Check value
    passed &= cell_a->getChildCell(0)->getChildCell(1)->getChildCell(1)->getCellData().getValue() == (other_rank);
  }

  // Test should pass on all processes
  bool all_passed;
  bool_and_allreduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}
