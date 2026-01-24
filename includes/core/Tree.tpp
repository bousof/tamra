#include "Tree.h"
#include "../utils/display_vector.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType, typename TreeIteratorType>
Tree<CellType, TreeIteratorType>::Tree(const unsigned min_level, const unsigned max_level, const unsigned rank, const unsigned size)
: min_level(min_level),
  max_level(max_level),
  rank(rank),
  size(size),
  balanceManager(min_level, max_level, rank, size),
  coarseManager(min_level, max_level, rank, size),
  ghostManager(min_level, max_level, rank, size),
  minLevelMeshManager(min_level, max_level, rank, size),
  refineManager(min_level, max_level, rank, size) {}

// Destructor
template<typename CellType, typename TreeIteratorType>
Tree<CellType, TreeIteratorType>::~Tree() {
  for (auto &root_cell : root_cells)
    if (root_cell)
      root_cell.reset();
  root_cells.clear();
}

// Create root cell
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::createRootCells(const std::vector<RootCellEntryType> &root_cell_entries) {
  root_cells.clear();
  for (const auto &entry : root_cell_entries) {
    auto cell = entry.cell;
    root_cells.push_back(cell);

    // Split root cells for setting child oct neighbors
    if (cell->isLeaf())
      cell->splitRoot(max_level, cell);

    // Connect the neighbors in the root's child_oct
    for (unsigned dir{0}; dir<CellType::number_neighbors; ++dir) {
      const auto &neighbor = entry.neighbor_cells[dir];
      if (neighbor)
        cell->getChildOct()->setNeighborCell(dir, neighbor);
    }
  }
}


//***********************************************************//
//  ACCESSORS                                                //
//***********************************************************//

// Get root cells
template<typename CellType, typename TreeIteratorType>
const std::vector<std::shared_ptr<CellType>>& Tree<CellType, TreeIteratorType>::getRootCells() const {
  return root_cells;
}

// Get min mesh level
template<typename CellType, typename TreeIteratorType>
unsigned Tree<CellType, TreeIteratorType>::getMinLevel() const {
  return min_level;
}

// Get max mesh level
template<typename CellType, typename TreeIteratorType>
unsigned Tree<CellType, TreeIteratorType>::getMaxLevel() const {
  return max_level;
}

// Get ghost manager
template<typename CellType, typename TreeIteratorType>
typename Tree<CellType, TreeIteratorType>::GhostManagerType Tree<CellType, TreeIteratorType>::getGhostManager() const {
  return ghostManager;
}


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Recusively mesh the tree to ensure every cell is at least at
// min level
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::meshAtMinLevel() {
  TreeIteratorType iterator(root_cells, max_level);
  meshAtMinLevel(iterator);
}
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::meshAtMinLevel(TreeIteratorType &iterator) {
  // Meshing at minimum level
	minLevelMeshManager.meshAtMinLevel(root_cells, iterator);
}

// Split all the leaf cells belonging to this proc that need to
// be refined  and are not at max level
template<typename CellType, typename TreeIteratorType>
bool Tree<CellType, TreeIteratorType>::refine(ExtrapolationFunctionType extrapolation_function) {
	// Refining mesh
	return refineManager.refine(root_cells, extrapolation_function);
}

// Creation of ghost cells
template<typename CellType, typename TreeIteratorType>
typename Tree<CellType, TreeIteratorType>::GhostManagerTaskType Tree<CellType, TreeIteratorType>::buildGhostLayer(InterpolationFunctionType interpolation_function, const std::vector<int> &directions) {
  TreeIteratorType iterator(root_cells, max_level);
  return ghostManager.buildGhostLayer(root_cells, iterator, directions, interpolation_function);
}

// Creation of ghost cells
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::exchangeGhostValues(GhostManagerTaskType &task, InterpolationFunctionType interpolation_function) {
  TreeIteratorType iterator(root_cells, max_level);
  ghostManager.exchangeGhostValues(task, iterator, interpolation_function);
}

// Coarse all the cells for which all child are set to be coarsened
template<typename CellType, typename TreeIteratorType>
bool Tree<CellType, TreeIteratorType>::coarsen(InterpolationFunctionType interpolation_function) {
	// Coarsening mesh
	return coarseManager.coarsen(root_cells, interpolation_function);
}

// Redistribute cells among processes to balance computation load
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::loadBalance(InterpolationFunctionType interpolation_function, const double max_pct_unbalance) {
	TreeIteratorType iterator(root_cells, max_level);
  return balanceManager.loadBalance(root_cells, iterator, max_pct_unbalance, interpolation_function);
}

