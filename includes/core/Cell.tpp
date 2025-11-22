#include "Cell.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<int Nx, int Ny, int Nz, typename DataType>
Cell<Nx, Ny, Nz, DataType>::Cell()
: data() {
  data = std::make_unique<DataType>();
  indicator = 0;
}

// Constructor (parent_oct=nullptr for root cell)
template<int Nx, int Ny, int Nz, typename DataType>
Cell<Nx, Ny, Nz, DataType>::Cell(std::shared_ptr<OctType> parent_oct, int indicator)
: indicator(indicator) {
  data = std::make_unique<DataType>();
  this->parent_oct = parent_oct;
  child_oct.reset();
}

// Destructor
template<int Nx, int Ny, int Nz, typename DataType>
Cell<Nx, Ny, Nz, DataType>::~Cell() {};

// Clear Cell
template<int Nx, int Ny, int Nz, typename DataType>
void Cell<Nx, Ny, Nz, DataType>::clear() {
  if (child_oct)
    child_oct->clear();
  reset();
}

// Reset Cell
template<int Nx, int Ny, int Nz, typename DataType>
void Cell<Nx, Ny, Nz, DataType>::reset() {
  data.reset();
  parent_oct.reset();
  if (child_oct)
    child_oct.reset();
}


//***********************************************************//
//  ACCESSORS                                                //
//***********************************************************//

// Get child oct
template<int Nx, int Ny, int Nz, typename DataType>
std::shared_ptr<typename Cell<Nx, Ny, Nz, DataType>::OctType> Cell<Nx, Ny, Nz, DataType>::getChildOct() const {
  return child_oct;
};

// Get a specific child cell
template<int Nx, int Ny, int Nz, typename DataType>
std::shared_ptr<Cell<Nx, Ny, Nz, DataType>> Cell<Nx, Ny, Nz, DataType>::getChildCell(const unsigned sibling_number) const {
  if (!isLeaf())
    return getChildOct()->getChildCell(sibling_number);
  return nullptr;
};

// Get child cells
template<int Nx, int Ny, int Nz, typename DataType>
const std::array<std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>, Cell<Nx, Ny, Nz, DataType>::number_children>& Cell<Nx, Ny, Nz, DataType>::getChildCells() const {
  return child_oct->getChildCells();
};

// Get child cells in a specific direction
template<int Nx, int Ny, int Nz, typename DataType>
const std::vector<std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>> Cell<Nx, Ny, Nz, DataType>::getDirChildCells(const unsigned dir) const {
  if (isLeaf())
    throw std::runtime_error("Cannot call on leaf in Cell::getDirChildCells()");

  if ((dir >= 0) && (dir < number_volume_neighbors)) {
    const auto &child_cells = getChildCells();
    const auto &dir_sibling_numbers = ChildAndDirectionTablesType::dir_sibling_numbers[dir];
    std::vector<std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>> dir_child_cells(dir_sibling_numbers.size());
    for (size_t i{0}; i<dir_sibling_numbers.size(); ++i)
      dir_child_cells[i] = child_cells[dir_sibling_numbers[i]];
    return dir_child_cells;
  }

  throw std::runtime_error("Direction 'dir' must be in [0, number_volume_neighbors) in Cell::getDirChildCells()");
}

// Get level of the cell
template<int Nx, int Ny, int Nz, typename DataType>
unsigned Cell<Nx, Ny, Nz, DataType>::getLevel() const {
  return (!isRoot() ? parent_oct->getLevel() : 0);
}

// Get parent oct
template<int Nx, int Ny, int Nz, typename DataType>
std::shared_ptr<typename Cell<Nx, Ny, Nz, DataType>::OctType> Cell<Nx, Ny, Nz, DataType>::getParentOct() const {
  return parent_oct;
};

// Get the sibling number (position of the cell in the parent oct child_cells array)
template<int Nx, int Ny, int Nz, typename DataType>
unsigned Cell<Nx, Ny, Nz, DataType>::getSiblingNumber() const {
  if (parent_oct)
    return parent_oct->getSiblingNumber(this);
  throw std::runtime_error("No parent Oct in Cell::getSiblingNumber()");
}

