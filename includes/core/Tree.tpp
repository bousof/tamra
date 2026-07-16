#include "Tree.h"
//#include "../utils/display_vector.h"

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

// Get ghost manager
template<typename CellType, typename TreeIteratorType>
bool Tree<CellType, TreeIteratorType>::isEmpty() const {
  if (root_cells.empty())
    return true;
  
  // Go to first owned cell and return true if it is not in this proc partition
  TreeIteratorType iterator(getRootCells(), getMaxLevel());
  return !iterator.toOwnedBegin();
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

// Count the number of cells
template<typename CellType, typename TreeIteratorType>
unsigned Tree<CellType, TreeIteratorType>::countCells(const std::shared_ptr<CellType> &cell) const {
  if (cell) {
    unsigned nb_cells = 1;
    if (!cell->isLeaf())
      for (const auto &child_cell : cell->getChildCells())
        nb_cells += countCells(child_cell);
    return nb_cells;
  }

  unsigned nb_cells{0};

  for (const auto &root_cell : root_cells)
    nb_cells += countCells(root_cell);

  return nb_cells;
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

// Apply a function to all cells
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::applyToAllCells(const std::function<void(const std::shared_ptr<CellType>&, const unsigned)> &f) const {
  unsigned index{0};
  for (const auto &root_cell : root_cells) {
    f(root_cell, index++);
    applyToAllCellsRecurs(root_cell, f, index);
  }
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

// Recursively apply a function to all child cells
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::applyToAllCellsRecurs(const std::shared_ptr<CellType> &cell, const std::function<void(const std::shared_ptr<CellType>&, const unsigned)> &f, unsigned &index) const {
  if (!cell->isLeaf())
    for (auto &child : cell->getChildCells()) {
      f(child, index++);
      applyToAllCellsRecurs(child, f, index);
    }
}

// Merge the tree with another one with partition following the same SFC
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::mergeWith(const Tree &other_tree, ExtrapolationFunctionType extrapolation_function, MergeFunctionType merge_function) {
  // If the other tree is empty, there is nothing to merge
  if (other_tree.isEmpty())
    return;

  // The tree should have the same max level because we use cell IDs to check if partitions are following each other
  if (getMaxLevel() != other_tree.getMaxLevel())
    throw std::runtime_error("Tree::mergeWith: Cannot merge trees with different max levels: " + std::to_string(getMaxLevel()) + " vs " + std::to_string(other_tree.getMaxLevel()));

  if (root_cells.size() != other_tree.root_cells.size())
    throw std::runtime_error("Tree::mergeWith: Cannot merge trees with different numbers of root cells");

  for (size_t i{0}; i<root_cells.size(); ++i)
    for (unsigned dir{0}; dir<CellType::number_neighbors; ++dir) {
      const auto this_neighbor = root_cells[i]->getNeighborCell(dir);
      const auto other_neighbor = other_tree.root_cells[i]->getNeighborCell(dir);
      if (!this_neighbor != !other_neighbor)
        throw std::runtime_error("Tree::mergeWith: Cannot merge trees with different root-cell topologies");
      if (this_neighbor) {
        const auto this_neighbor_index = std::find(root_cells.begin(), root_cells.end(), this_neighbor);
        const auto other_neighbor_index = std::find(other_tree.root_cells.begin(), other_tree.root_cells.end(), other_neighbor);
        if (this_neighbor_index - root_cells.begin() != other_neighbor_index - other_tree.root_cells.begin())
          throw std::runtime_error("Tree::mergeWith: Cannot merge trees with different root-cell topologies");
      }
    }

  // If the tree calling the merge is not empty, we need to verify that the two partitions are compatible
  if (!isEmpty()) {
    TreeIteratorType this_end_iterator(getRootCells(), getMaxLevel());
    this_end_iterator.toOwnedEnd();

    TreeIteratorType other_start_iterator(other_tree.getRootCells(), getMaxLevel());
    other_start_iterator.toOwnedBegin();

    if (!this_end_iterator.cellIdLt(other_start_iterator.getCellId()))
      throw std::runtime_error("Tree::mergeWith: Cannot merge trees with non-contiguous partitions");

    // We move hte iterators one cell step and they should be in reverse order in the SFC
    this_end_iterator.next();
    other_start_iterator.prev();
    if (!this_end_iterator.cellIdGt(other_start_iterator.getCellId()))
      throw std::runtime_error("Tree::mergeWith: Cannot merge trees with non-contiguous partitions");
  }

  // Merge the root cells of the two trees
  for (unsigned i{0}; i<other_tree.getRootCells().size(); ++i)
    mergeCellRecurs(root_cells[i], other_tree.getRootCells()[i], extrapolation_function, merge_function);
}

// Merge the tree with another one with partition following the same SFC
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::mergeCellRecurs(const std::shared_ptr<CellType> &cell, const std::shared_ptr<CellType> &other_cell, ExtrapolationFunctionType extrapolation_function, MergeFunctionType merge_function) {
  // Merge the data of the two corresponding cells
  merge_function(cell, other_cell);

  // If one of the cells is a leaf and the other is not, we need to split the leaf cell to ensure that both cells have the same structure before merging their children
  if (cell->isLeaf() && !other_cell->isLeaf())
    cell->split(max_level, extrapolation_function);
  if (!cell->isLeaf() && other_cell->isLeaf())
    other_cell->split(max_level, extrapolation_function);

  // Recursive call at deeper tree levels
  if (!other_cell->isLeaf())
    for (unsigned i{0}; i<CellType::number_children; ++i)
      mergeCellRecurs(cell->getChildCell(i), other_cell->getChildCell(i), extrapolation_function, merge_function);

  // Add the cell to the destination partition after merging its children so newly split cells keep their previous ownership while their data is merged.
  if (other_cell->belongToThisProc())
    cell->setToThisProc();
}
