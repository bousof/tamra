#include "AbstractTreeIterator.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType>
AbstractTreeIterator<CellType>::AbstractTreeIterator(const std::vector<std::shared_ptr<CellType>> &root_cells, const int max_level)
: root_cells(root_cells),
  max_level(max_level),
  cell_id_manager(root_cells.size(), max_level) {
  level_partition_sizes.assign(max_level+1, 1);
  for (int i{max_level-1}; i>=0; --i)
    level_partition_sizes[i] = level_partition_sizes[i+1] * CellType::number_children;
  index_path.reserve(max_level+1);
  index_path.resize(1);
  index_path[0] = 0;
  order_path.reserve(max_level+1);
  order_path.resize(1);
  order_path[0] = 0;
  cell_id_manager.resetCellID(current_cell_id);
}

//***********************************************************//
//  ACCESSORS                                                //
//***********************************************************//

// Get current cell
template<typename CellType>
std::shared_ptr<CellType> AbstractTreeIterator<CellType>::getCell() const {
  return current_cell;
}

// Get current cell partition
template<typename CellType>
const std::pair<int, int>& AbstractTreeIterator<CellType>::getPartition() const {
  return current_cell_partition;
}

// Get current index path
template<typename CellType>
const std::vector<unsigned>& AbstractTreeIterator<CellType>::getIndexPath() const {
  return index_path;
};

// Get current order path
template<typename CellType>
const std::vector<unsigned>& AbstractTreeIterator<CellType>::getOrderPath() const {
  return order_path;
};

// Get current cell ID
template<typename CellType>
std::vector<unsigned> AbstractTreeIterator<CellType>::getCellId() const {
  return current_cell_id;
}

// Get cell ID manager
template<typename CellType>
typename AbstractTreeIterator<CellType>::CellIdManagerType AbstractTreeIterator<CellType>::getCellIdManager() const {
  return cell_id_manager;
};

// Construct cell id
template<typename CellType>
std::vector<unsigned> AbstractTreeIterator<CellType>::getCellId(const std::shared_ptr<CellType> &cell) const {
  return indexPathToId(getCellIndexPath(cell));
}

// Construct cell index path
template<typename CellType>
std::vector<unsigned> AbstractTreeIterator<CellType>::getCellIndexPath(const std::shared_ptr<CellType> &cell) const {
  std::vector<unsigned> cell_index_path(cell->getLevel()+1);
  // Browse parents until root  to extract index path
  std::shared_ptr<CellType> parent = cell;
  for (unsigned l{parent->getLevel()}; l>0; --l) {
    cell_index_path[l] = parent->getSiblingNumber();
    parent = parent->getParentOct()->getParentCell();
  }
  // Find the associated root (parent should be a root now)
  for (unsigned i{0}; i<root_cells.size(); ++i)
    if (root_cells[i] == parent)
      cell_index_path[0] = i;

  return cell_index_path;
}

//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Go to the next leaf cell
template<typename CellType>
bool AbstractTreeIterator<CellType>::next(const int sweep_level) {
  unsigned current_order = order_path.back();
  if (current_order < (CellType::number_children-1)) { // Going to the next sibling
    this->toParent();
    this->toChild(current_order+1);
    toLeaf(sweep_level, false);
    return true;
  }
  if (order_path.size() > 2) { // Mother cell is not a root cell
    this->toParent();
    return next(sweep_level);
  }
  // Mother cell is root cell
  this->toRoot((index_path[0]+1) % root_cells.size());
  toLeaf(sweep_level, false);
  return index_path[0] != 0;
}

// Go to the next leaf cell belonging to this proc
template<typename CellType>
bool AbstractTreeIterator<CellType>::ownedNext(const int sweep_level) {
  bool notLoop = next(sweep_level);
  return notLoop && current_cell->belongToThisProc();
}

// Go to the previous leaf cell
template<typename CellType>
bool AbstractTreeIterator<CellType>::prev(const int sweep_level) {
  unsigned current_order = order_path.back();
  if (current_order > 0) { // Going to the next sibling
    this->toParent();
    this->toChild(current_order-1);
    toLeaf(sweep_level, true);
    return true;
  }
  if (order_path.size() > 2) { // Mother cell is not a root cell
    this->toParent();
    return prev(sweep_level);
  }
  // Mother cell is root cell
  this->toRoot((index_path[0]+root_cells.size()-1) % root_cells.size());
  toLeaf(sweep_level, true);
  return index_path[0] != (root_cells.size()-1);
}

// Go to the previous leaf cell belonging to this proc
template<typename CellType>
bool AbstractTreeIterator<CellType>::ownedPrev(const int sweep_level) {
  bool notLoop = prev(sweep_level);
  return notLoop && current_cell->belongToThisProc();
}

// Go to the first leaf cell of first root
template<typename CellType>
void AbstractTreeIterator<CellType>::toBegin(const int sweep_level) {
  // Go to first root
  this->toRoot(0);
  toLeaf(sweep_level, false);
}

// Go to the first leaf cell of first root belonging to this process
template<typename CellType>
bool AbstractTreeIterator<CellType>::toOwnedBegin(const int sweep_level) {
  // Find first owned root
  unsigned i;
  bool found = false;
  for (i=0; i<root_cells.size(); ++i)
    if (root_cells[i]->belongToThisProc()) {
      found = true;
      break;
    }

  if (!found)
    return false;

  // Go to first owned root
  this->toRoot(i);
  toOwnedLeaf(sweep_level, false);
  return true;
}

// Go to the last leaf cell of last root
template<typename CellType>
void AbstractTreeIterator<CellType>::toEnd(const int sweep_level) {
  // Go last root
  this->toRoot(root_cells.size()-1);
  toLeaf(sweep_level, true);
}