// Get the computation load of the cell
template<int Nx, int Ny, int Nz, typename DataType>
double Cell<Nx, Ny, Nz, DataType>::getLoad() const {
  return data->getLoad(isLeaf(), std::static_pointer_cast<void>(thisAsSmartPtr()));
}

// Get the cell as a smart pointer for referencing
template<int Nx, int Ny, int Nz, typename DataType>
std::shared_ptr<Cell<Nx, Ny, Nz, DataType>> Cell<Nx, Ny, Nz, DataType>::thisAsSmartPtr() const {
  if (!isLeaf())
    return child_oct->getParentCell();
  if (!isRoot())
    return parent_oct->getChildCell(getSiblingNumber());
  throw std::runtime_error("No child/parent Oct in Cell::thisAsSmartPtr()");
}


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// True if leaf cell (no child oct)
template<int Nx, int Ny, int Nz, typename DataType>
bool Cell<Nx, Ny, Nz, DataType>::isLeaf() const {
  return !child_oct;
}

// True if root cell (no parent oct)
template<int Nx, int Ny, int Nz, typename DataType>
bool Cell<Nx, Ny, Nz, DataType>::isRoot() const {
  return !parent_oct;
}

// Count the number of leaf cells
template<int Nx, int Ny, int Nz, typename DataType>
unsigned Cell<Nx, Ny, Nz, DataType>::countLeaves() const {
  if (isLeaf())
    return 1;

  unsigned nb_leaves = 0;
  for (const auto &child : getChildCells())
    nb_leaves += child->countLeaves();
  return nb_leaves;
}

// Count the number of owned leaf cells
template<int Nx, int Ny, int Nz, typename DataType>
unsigned Cell<Nx, Ny, Nz, DataType>::countOwnedLeaves() const {
  if (!this->belongToThisProc())
    return 0;

  if (isLeaf())
    return 1;

  unsigned nb_owned_leaves = 0;
  for (const auto &child : getChildCells())
    nb_owned_leaves += child->countOwnedLeaves();
  return nb_owned_leaves;
}

// Split a root cell (a pointer to the root is needed for back reference in child oct)
template<int Nx, int Ny, int Nz, typename DataType>
const std::array<std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>, Cell<Nx, Ny, Nz, DataType>::number_children>& Cell<Nx, Ny, Nz, DataType>::splitRoot(const unsigned max_level, std::shared_ptr<Cell> root_cell, ExtrapolationFunctionType extrapolation_function) {
  if (!isRoot())
    throw std::runtime_error("Can be call only on root in Cell::splitRoot()");

  // Split normally
  split(max_level);

  child_oct->setParentCell(root_cell);

  // Call extrapolation function
  extrapolation_function(root_cell);

  return child_oct->getChildCells();
}

// Split a cell and it's direct neighbors if needed for mesh conformity
template<int Nx, int Ny, int Nz, typename DataType>
const std::array<std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>, Cell<Nx, Ny, Nz, DataType>::number_children>& Cell<Nx, Ny, Nz, DataType>::split(const unsigned max_level, ExtrapolationFunctionType extrapolation_function) {
  if (!isLeaf() || getLevel()>=max_level)
    throw std::runtime_error("Cell cannot be splitted in Cell::split()");

  // We check if the neighbors are at level superior or equal to
  // prLvl, if not we refine them first
  checkSplitNeighbors(getLevel() + 1, extrapolation_function);

  // Initialize oct and establish neighbors
  std::shared_ptr oct = std::make_shared<OctType>();
  oct->init(!isRoot() ? thisAsSmartPtr() : nullptr, getLevel() + 1);
  if (!isRoot())
    for (unsigned dir{0}; dir<number_neighbors; ++dir)
      oct->setNeighborCell(dir, getNeighborCell(dir));

  // Make oct as child
  child_oct = oct;

  std::shared_ptr<Cell> cell;
  for (unsigned i{0}; i<number_children; ++i) {
    cell = std::make_shared<Cell>(oct, indicator);
    oct->setChildCell(i, cell);
    cell->setToUnchange();
  }

  // Call extrapolation function
  extrapolation_function(thisAsSmartPtr());

  return child_oct->getChildCells();
}