// Count the number of owned leaf cells
template<typename CellType, typename TreeIteratorType>
unsigned Tree<CellType, TreeIteratorType>::countOwnedLeaves() const {
  unsigned nb_owned_leaves = 0;

  for (const auto &root_cell : root_cells)
    if (root_cell->belongToThisProc())
      nb_owned_leaves += root_cell->countOwnedLeaves();
  return nb_owned_leaves;
}

// Count the number of owned leaf cells
template<typename CellType, typename TreeIteratorType>
unsigned Tree<CellType, TreeIteratorType>::countGhostLeaves() const {
  unsigned nb_ghost_leaves = 0;

  for (const auto &root_cell : root_cells)
    if (root_cell->belongToOtherProc())
      nb_ghost_leaves += root_cell->countGhostLeaves();
  return nb_ghost_leaves;
}

// Apply a function to owned leaf cells
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::applyToOwnedLeaves(const std::function<void(const std::shared_ptr<CellType>&, const unsigned)> &f) const {
  TreeIteratorType iterator(getRootCells(), getMaxLevel());

  unsigned index{0};
  if (!iterator.toOwnedBegin())
    return;
  do {
    f(iterator.getCell(), index++);
  } while (iterator.ownedNext());
}

template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::sharePartitions(std::vector<std::vector<unsigned>> &begin_ids, std::vector<std::vector<unsigned>> &end_ids) const {
  TreeIteratorType iterator(root_cells, max_level);
  sharePartitions(begin_ids, end_ids, iterator);
}

// Share the partiion start and end cells
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::sharePartitions(std::vector<std::vector<unsigned>> &begin_ids, std::vector<std::vector<unsigned>> &end_ids, TreeIteratorType &iterator) const {
  ghostManager.sharePartitions(begin_ids, end_ids, iterator);
}

// Share the partiion start and end cells
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::applyToGhostLeavesRanks(const std::function<void(const std::shared_ptr<CellType>&, const unsigned, const unsigned)> &f) const {
  TreeIteratorType iterator(root_cells, max_level);
  applyToGhostLeavesRanks(f, iterator);
}

// Apply a function to owned leaf cells
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::applyToGhostLeavesRanks(const std::function<void(const std::shared_ptr<CellType>&, const unsigned, const unsigned)> &f, TreeIteratorType &iterator) const {
  std::vector<std::vector<unsigned>> partitions_begin_ids, partitions_end_ids;
  sharePartitions(partitions_begin_ids, partitions_end_ids, iterator);

  // Process has no ghost cells
  unsigned index{0};
  if (!iterator.toOwnedBegin()) {
    iterator.toBegin();
    applyToGhostLeaves(f, partitions_begin_ids, partitions_end_ids, index, iterator);
    return;
  }

  // Determine if there is any rank lower that have ghost cells and loop through them
  {
    iterator.toOwnedBegin();
    std::shared_ptr<CellType> owned_begin_cell = iterator.getCell();
    iterator.toBegin();
    if (iterator.getCell() != owned_begin_cell)
      applyToGhostLeaves(f, partitions_begin_ids, partitions_end_ids, index, iterator); // Loop through ghost from begin to owned begin - 1
  }

  // Determine if there is any rank higher that have ghost cells and loop through them
  {
    iterator.toEnd();
    std::shared_ptr<CellType> end_cell = iterator.getCell();
    iterator.toOwnedEnd();
    if (iterator.getCell() != end_cell) {
      iterator.next();
      applyToGhostLeaves(f, partitions_begin_ids, partitions_end_ids, index, iterator); // Loop through ghost from owned end + 1 to end
    }
  }
}

template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::applyToGhostLeaves(const std::function<void(const std::shared_ptr<CellType>&, const unsigned, const unsigned)> &f, std::vector<std::vector<unsigned>> &begin_ids, std::vector<std::vector<unsigned>> &end_ids, unsigned &index, TreeIteratorType &iterator) const {
  unsigned other_rank = 0;
  auto cell_id_manager = iterator.getCellIdManager();
  bool loop{true};
  do {
    // If empty partition skip
    if (!cell_id_manager.cellIdLte(begin_ids[other_rank], end_ids[other_rank])) {
      other_rank++;
      continue;
    }

    std::shared_ptr<CellType> cell = iterator.getCell();

    if (iterator.cellIdLte(end_ids[other_rank])) // Cell is in the proc partition -> callback + next cell (no next rank)
      f(cell, index++, other_rank);
    else if (!iterator.cellIdGt(end_ids[other_rank])) { // Cell may be shared between partitions -> callback + next rank (no next cell)
      f(cell, index++, other_rank);
      other_rank++;
      continue;
    } else { // Cell is in the proc partition -> no callback + next rank
      other_rank++;
      continue;
    }
    // Move iterator and check if still in partition
    loop = iterator.next() && iterator.getCell()->belongToOtherProc();
  } while (loop);
}