// Go to the last leaf cell of last root belonging to this process
template<typename CellType>
bool AbstractTreeIterator<CellType>::toOwnedEnd(const int sweep_level) {
  // Find last owned root
  unsigned i;
  bool found = false;
  for (i=root_cells.size()-1; i>=0; --i)
    if (root_cells[i]->belongToThisProc()) {
      found = true;
      break;
    }

  if (!found)
    return false;

  // Go to last owned root
  this->toRoot(i);
  toOwnedLeaf(sweep_level, true);
  return true;
}

// Moves the iterator to a leaf cell of the current cell
template<typename CellType>
void AbstractTreeIterator<CellType>::toLeaf(const int sweep_level, const bool reverse) {
  while (!current_cell->isLeaf() && order_path.size()<=sweep_level)
    // Update current to child
    if (reverse)
      this->toChild(CellType::number_children - 1);
    else
      this->toChild(0);
}

// Moves the iterator to a leaf cell of the current cell that belong to the process
template<typename CellType>
void AbstractTreeIterator<CellType>::toOwnedLeaf(const int sweep_level, const bool reverse) {
  while (!current_cell->isLeaf() && order_path.size()<=sweep_level) {
    unsigned order;
    if (reverse)
      for (order=CellType::number_children-1; order>=0; --order) {
        if (getChildCellFromOrder(current_cell, order, true)->belongToThisProc())
          break;
      } // Keep braces to avoid dangling (if, else, what else?)
    else
      for (order=0; order<CellType::number_children; ++order)
        if (getChildCellFromOrder(current_cell, order, true)->belongToThisProc())
          break;

    // Update current to child
    this->toChild(order);
  }
}

// Move iterator to a specific cell ID (can also create it with a flag)
template<typename CellType>
void AbstractTreeIterator<CellType>::toCellId(const std::vector<unsigned> &cell_id, const bool create, ExtrapolationFunctionType extrapolation_function) {
  std::vector<unsigned> order_path = idToOrderPath(cell_id);

  // Go to right cell
  this->toRoot(order_path[0]);
  for (unsigned i{1}; i<order_path.size(); ++i) {
    if (create && current_cell->isLeaf())
      current_cell->split(max_level, extrapolation_function);
    if (!current_cell->isLeaf())
      this->toChild(order_path[i]);
    else
      throw std::runtime_error("Cannot reach cell in AbstractTreeIterator::toCellId()");
  }
}

// Generate an ID from the genealogy of a cell.
template<typename CellType>
std::vector<unsigned> AbstractTreeIterator<CellType>::indexPathToId(const std::vector<unsigned> &index_path) const {
  const std::vector<unsigned> order_path = this->indexToOrderPath(index_path);
  return cell_id_manager.orderPathToId(order_path);
}

// Generate an ID from the genealogy of a cell.
template<typename CellType>
std::vector<unsigned> AbstractTreeIterator<CellType>::orderPathToId(const std::vector<unsigned> &order_path) const {
  return cell_id_manager.orderPathToId(order_path);
}

// Generate an ID from the genealogy of a cell.
template<typename CellType>
std::vector<unsigned> AbstractTreeIterator<CellType>::idToOrderPath(const std::vector<unsigned> &cell_id) const {
  return cell_id_manager.idToOrderPath(cell_id);
}

// Put iterator to child cell
template<typename CellType>
void AbstractTreeIterator<CellType>::toChild(const unsigned order) {
  order_path.push_back(order);
  index_path.push_back(this->orderToSiblingNumber(order));
  current_cell = getChildCellFromOrder(current_cell, order);
  cell_id_manager.toChild(current_cell_id, order);
  //compareID("toChild: ");
  // Update current cell partition
  int child_partition_size = level_partition_sizes[order_path.size()-1];
  current_cell_partition = std::make_pair(
    current_cell_partition.first + order * child_partition_size,
    current_cell_partition.first + (order+1) * child_partition_size - 1
  );
}

// Put iterator to parent cell
template<typename CellType>
void AbstractTreeIterator<CellType>::toParent() {
  order_path.pop_back();
  index_path.pop_back();
  current_cell = current_cell->getParentOct()->getParentCell();
  cell_id_manager.toParent(current_cell_id);
  //compareID("toParent: ");
  // Update current cell partition
  int parent_partition_size = level_partition_sizes[order_path.size()-1];
  current_cell_partition = std::make_pair(
    current_cell_partition.first - current_cell_partition.first % parent_partition_size,
    current_cell_partition.first - current_cell_partition.first % parent_partition_size + parent_partition_size - 1
  );
}

// Put iterator to root cell
template<typename CellType>
void AbstractTreeIterator<CellType>::toRoot(const unsigned root_number) {
  index_path.clear();
  order_path.clear();
  index_path.push_back(root_number);
  order_path.push_back(root_number);
  current_cell = root_cells[root_number];
  cell_id_manager.toRoot(current_cell_id, root_number);
  //compareID("toRoot: ");
  // Update current cell partition
  current_cell_partition = std::make_pair(
    root_number * level_partition_sizes[0],
    (root_number+1) * level_partition_sizes[0] - 1
  );
}

// Return the child cell from order and mother cell orientation (obtained by following the curve)
template<typename CellType>
std::shared_ptr<CellType> AbstractTreeIterator<CellType>::getChildCellFromOrder(std::shared_ptr<CellType> cell, unsigned order, const bool compute_orientation) {
  return cell->getChildCell(this->orderToSiblingNumber(order, compute_orientation));
}