// Coarsen a cell if neighbors cell allow to preserve consistency else nothing is done
template<int Nx, int Ny, int Nz, typename DataType>
bool Cell<Nx, Ny, Nz, DataType>::coarsen(const unsigned min_level, InterpolationFunctionType interpolation_function) {
  if (isLeaf() || getLevel()<min_level)
    throw std::runtime_error("Cell cannot be coarsened in Cell::coarsenn()");

  if (!verifyCoarsenChildren() || !verifyCoarsenNeighbors())
    return false;

  // Call interpolation function
  interpolation_function(thisAsSmartPtr());

  // Clear Oct and child cells
  child_oct->clear();
  child_oct.reset();
  setToUnchange();

  return true;
}

//┌────────────┬──────────────────┬──────────────────────────────────┐
//│  Priority  │   Available if   │   Indexes (dir)                  │
//├────────────┼──────────────────┼──────────────────────────────────┤
//│  -X        │  Nx>0            │                                  │
//│  +X        │  Nx>0            │                                  │
//│     -Y     │       Ny>0       │  MIN: 0                          │
//│     +Y     │       Ny>0       │  MAX: number_neighbors-1         │
//│        -Z  │            Nz>0  │                                  │
//│        +Z  │            Nz>0  │                                  │
//├────────────┼──────────────────┼──────────────────────────────────┤
//│  -X -Y     │  Nx>0 Ny>0       │                                  │
//│  +X -Y     │  Nx>0 Ny>0       │                                  │
//│  -X +Y     │  Nx>0 Ny>0       │                                  │
//│  +X +Y     │  Nx>0 Ny>0       │                                  │
//│  -X    -Z  │  Nx>0      Nz>0  │                                  │
//│  +X    -Z  │  Nx>0      Nz>0  │  MIN: number_neighbors           │
//│  -X    +Z  │  Nx>0      Nz>0  │  MAX: number_plane_neighbors-1   │
//│  +X    +Z  │  Nx>0      Nz>0  │                                  │
//│     -Y -Z  │       Ny>0 Nz>0  │                                  │
//│     +Y -Z  │       Ny>0 Nz>0  │                                  │
//│     -Y +Z  │       Ny>0 Nz>0  │                                  │
//│     +Y +Z  │       Ny>0 Nz>0  │                                  │
//├────────────┼──────────────────┼──────────────────────────────────┤
//│  -X -Y -Z  │  Nx>0 Ny>0 Nz>0  │                                  │
//│  +X -Y -Z  │  Nx>0 Ny>0 Nz>0  │                                  │
//│  -X +Y -Z  │  Nx>0 Ny>0 Nz>0  │                                  │
//│  +X +Y -Z  │  Nx>0 Ny>0 Nz>0  │  MIN: number_plane_neighbors     │
//│  -X -Y +Z  │  Nx>0 Ny>0 Nz>0  │  MAX: number_volume_neighbors-1  │
//│  +X -Y +Z  │  Nx>0 Ny>0 Nz>0  │                                  │
//│  -X +Y +Z  │  Nx>0 Ny>0 Nz>0  │                                  │
//│  +X +Y +Z  │  Nx>0 Ny>0 Nz>0  │                                  │
//└────────────┴──────────────────┴──────────────────────────────────┘
// Get a pointer to a neighbor cell
template<int Nx, int Ny, int Nz, typename DataType>
std::shared_ptr<Cell<Nx, Ny, Nz, DataType>> Cell<Nx, Ny, Nz, DataType>::getNeighborCell(const unsigned dir, std::array<std::shared_ptr<Cell>, number_plane_neighbors> *cached_neighbors) const {
  if (dir>=number_volume_neighbors)
    throw std::runtime_error("Invalid neighbor direction in Cell::getNeighborCell()");

  if (!isLeaf() && dir < number_neighbors)
    return child_oct->getNeighborCell(dir);
  if (isRoot())
    return nullptr;

  // Determine if the neighbor cell is a sibling or should be deduced from parent oct neighbors
  if (dir < number_neighbors) {
    const auto [neighbor_is_sibling, neighbor_sibling_number] = getDirectNeighborCellInfos(getSiblingNumber(), dir);

    std::shared_ptr<Cell> neighbor_cell;
    if (neighbor_is_sibling)
      neighbor_cell = this->parent_oct->getChildCell(neighbor_sibling_number);
    else {
      neighbor_cell = this->parent_oct->getNeighborCell(dir);
      if (neighbor_cell && !neighbor_cell->isLeaf())
        neighbor_cell = neighbor_cell->getChildCell(neighbor_sibling_number);
    }
    return neighbor_cell;
  } else if (dir < number_plane_neighbors)
    return getPlaneNeighborCell(dir, cached_neighbors);
  else
    return getVolumeNeighborCell(dir, cached_neighbors);
}

