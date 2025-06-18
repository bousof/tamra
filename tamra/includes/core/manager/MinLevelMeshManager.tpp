#include "./MinLevelMeshManager.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType, typename TreeIteratorType>
MinLevelMeshManager<CellType, TreeIteratorType>::MinLevelMeshManager(const int min_level, const int max_level, const int rank, const int size)
: min_level(min_level), max_level(max_level), rank(rank), size(size) {}

// Destructor
template<typename CellType, typename TreeIteratorType>
MinLevelMeshManager<CellType, TreeIteratorType>::~MinLevelMeshManager() {};


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//
// Serial meshing at min level
template<typename CellType, typename TreeIteratorType>
void MinLevelMeshManager<CellType, TreeIteratorType>::meshAtMinLevel(const std::vector< std::shared_ptr<CellType> >& root_cells) {
  // Looping on all root cells
  for (const auto &root_cell: root_cells)
    // Recursively mesh cells at min level
    serialMeshAtMinLevelRecurs(root_cell);
}

// Parallel meshing at min level
template<typename CellType, typename TreeIteratorType>
void MinLevelMeshManager<CellType, TreeIteratorType>::meshAtMinLevel(const std::vector< std::shared_ptr<CellType> >& root_cells, TreeIteratorType &iterator) {
  if (size>1) {
    // Construct partitions and refine all cells in the process partition at min level
    parallelMeshAtMinLevel(root_cells, iterator);
  } else
    // Meshing at minimum level all roots (no partitioning)
    meshAtMinLevel(root_cells);
}

// Get the partition (start and end position in the SFC) that belong to this process
template<typename CellType, typename TreeIteratorType>
std::pair<int, int> MinLevelMeshManager<CellType, TreeIteratorType>::getParallelPartition(const unsigned number_root_cells) {
  // We determine the start and the end of the partition
  int total_cells_min_level = number_root_cells * static_cast<int>(pow(CellType::number_children, min_level));
  int partition_size_min_level = static_cast<int>(pow(CellType::number_children, max_level - min_level));

  // The number of cells at minimum level must be >= to the number of processes
  if (size > total_cells_min_level)
    throw std::runtime_error("To feww cells at nim level in MinLevelMeshManager::meshAtMinLevel()");

  // Define the partitions for each processor
  int partition_start = 0,
      partition_end = total_cells_min_level;
  if (rank != 0)
    partition_start = ceil( (static_cast<double>(rank)/size) * ((double) total_cells_min_level) );
  if (rank != (size-1))
    partition_end = ceil( (static_cast<double>(rank+1)/size) * ((double) total_cells_min_level) );
  if (partition_end < partition_start)
    partition_start = partition_end;
  partition_start = partition_start * partition_size_min_level;
  partition_end   = partition_end * partition_size_min_level-1;

  return std::make_pair(partition_start, partition_end);
}

// Recursively mesh cells at min level
template<typename CellType, typename TreeIteratorType>
void MinLevelMeshManager<CellType, TreeIteratorType>::serialMeshAtMinLevelRecurs(const std::shared_ptr<CellType>& cell) {
  if (cell->getLevel() < min_level) {
    if (cell->isLeaf())
      cell->split(max_level);
    std::array< std::shared_ptr<CellType>, CellType::number_children > child_cells = cell->getChildCells();

    for (const auto &child: child_cells) {
      serialMeshAtMinLevelRecurs(child);
    }
	}
}

// Mesh all cells in the process partition at min level
template<typename CellType, typename TreeIteratorType>
void MinLevelMeshManager<CellType, TreeIteratorType>::parallelMeshAtMinLevel(const std::vector< std::shared_ptr<CellType> >& root_cells, TreeIteratorType &iterator) {
  // Set all cells to belong to other process
  for (const auto &root_cell: root_cells)
    root_cell->setToOtherProcRecurs();

  // Compute and share partitions for all process
  std::vector< std::vector<unsigned> > partitions;
  if (rank==0) {
    partitions = iterator.getCellIdManager().getEqualPartitions(min_level, size);;
  }
  int rowCount=size, colCount=iterator.getCellIdManager().getCellIdSize();
  matrixUnsignedBCast(partitions, 0, rank, rowCount, colCount);

  // Create the first partition cell
  iterator.toCellId(partitions[rank], true);
  bool loop;
  do {
    if (rank==size-1 || iterator.cellIdLte(partitions[rank+1])) {
      if (iterator.getCell()->isLeaf() && iterator.getCell()->getLevel()<min_level) {
        iterator.getCell()->split(max_level);
        iterator.toLeaf();
        continue;
      }
    }

    // Set the cell to belong to this proc
    iterator.getCell()->setToThisProc();

    // Move iterator and check if still in partition
    if (rank==size-1)
      loop = iterator.next();
    else
      loop = iterator.next() && !iterator.cellIdGte(partitions[rank+1]);
  } while (loop);

  // Backporpagate belongToThisProc flags
  for (const auto &root_cell: root_cells)
    backPropagateToThisProc(root_cell);
}

// Set a parent to belong to this proc if any of its child do
template<typename CellType, typename TreeIteratorType>
bool MinLevelMeshManager<CellType, TreeIteratorType>::backPropagateToThisProc(const std::shared_ptr<CellType>& cell) {
  if (cell->isLeaf())
    return cell->belongToThisProc();

  bool to_this_proc = false;
  for (const auto &child: cell->getChildCells())
    if (backPropagateToThisProc(child))
      to_this_proc = true;

  if (to_this_proc)
    cell->setToThisProc();

  return to_this_proc;
}
