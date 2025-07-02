#include "Tree.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType, typename TreeIteratorType>
Tree<CellType, TreeIteratorType>::Tree(const int min_level, const int max_level, const int rank, const int size)
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
  for (auto &root_cell: root_cells)
    if (root_cell)
      root_cell.reset();
  root_cells.clear();
}

// Create root cell
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::createRootCells(const std::vector< RootCellEntryType > &root_cell_entries) {
  root_cells.clear();
  for (const auto& entry : root_cell_entries) {
    auto cell = entry.cell;
    root_cells.push_back(cell);

    // Split root cells for setting child oct neighbors
    if (cell->isLeaf())
      cell->splitRoot(max_level, cell);

    // Connect the neighbors in the root's child_oct
    for (int dir = 0; dir < CellType::number_neighbors; ++dir) {
      const auto& neighbor = entry.neighbor_cells[dir];
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
const std::vector< std::shared_ptr<CellType> >& Tree<CellType, TreeIteratorType>::getRootCells() const {
  return root_cells;
}

// Get min mesh level
template<typename CellType, typename TreeIteratorType>
int Tree<CellType, TreeIteratorType>::getMinLevel() const {
  return min_level;
}

// Get max mesh level
template<typename CellType, typename TreeIteratorType>
int Tree<CellType, TreeIteratorType>::getMaxLevel() const {
  return max_level;
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
void Tree<CellType, TreeIteratorType>::refine() {
	// Refining mesh
	refineManager.refine(root_cells);
}

// Creation of ghost cells
template<typename CellType, typename TreeIteratorType>
typename Tree<CellType, TreeIteratorType>::GhostManagerTaskType Tree<CellType, TreeIteratorType>::buildGhostLayer(TreeIteratorType &iterator) {
  return ghostManager.buildGhostLayer(root_cells, iterator);
}

// Coarse all the cells for which all child are set to be coarsened
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::coarsen() {
	// Coarsening mesh
	coarseManager.coarsen(root_cells);
}

// Redistribute cells among processes to balance computation load
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::loadBalance() {
	TreeIteratorType iterator(root_cells, max_level);
	return loadBalance(iterator);
}
// Redistribute cells among processes to balance computation load
template<typename CellType, typename TreeIteratorType>
void Tree<CellType, TreeIteratorType>::loadBalance(TreeIteratorType &iterator) {
	// Load balancing mesh
	return balanceManager.loadBalance(root_cells, iterator, 0.1);
}