// Get a pointer to a neighbor cell and save it  to array for reuse
// If the neighbor was already computed, extract from cached_neighbors array
template<int Nx, int Ny, int Nz, typename DataType>
std::shared_ptr<Cell<Nx, Ny, Nz, DataType>> Cell<Nx, Ny, Nz, DataType>::getNeighborCellAndSave(const unsigned dir, std::array<std::shared_ptr<Cell>, number_plane_neighbors> *cached_neighbors) const {
  // Check if direction already cached
  std::shared_ptr<Cell> this_cell = thisAsSmartPtr();
  if (cached_neighbors && dir < number_plane_neighbors && (*cached_neighbors)[dir] != this_cell)
    return (*cached_neighbors)[dir];

  // Compute neighbors
  std::shared_ptr<Cell> neighbor_cell = getNeighborCell(dir, cached_neighbors);

  // Cache the neighbor cell
  if (cached_neighbors && dir < number_plane_neighbors)
    (*cached_neighbors)[dir] = neighbor_cell;

  return neighbor_cell;
};

// Loop on all neighbor leaf cells and apply a function
template<int Nx, int Ny, int Nz, typename DataType>
void Cell<Nx, Ny, Nz, DataType>::applyToNeighborCells(const std::function<void(const std::shared_ptr<Cell>&, const std::shared_ptr<Cell>&, const unsigned&)> &&f, const bool only_once, const bool skip_null, const std::vector<int> &directions) const {
  // If a neighbor is used more than once in the process it can be retrived from this array
  std::array<std::shared_ptr<Cell>, number_plane_neighbors> cached_neighbors;
  cached_neighbors.fill(thisAsSmartPtr());

  // Cells already seen are saved in this set
  std::unordered_set<Cell*> cell_already_seen;
  cell_already_seen.reserve(ChildAndDirectionTablesType::max_number_neighbor_leaf_cells);

  // Loop through all directions
  for (const unsigned dir : directions) {
    // Get the neighbor cell
    std::shared_ptr<Cell> neighbor = getNeighborCellAndSave(dir, &cached_neighbors);

    // No neighbor cell in this direction
    if (!neighbor) {
      if (!skip_null)
        f(thisAsSmartPtr(), nullptr, dir);
      continue;
    }

    // The neighbor leaf cell has the same level
    if (neighbor->isLeaf()) {
      if (!only_once || !cell_already_seen.count(neighbor.get()))
        f(thisAsSmartPtr(), neighbor, dir);
      else if (only_once && !cell_already_seen.count(neighbor.get()))
        cell_already_seen.insert(neighbor.get());
    }
  }
}

// Loop on all neighbor leaf cells in a specific direction and apply a function
template<int Nx, int Ny, int Nz, typename DataType>
void Cell<Nx, Ny, Nz, DataType>::applyToDirNeighborLeafCells(const unsigned dir, const std::function<void(const std::shared_ptr<Cell>&, const std::shared_ptr<Cell>&, const unsigned&)> &&f) const {
  // Get the neighbor cell
  std::shared_ptr<Cell> neighbor = getNeighborCell(dir);

  // No neighbor cell in this direction
  if (!neighbor) {
    f(thisAsSmartPtr(), nullptr, dir);
    return;
  }

  // The neighbor leaf cell has the same level
  if (neighbor->isLeaf()) {
    f(thisAsSmartPtr(), neighbor, dir);
    return;
  }

  // The neighbor cell is higher level so we loop on its children
  for (const auto &neighbor_child_cell : neighbor->getDirChildCells(dir))
    f(thisAsSmartPtr(), neighbor_child_cell, dir);
}

