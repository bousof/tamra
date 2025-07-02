#include "./GhostManager.h"
//#include "../../../display/display_vector.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType, typename TreeIteratorType>
GhostManager<CellType, TreeIteratorType>::GhostManager(const int min_level, const int max_level, const int rank, const int size)
: min_level(min_level),
  max_level(max_level),
  rank(rank),
  size(size) {}

// Destructor
template<typename CellType, typename TreeIteratorType>
GhostManager<CellType, TreeIteratorType>::~GhostManager() {};


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Creation of ghost cells and exchange of ghost values
template<typename CellType, typename TreeIteratorType>
typename GhostManager<CellType, TreeIteratorType>::GhostManagerTaskType GhostManager<CellType, TreeIteratorType>::buildGhostLayer(std::vector< std::shared_ptr<CellType> >& root_cells, TreeIteratorType &iterator) const {
  // Number of leaf cells before creating ghost.
	unsigned old_nb_owned_leaves = 0;
  for (const auto &root_cell: root_cells)
    old_nb_owned_leaves += root_cell->countOwnedLeaves();
	// If the creation of ghost cells leads to the splitting of cells inside the partition of
	// the proc, then it will be necessary to reapply the ghost cell creation process in
	// order to be sure about the correctness of the tree after having those new cell values.

  // Set all ghost cells to coarse
  for (const auto &root_cell: root_cells)
    setGhostToCoarseRecurs(root_cell);

  //std::cout << "P_" << rank << ": nb owned leaves " << old_nb_owned_leaves << std::endl;

  // Share the partitions start and end IDs to be able to determine what cell belongs to what process
  std::vector< std::vector<unsigned> > begin_ids, end_ids;
  sharePartitions(begin_ids, end_ids, iterator);

  //std::cout << "P_" << rank << ": start ids";
  //displayVector(std::cout, begin_ids) << std::endl;
  //std::cout << "P_" << rank << ": end ids";
  //displayVector(std::cout, end_ids) << std::endl;

  // Loop on owned cells and check if neighbors belong to another process
  std::vector< std::vector<std::shared_ptr<CellType>> > cells_to_send;
  findCellsToSend(root_cells, begin_ids, end_ids, cells_to_send, iterator);

  // Share cell IDs of the cells to create on other process
  std::vector< std::vector<std::vector<unsigned>> > cell_ids_to_send(size);
  for (int p{0}; p<size; ++p) {
    cell_ids_to_send[p].reserve(cells_to_send[p].size());
    for (const std::shared_ptr<CellType> &cell: cells_to_send[p])
      cell_ids_to_send[p].push_back(iterator.getCellId(cell));
  }

  //std::cout << "P_" << rank << ": send cell ids ";
  //displayVector(std::cout, cell_ids_to_send) << std::endl;

  // Gather cell data in a vector for sharing between process
  std::vector< std::vector< std::unique_ptr<ParallelData> > > all_cell_data(size);
  {
    for (unsigned p{0}; p<size; ++p) {
      all_cell_data[p].reserve(cells_to_send[p].size());
      for (unsigned i{0}; i<cells_to_send[p].size(); ++i)
        all_cell_data[p].push_back(std::make_unique<typename CellType::CellDataType>(cells_to_send[p][i]->getCellData()));
    }
  }

  const unsigned cell_id_size = iterator.getCellIdManager().getCellIdSize();
  std::vector< std::vector<unsigned> > recv_cell_ids;
  matrixUnsignedAlltoallv(cell_ids_to_send, recv_cell_ids, size, cell_id_size);

  // Exchange cell data
  std::vector< std::unique_ptr<ParallelData> > all_cell_data_recv;
  vectorDataAlltoallv(all_cell_data, all_cell_data_recv, size, []() {
    return std::make_unique<typename CellType::CellDataType>();
  });

  //std::cout << "P_" << rank << ": recv cell ids ";
  //displayVector(std::cout, recv_cell_ids) << std::endl;

  std::vector<std::shared_ptr<CellType>> extrapolate_ghost_cells;
  for (int i{0}; i<recv_cell_ids.size(); ++i) {
    // Move iterator to cell ID and create it if needed
    iterator.toCellId(recv_cell_ids[i], true);

    // Set cell data
    iterator.getCell()->setCellData(std::unique_ptr<typename CellType::CellDataType>(
      static_cast<typename CellType::CellDataType*>(all_cell_data_recv[i].release())
    ));

    // If cell is alraedy split, ghost extrapolation should be done to set child values
    if (!iterator.getCell()->isLeaf())
      extrapolate_ghost_cells.push_back(iterator.getCell());
  }

  // Number of leaf cells befor creating ghost.
	unsigned nb_owned_leaves = 0;
  for (const auto &root_cell: root_cells)
    nb_owned_leaves += root_cell->countOwnedLeaves();

  // If number of owned cells has increased. Find cells that need to be extrapolated::
  std::vector<std::shared_ptr<CellType>> extrapolate_owned_cells;
  if (nb_owned_leaves > old_nb_owned_leaves)
    for (int p{0}; p<size; ++p)
      for (const std::shared_ptr<CellType> &cell: cells_to_send[p])
        if (!cell->isLeaf())
          extrapolate_owned_cells.push_back(cell);

  // Check if all process have finished
  bool is_finished = extrapolate_owned_cells.size()==0 && extrapolate_ghost_cells.size()==0;
  boolAndAllReduce(is_finished, is_finished);

  if (is_finished) // Create a finished task (no need to keep a copy of anything)
    return GhostManagerTaskType(*this, is_finished);
  else // Create an unfinished task (keep a copy of arrays needed for finishing the task)
    return GhostManagerTaskType(*this, is_finished, std::move(cells_to_send), std::move(extrapolate_owned_cells), std::move(extrapolate_ghost_cells), std::move(begin_ids), std::move(end_ids));
}

