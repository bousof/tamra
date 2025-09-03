#include <mpi.h>

#include <core/Cell.h>
#include <core/RootCellEntry.h>
#include <core/Tree.h>
#include <memory>
#include <parallel/allreduce.h>
#include <testing/UnitTestRegistry.h>
#include <vector>

bool ghostOneRoot1DParallel(int rank, int size);
bool ghostOneRootExtrapolateConflict1DParallel(int rank, int size);

void registerCoreManagerGhostParallelTests() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  UnitTestRegistry::label = "P_" + std::to_string(rank) + ": ";
  UnitTestRegistry::registerParallelTest("1D Ghost Cells creation (one root, 1D)", [=]() { return ghostOneRoot1DParallel(rank, size); }, "core/manager/ghost");
  UnitTestRegistry::registerParallelTest("1D Ghost Cells leading to extrapolate conflict (one root, 1D)", [=]() { return ghostOneRootExtrapolateConflict1DParallel(rank, size); }, "core/manager/ghost");
}

// 1D Ghost Cells (one root, 1D)
// X show the leaf cells that belong to each process
// Example for size==2
//                      rank 0             rank 1
//                |       |       |  |       |       |
//                | X | X |       |  |       | X | X |
// structure  ->  |___|___|_______|  |_______|___|___|
// Ghost cells creation should lead to the creation of cells Y
//                      rank 0             rank 1
//                |       |       |  |       |       |
//                |   |   | Y |   |  |   | Y |   |   |
// structure  ->  |___|___|___|___|  |___|___|___|___|
bool ghostOneRoot1DParallel(int rank, int size) {
  using Cell1D = Cell<2>;
  // Create 2 root cells
  auto A = std::make_shared<Cell1D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell1D> eA{A};
  std::vector< RootCellEntry<Cell1D> > entries { eA };

  // Construction of the tree
  int min_level = 1, max_level = 2;
  Tree<Cell1D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Mesh until min level (2)
  A->setToOtherProcRecurs();
  if (rank==0 || rank==1)  {
    A->setToThisProc();
    A->getChildCell(rank)->split(max_level);
    A->getChildCell(rank)->setToThisProcRecurs();
    A->getChildCell(rank)->getChildCell(0)->getCellData().setValue(2*rank);
    A->getChildCell(rank)->getChildCell(1)->getCellData().setValue(2*rank+1);
  }

  // Ghost parent cell should be leaf at that point
  bool passed = true;
  if (rank==0 || rank==1)
    passed &= A->getChildCell((rank+1)%2)->isLeaf();

  // Create ghost cells
  Tree<Cell1D>::GhostManagerTaskType task = tree.buildGhostLayer();

  // Exchange ghost values
  tree.exchangeGhostValues(task);

  // Ghost parent cell Y should not be leaf anymore
  if (rank==0 || rank==1) {
    int otherRank = (rank+1) % 2;
    passed &= !A->getChildCell(otherRank)->isLeaf();
    // Check value
    passed &= A->getChildCell(otherRank)->getChildCell(rank)->getCellData().getValue() == (2*otherRank+rank);
  }

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}

// 1D Ghost Cells leading to extrapolate conflict (one root, 1D)
// X show the leaf cells that belong to each process
// Example for size==2
//                              rank 0                             rank 1
//                |       |       |       |       |  |               |               |
//                |   X   | X | X |       |       |  |               |       X       |
// structure  ->  |_______|___|___|_______|_______|  |_______________|_______________|
//
// In process 0:
// - Cell Y received from process 1 is found splitted.
// - > Ghost cell interpolation conflict.
//                              rank 0
//                |       |       |       |       |
//                |   X   | X | X |   Y   |   Y   |
// structure  ->  |_______|___|___|_______|_______|
//
// In process 1:
// - Cell Y received from process 0 result in a splitting of cells Z to a conflict.
// - > Owned cell interpolation conflict.
//                              rank 1
//                |       |       |       |       |
//                |       |   | Y |   Z   |   Z   |
// structure  ->  |_______|___|___|_______|_______|
bool ghostOneRootExtrapolateConflict1DParallel(int rank, int size) {
  using Cell1D = Cell<2>;
  // Create 2 root cells
  auto A = std::make_shared<Cell1D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell1D> eA{A};
  std::vector< RootCellEntry<Cell1D> > entries { eA };

  // Construction of the tree
  int min_level = 1, max_level = 3;
  Tree<Cell1D> tree(min_level, max_level, rank, size);
  tree.createRootCells(entries);

  // Create the tree structure
  A->setToOtherProcRecurs();
  if (rank==0)  {
    A->setToThisProc();
    A->getChildCell(0)->split(max_level);
    A->getChildCell(0)->getChildCell(1)->split(max_level);
    A->getChildCell(0)->setToThisProcRecurs();
    A->getChildCell(0)->getChildCell(1)->getChildCell(1)->getCellData().setValue(rank);
  }
  if (rank==1)  {
    A->setToThisProc();
    A->getChildCell(1)->setToThisProc();
    A->getChildCell(1)->setToThisProcRecurs();
    A->getChildCell(1)->getCellData().setValue(rank);
  }

  // Create ghost cells
  Tree<Cell1D>::GhostManagerTaskType task = tree.buildGhostLayer();

  // Task should not be finished because of conflicts
  bool passed = !task.is_finished;

  // Set up the task conflict resolution strategy that consist nin copying value to child cells
  auto copyInterpolationFunction = [](const std::shared_ptr<Cell1D> & parent_cell) -> bool {
    // Example: simply duplicate the parent value for each child (placeholder logic)
    for (const auto &child: parent_cell->getChildCells())
      child->getCellData().setValue(parent_cell->getCellData().getValue());
    return true;
  };
  task.setOwnedExtrapolationFunction(copyInterpolationFunction);
  task.setGhostExtrapolationFunction(copyInterpolationFunction);
  task.setOwnedConflictResolutionStrategy({OwnedConflictResolutionStrategy::EXTRAPOLATE});
  task.setGhostConflictResolutionStrategy({GhostConflictResolutionStrategy::EXTRAPOLATE});

  // Exchange ghost values and solve conflicts
  tree.exchangeGhostValues(task);

  // Task should be finished because of conflicts
  passed &= task.is_finished;

  // Ghost parent cell Y should not be leaf anymore
  if (rank==0) {
    int otherRank = 1;
    passed &= !A->getChildCell(1)->isLeaf();
    // Check value
    passed &= A->getChildCell(1)->getChildCell(0)->getCellData().getValue() == (otherRank);
  }
  if (rank==1) {
    int otherRank = 0;
    passed &= !A->getChildCell(0)->isLeaf();
    passed &= !A->getChildCell(0)->getChildCell(1)->isLeaf();
    // Check value
    passed &= A->getChildCell(0)->getChildCell(1)->getChildCell(1)->getCellData().getValue() == (otherRank);
  }

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}