// Loop on all neighbor leaf cells and apply a function
template<int Nx, int Ny, int Nz, typename DataType>
void Cell<Nx, Ny, Nz, DataType>::applyToNeighborLeafCells(const std::function<void(const std::shared_ptr<Cell>&, const std::shared_ptr<Cell>&, const unsigned&)> &&f, const bool only_once, const bool skip_null, const std::vector<int> &directions) const {
  applyToNeighborCells(
    [&f](const std::shared_ptr<Cell> &c, const std::shared_ptr<Cell> &n, const unsigned &dir) {
      // No neighbor cell in this direction
      if (!n) {
        f(c, nullptr, dir);
        return;
      }

      // The neighbor leaf cell has the same level
      if (n->isLeaf()) {
        f(c, n, dir);
        return;
      }

      // The neighbor cell is higher level so we loop on its children
      for (const auto &neighbor_child_cell : n->getDirChildCells(dir)) {
        f(c, neighbor_child_cell, dir);
      }
    },
    only_once, // Only once per cell
    skip_null, // Skip nullptr values
    directions
  );
}

//Apply extrapolation function to all non-leaf descendent cells recursively
template<int Nx, int Ny, int Nz, typename DataType>
void Cell<Nx, Ny, Nz, DataType>::extrapolateRecursively(ExtrapolationFunctionType extrapolation_function) const {
  if (isLeaf())
    return;

  extrapolation_function(thisAsSmartPtr());

  for (const auto &child : getChildCells())
    child->extrapolateRecursively(extrapolation_function);
}

// Verify if neighbors splitting is needed before cell splitting
template<int Nx, int Ny, int Nz, typename DataType>
bool Cell<Nx, Ny, Nz, DataType>::verifySplitNeighbors(const unsigned max_level) {
  if (isRoot() && max_level>0)
    return (max_level>0);

  for (unsigned dir{0}; dir<number_neighbors; ++dir) {
    auto neighbor_cell = getNeighborCell(dir);
    if (neighbor_cell && neighbor_cell->getLevel()<getLevel() && neighbor_cell->isLeaf())
      return false;
  }
  return true;
}

// Split neighbors first if needed before cell splitting
template<int Nx, int Ny, int Nz, typename DataType>
void Cell<Nx, Ny, Nz, DataType>::checkSplitNeighbors(const unsigned max_level, ExtrapolationFunctionType extrapolation_function) {
  if (isRoot())
    return;

  // Assuming 4 connexity neighbor size consistency
  for (unsigned dir{0}; dir<number_neighbors; ++dir) {
    auto neighbor_cell = getNeighborCell(dir);
    if (neighbor_cell && neighbor_cell->getLevel()<getLevel() && neighbor_cell->isLeaf())
      neighbor_cell->split(max_level, extrapolation_function);
  }
}

// Verify if children coarsening is needed before cell coarsening
template<int Nx, int Ny, int Nz, typename DataType>
bool Cell<Nx, Ny, Nz, DataType>::verifyCoarsenChildren() {
  if (isLeaf())
    return false;

  for (unsigned i{0}; i<number_children; ++i) {
    auto child_cell = getChildCell(i);
    if (!child_cell->isLeaf())
      return false;
  }
  return true;
}

// Verify if neighbors coarsening is needed before cell coarsening
template<int Nx, int Ny, int Nz, typename DataType>
bool Cell<Nx, Ny, Nz, DataType>::verifyCoarsenNeighbors() {
  if (isLeaf())
    return false;

  // Get the neighbor cell
  for (unsigned dir{0}; dir<number_neighbors; ++dir) {
    // Get only the neighbor's children adjacent to the shared face (opposite direction)
    std::shared_ptr<Cell> neighbor = getNeighborCell(dir);
    if (!neighbor || neighbor->isLeaf())
      continue;
    // Verify the neighbor child cells are not split
    auto neighbor_face_child = neighbor->getDirChildCells(dir);
    for (const auto &nbChildCell : neighbor_face_child)
      if (!nbChildCell->isLeaf())
        return false;
  }

  return true;
}

