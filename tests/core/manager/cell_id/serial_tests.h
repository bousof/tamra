#include <cmath>
#include <memory>
#include <vector>
#include <core/Cell.h>
#include <core/manager/CellIdManager.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>
#include <testing/UnitTestRegistry.h>

bool cellIDInversion2x2();
bool cellIDInversion3x2();
bool computeEqualPartition();

void registerCoreManagerCellIdSerialTests() {
  UnitTestRegistry::registerSerialTest("Cell ID computation and inversion (2x2)", cellIDInversion2x2, "core/manager/cell_id");
  UnitTestRegistry::registerSerialTest("Cell ID computation and inversion (3x2 deep)", cellIDInversion3x2, "core/manager/cell_id");
  UnitTestRegistry::registerSerialTest("Equal size partition computation", computeEqualPartition, "core/manager/cell_id");
}

// Cell ID computation and inversion (2x2)
//                 ____________________
//                |   | X |           |
//                |___|___|           |
//                |   |   |           |
// structure  ->  |___|___|___________|
//                |       |           |
//                |       |           |
//                |       |           |
//                |_______|___________|
// X is the cell that we test
bool cellIDInversion2x2() {
  using Cell2D = Cell<2,2>;
  // Create 4 root cells
  auto A = std::make_shared<Cell2D>(nullptr);

  //Splitting some cells
  int max_level = 2;
  A->splitRoot(max_level, A);
  A->getChildCell(2)->split(max_level);

  // Initialize iterator
  std::vector< std::shared_ptr<Cell2D> > roots = {A};
  TreeIterator<Cell2D> iterator(roots, max_level);
  
  // Go to target cell 
  iterator.toBegin(max_level);
  for (int i{0}; i<5; ++i)
    iterator.next(max_level);
  const std::vector<unsigned> index_path = iterator.getIndexPath();
  const std::vector<unsigned> index_path_value = {0, 2, 3};

  bool passed = iterator.getIndexPath() == index_path_value;
  passed &= iterator.getCellId() == iterator.indexPathToId(index_path_value);
  passed &= iterator.idToIndexPath(iterator.getCellId()) == index_path_value;
  return passed;
}

// Cell ID computation and inversion (3x2 deep)
//                 ___________________________________ 
//                |           |           |           |
//                |           |           |           |
//                |           |           |           |
// structure  ->  |___________|___________|___________|
//                |   |   |   |           |           |
//                |___|___|___|           |           |
//                | X |   |   |           |           |
//                |___|___|___|___________|___________|
// Recursively split first child of leaf marked X untile level 20
bool cellIDInversion3x2() {
  using Cell2D = Cell<3,2>;
  // Create 4 root cells
  auto A = std::make_shared<Cell2D>(nullptr);

  //Splitting some cells
  int max_level = 20;
  auto child_cells = A->splitRoot(max_level, A);
  for (int i{0}; i<(max_level-1); ++i)
    child_cells = child_cells[0]->split(max_level);

  // Initialize iterator
  std::vector< std::shared_ptr<Cell2D> > roots = {A};
  TreeIterator<Cell2D> iterator(roots, max_level);

  // Go to target cell 
  iterator.toBegin(max_level);
  bool passed = iterator.getIndexPath() == iterator.idToIndexPath(iterator.getCellId());
  while (iterator.next())
    passed = iterator.getIndexPath() == iterator.idToIndexPath(iterator.getCellId());
  return passed;
}

// Equal size partition computation
bool computeEqualPartition() {
  using Cell2D = Cell<2,2>;
  CellIdManager<Cell2D> cell_id_manager(1, 3);
  
  cell_id_manager.getEqualPartitions(2, 3);

  return true;
}
