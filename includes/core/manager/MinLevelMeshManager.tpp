#include "MinLevelMeshManager.h"
//#include "../../utils/display_vector.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType, typename TreeIteratorType>
MinLevelMeshManager<CellType, TreeIteratorType>::MinLevelMeshManager(const unsigned min_level, const unsigned max_level, const unsigned rank, const unsigned size)
: min_level(min_level),
  max_level(max_level),
  rank(rank),
  size(size) {}

// Destructor
template<typename CellType, typename TreeIteratorType>
MinLevelMeshManager<CellType, TreeIteratorType>::~MinLevelMeshManager() {};


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//
// Serial meshing at min level
template<typename CellType, typename TreeIteratorType>
void MinLevelMeshManager<CellType, TreeIteratorType>::meshAtMinLevel(const std::vector<std::shared_ptr<CellType>> &root_cells) const {
  // Looping on all root cells
  for (const auto &root_cell : root_cells)
    // Recursively mesh cells at min level
    serialMeshAtMinLevelRecurs(root_cell);
}

// Parallel meshing at min level
template<typename CellType, typename TreeIteratorType>
void MinLevelMeshManager<CellType, TreeIteratorType>::meshAtMinLevel(const std::vector<std::shared_ptr<CellType>> &root_cells, TreeIteratorType &iterator) const {
  if (size > 1)
    // Construct partitions and refine all cells in the process partition at min level
    parallelMeshAtMinLevel(root_cells, iterator);
  else
    // Meshing at minimum level all roots (no partitioning)
    meshAtMinLevel(root_cells);
}

// Recursively mesh cells at min level
template<typename CellType, typename TreeIteratorType>
void MinLevelMeshManager<CellType, TreeIteratorType>::serialMeshAtMinLevelRecurs(const std::shared_ptr<CellType> &cell) const {
  if (cell->getLevel() < min_level) {
    if (cell->isLeaf())
      cell->split(max_level);
    std::array<std::shared_ptr<CellType>, CellType::number_children> child_cells = cell->getChildCells();

    for (const auto &child : child_cells) {
      serialMeshAtMinLevelRecurs(child);
    }
	}
}

// Mesh all cells in the process partition at min level
template<typename CellType, typename TreeIteratorType>
void MinLevelMeshManager<CellType, TreeIteratorType>::parallelMeshAtMinLevel(const std::vector<std::shared_ptr<CellType>> &root_cells, TreeIteratorType &iterator) const {
  // Set all cells to belong to other process
  for (const auto &root_cell : root_cells)
    root_cell->setToOtherProcRecurs();

  // Compute and share partitions for all process
  std::vector<std::vector<unsigned>> partitions;
  if (rank == 0)
    partitions = iterator.getCellIdManager().getEqualPartitions(min_level, size);;
  unsigned rowCount=size, colCount=iterator.getCellIdManager().getCellIdSize();
  matrixBcast<unsigned>(partitions, 0, rank, rowCount, colCount);

  // Create the first partition cell
  iterator.toCellId(partitions[rank], true);
  bool loop;
  do {
    if ((rank==size-1) || iterator.cellIdLte(partitions[rank+1]))
      if (iterator.getCell()->isLeaf() && (iterator.getCell()->getLevel()<min_level)) {
        iterator.getCell()->split(max_level);
        iterator.toLeaf();
        continue;
      }

    // Set the cell to belong to this proc
    iterator.getCell()->setToThisProc();

    // Move iterator and check if still in partition
    if (rank == size-1)
      loop = iterator.next();
    else
      loop = iterator.next() && !iterator.cellIdGte(partitions[rank+1]);
  } while (loop);

  // Backporpagate belongToThisProc flags
  for (const auto &root_cell : root_cells)
    backPropagateToThisProc(root_cell);
}

// Set a parent to belong to this proc if any of its child do
template<typename CellType, typename TreeIteratorType>
bool MinLevelMeshManager<CellType, TreeIteratorType>::backPropagateToThisProc(const std::shared_ptr<CellType> &cell) const {
  if (cell->isLeaf())
    return cell->belongToThisProc();

  bool to_this_proc = false;
  for (const auto &child : cell->getChildCells())
    if (backPropagateToThisProc(child))
      to_this_proc = true;

  if (to_this_proc)
    cell->setToThisProc();

  return to_this_proc;
}