// Get a pointer to a neighbor cell accessible by 2 consecutive othogonal direction (corners in 2D)
template<int Nx, int Ny, int Nz, typename DataType>
std::shared_ptr<Cell<Nx, Ny, Nz, DataType>> Cell<Nx, Ny, Nz, DataType>::getPlaneNeighborCell(const unsigned dir, std::array<std::shared_ptr<Cell>, number_plane_neighbors> *cached_neighbors) const {
  // Split plane neighbor dir to 2 direct neighbor dirs
  const auto [dir1, dir2] = ChildAndDirectionTablesType::planeToDirectDirs(dir);

  std::shared_ptr<Cell> neighbor_cell1 = getNeighborCellAndSave(dir1, cached_neighbors), neighbor_cell12;
  if (neighbor_cell1) {
    neighbor_cell12 = neighbor_cell1->getNeighborCell(dir2);
    if (neighbor_cell1->getLevel() >= getLevel() && neighbor_cell12)
      return neighbor_cell12;
  }

  std::shared_ptr<Cell> neighbor_cell2 = getNeighborCellAndSave(dir2, cached_neighbors), neighbor_cell21;
  if (neighbor_cell2) {
    neighbor_cell21 = neighbor_cell2->getNeighborCell(dir1);
    if (neighbor_cell2->getLevel() >= getLevel() && neighbor_cell21)
      return neighbor_cell21;
  }

  if (!neighbor_cell1 && !neighbor_cell2)
    return nullptr;
  if (!neighbor_cell12 && !neighbor_cell21)
    return nullptr;

  if (neighbor_cell12 != neighbor_cell21) // Keep the cell with the highest level
    return (neighbor_cell12->getLevel()>=neighbor_cell21->getLevel()) ? neighbor_cell12 : neighbor_cell21;

  if (neighbor_cell12->isLeaf() || neighbor_cell12->getLevel()>=getLevel())
    return neighbor_cell12;
  else {
    const unsigned neighbor_sibling_number1 = std::get<1>(getDirectNeighborCellInfos(getSiblingNumber(), dir1));
    const unsigned neighbor_sibling_number12 = std::get<1>(getDirectNeighborCellInfos(neighbor_sibling_number1, dir2));
    return neighbor_cell12->getChildOct()->getChildCell(neighbor_sibling_number12);
  }
}