// Update ghost cells and exchange values for solving conflicts
template<typename CellType, typename TreeIteratorType>
void GhostManager<CellType, TreeIteratorType>::updateGhostLayer(GhostManagerTaskType &task, TreeIteratorType &iterator) const {
  // This method handles the resend logic when conflicts are resolved
  // It loops through children of owned cells that were split and checks their neighbors
  // TODO: Implement this
}

// Share the partiion start and end cells
template<typename CellType, typename TreeIteratorType>
void GhostManager<CellType, TreeIteratorType>::sharePartitions(std::vector< std::vector<unsigned> > &begin_ids, std::vector< std::vector<unsigned> > &end_ids, TreeIteratorType &iterator) const {
  std::shared_ptr<CellType> begin_cell, end_cell;
  std::vector<unsigned> begin_id, end_id;
  if (iterator.toOwnedBegin()) { // Partition is not empty
    begin_cell = iterator.getCell();
    begin_id = iterator.getCellId();
    iterator.toOwnedEnd();
    end_cell = iterator.getCell();
    end_id = iterator.getCellId();
  } else { // If partition is empty we set start > end
    iterator.toEnd();
    begin_cell = iterator.getCell();
    begin_id = iterator.getCellId();
    iterator.toBegin();
    end_cell = iterator.getCell();
    end_id = iterator.getCellId();
  }

  // All gather the start and end of each process partition
  std::vector< std::vector<unsigned> > send_partition_ids= { begin_id, end_id }, all_partition_ids;
  matrixUnsignedAllgather(send_partition_ids, all_partition_ids, size);

  // Format outputs
  begin_ids.resize(size);
  end_ids.resize(size);
  for (int p{0}; p<size; ++p) {
    begin_ids[p] = all_partition_ids[2*p];
    end_ids[p] = all_partition_ids[2*p+1];
  }
}

// Loop on owned cells and check if neighbors belong to another process
template<typename CellType, typename TreeIteratorType>
void GhostManager<CellType, TreeIteratorType>::findCellsToSend(const std::vector< std::shared_ptr<CellType> > &root_cells, const std::vector< std::vector<unsigned> > &begin_ids, const std::vector< std::vector<unsigned> > &end_ids, std::vector< std::vector<std::shared_ptr<CellType>> > &cells_to_send, TreeIteratorType &iterator) const {
  // Cell ID manager
  typename TreeIteratorType::CellIdManagerType cell_id_manager = iterator.getCellIdManager();

  // Rest output arrays
  cells_to_send.resize(size);
  for (int p{0}; p<size; ++p)
    cells_to_send[p].resize(0);

  // If partition is empty no cells to be shared
  if (!iterator.toOwnedBegin())
    return;

  // Get
  // Non void partitions processes
  std::vector<int> non_void_proc;
  non_void_proc.reserve(size);
  for (int p{0}; p<size; ++p)
    if (p!=rank && cell_id_manager.cellIdLte(begin_ids[p], end_ids[p]))
      non_void_proc.push_back(p);

  //for (const int p: non_void_proc)
  //  std::cout << "P_" << rank << ": " << p << " non void" << std::endl;

  // Main loop on cells in partition
  std::shared_ptr<CellType> cell, neighbor_cell;
  std::vector<unsigned> neighbor_id;
  std::vector<bool> found(size);
  do {
    cell = iterator.getCell();

    // Loop on cell's neighbors
    // Connect the neighbors in the root's child_oct
    std::fill(found.begin(), found.end(), false);
    bool allTrue;
    for (int dir = 0; dir < CellType::number_neighbors; ++dir) {
      // If cell already shared with all proc no need to add it again
      allTrue = true;
      for (const int p: non_void_proc)
        allTrue &= found[p];
      if (allTrue)
        break;

      // Get the neighbor cell ID
      neighbor_cell = cell->getNeighborCell(dir);
      if (!neighbor_cell || neighbor_cell->belongToThisProc())
        continue;
      neighbor_id = iterator.getCellId(neighbor_cell);

      //std::cout << "P_" << rank << " dir " << dir << " -> ";
      //displayVector(std::cout, neighbor_id) << std::endl;

      // Check if the neighbor cell cross other process partitions
      for (const int p: non_void_proc)
        if (!found[p] &&
            !cell_id_manager.cellIdGt(begin_ids[p], neighbor_id) &&
            !cell_id_manager.cellIdGt(neighbor_id, end_ids[p])) {
          //std::cout << "P_" << rank << " proc " << p << " needs ";
          //displayVector(std::cout, iterator.getCellId()) << std::endl;
          cells_to_send[p].push_back(cell);
          found[p] = true;
          break;
        }
    }
  } while (iterator.ownedNext());
}

// Set all ghost cells to coarse
template<typename CellType, typename TreeIteratorType>
void GhostManager<CellType, TreeIteratorType>::setGhostToCoarseRecurs(const std::shared_ptr<CellType> &cell) const {
  if (!cell->belongToThisProc())
    cell->setToCoarseRecurs();
  else if (!cell->isLeaf())
    for (const auto &child: cell->getChildCells())
      setGhostToCoarseRecurs(child);
}