// Get a pointer to a neighbor cell accessible by 3 consecutive othogonal direction (corners in 3D)
template<int Nx, int Ny, int Nz, typename DataType>
std::shared_ptr<Cell<Nx, Ny, Nz, DataType>> Cell<Nx, Ny, Nz, DataType>::getVolumeNeighborCell(const unsigned dir, std::array<std::shared_ptr<Cell>, number_plane_neighbors> *cached_neighbors) const {
  const auto [dir1, dir2, dir3] = ChildAndDirectionTablesType::volumeToDirectDirs(dir);

  // If one of the neighbor cells have the same level, then finding the target neighbor cell is easier:
  // - get the direct neighbor cell of same level in direction dir in {dir1, dir2, dir3}
  // - get the target cell by getting its plane neighbor cell in directions {dir1, dir2, dir3} - {dir}
  std::shared_ptr<Cell> neighbor_cell1 = getNeighborCellAndSave(dir1, cached_neighbors);
  if (neighbor_cell1 && neighbor_cell1->getLevel() >= getLevel()) {
    std::shared_ptr<Cell> neighbor_cell1_23 = neighbor_cell1->getNeighborCell(directToPlaneDir(dir2, dir3));
    if (neighbor_cell1_23)
      return neighbor_cell1_23;
  }
  std::shared_ptr<Cell> neighbor_cell2 = getNeighborCellAndSave(dir2, cached_neighbors);
  if (neighbor_cell2 && neighbor_cell2->getLevel() >= getLevel()) {
    std::shared_ptr<Cell> neighbor_cell2_13 = neighbor_cell2->getNeighborCell(directToPlaneDir(dir1, dir3));
    if (neighbor_cell2_13)
      return neighbor_cell2_13;
  }
  std::shared_ptr<Cell> neighbor_cell3 = getNeighborCellAndSave(dir3, cached_neighbors);
  if (neighbor_cell3 && neighbor_cell3->getLevel() >= getLevel()) {
    std::shared_ptr<Cell> neighbor_cell3_12 = neighbor_cell3->getNeighborCell(directToPlaneDir(dir1, dir2));
    if (neighbor_cell3_12)
      return neighbor_cell3_12;
  }
  if (!neighbor_cell1 && !neighbor_cell2 && !neighbor_cell3)
    return nullptr;

  std::shared_ptr<Cell> neighbor_cell12 = getNeighborCellAndSave(directToPlaneDir(dir1, dir2), cached_neighbors),
                        neighbor_cell13 = getNeighborCellAndSave(directToPlaneDir(dir1, dir3), cached_neighbors),
                        neighbor_cell23 = getNeighborCellAndSave(directToPlaneDir(dir2, dir3), cached_neighbors);
  if (!neighbor_cell12 && !neighbor_cell13 && !neighbor_cell23)
    return nullptr;
  else if (neighbor_cell12==neighbor_cell13) // If two of the plane neighbors are the same, then they all direct to the target neighbor cell
    return neighbor_cell12;
  else if (neighbor_cell12==neighbor_cell23) // If two of the plane neighbors are the same, then they all direct to the target neighbor cell
    return neighbor_cell12;
  else if (neighbor_cell13==neighbor_cell23) // If two of the plane neighbors are the same, then they all direct to the target neighbor cell
    return neighbor_cell13;

  std::shared_ptr<Cell> neighbor_cell123 = neighbor_cell12 ? neighbor_cell12->getNeighborCell(dir3) : nullptr,
                        neighbor_cell132 = neighbor_cell13 ? neighbor_cell13->getNeighborCell(dir2) : nullptr,
                        neighbor_cell231 = neighbor_cell23 ? neighbor_cell23->getNeighborCell(dir1) : nullptr;

  if (!neighbor_cell123 && !neighbor_cell132 && !neighbor_cell231)
    return nullptr;

  // Keep the cell with the highest level
  if (neighbor_cell123 && neighbor_cell12->getLevel() >= getLevel())
    return neighbor_cell123;
  if (neighbor_cell132 && neighbor_cell13->getLevel() >= getLevel())
    return neighbor_cell132;
  if (neighbor_cell231 && neighbor_cell23->getLevel() >= getLevel())
    return neighbor_cell231;

  // Keep the cell with the highest level
  std::shared_ptr<Cell> neighbor_cell_ijk;
  unsigned level_ij;
  if (!neighbor_cell123) {
    neighbor_cell_ijk = neighbor_cell123;
    level_ij = neighbor_cell12->getLevel();
  }
  if (!neighbor_cell_ijk || (neighbor_cell132 && level_ij < neighbor_cell13->getLevel())) {
    neighbor_cell_ijk = neighbor_cell132;
    level_ij = neighbor_cell13->getLevel();
  }
  if (!neighbor_cell_ijk || (neighbor_cell231 && level_ij < neighbor_cell23->getLevel())) {
    neighbor_cell_ijk = neighbor_cell231;
    level_ij = neighbor_cell23->getLevel();
  }

  if (neighbor_cell_ijk->isLeaf() || neighbor_cell_ijk->getLevel()>=getLevel())
    return neighbor_cell_ijk;
  else {
    while (!neighbor_cell_ijk->isLeaf() && neighbor_cell_ijk->getLevel()<getLevel())
      neighbor_cell_ijk = neighbor_cell_ijk->getDirChildCells(dir)[0];
    return neighbor_cell_ijk;
  }
}

// Flags propagation from parent to children
template<int Nx, int Ny, int Nz, typename DataType>
void Cell<Nx, Ny, Nz, DataType>::setIndicatorFromParent(const Cell &parent_cell) {
  // If parent belong to this proc then child also
  if (parent_cell.belongToThisProc()) {
    setToThisProc();
    return;
  }
  // If parent is boundary then child also
  if (parent_cell.isBoundaryCell()) {
    setToBoundary();
    return;
  }
}
