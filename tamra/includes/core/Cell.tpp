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
  if (child_oct) {
    child_oct.reset();
  }
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
std::shared_ptr<Cell<Nx, Ny, Nz, DataType>> Cell<Nx, Ny, Nz, DataType>::getChildCell(const unsigned neighbor_sibling_number) const {
  if (!isLeaf())
    return getChildOct()->getChildCell(neighbor_sibling_number);
  return nullptr;
};

// Get child cells
template<int Nx, int Ny, int Nz, typename DataType>
const std::array<std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>, Cell<Nx, Ny, Nz, DataType>::number_children>& Cell<Nx, Ny, Nz, DataType>::getChildCells() const {
  return child_oct->getChildCells();
};

// Get child cells in a specific direction
template<int Nx, int Ny, int Nz, typename DataType>
const std::vector<std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>> Cell<Nx, Ny, Nz, DataType>::getDirChildCells(const int dir) const {
  if (isLeaf())
    throw std::runtime_error("Cannot call on leaf in Cell::getDirChildCells()");

  if (dir < number_neighbors)
    return getDirFaceChildCells(dir);
  else if (dir < number_plane_neighbors)
    return getDirEdgeChildCells(dir);
  else if (dir < number_volume_neighbors)
    return getDirCornerChildCells(dir);

  throw std::runtime_error("Direction 'dir' must be in [0, number_volume_neighbors) in Cell::getDirChildCells()");
}

template<int Nx, int Ny, int Nz, typename DataType>
const std::vector<std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>> Cell<Nx, Ny, Nz, DataType>::getDirFaceChildCells(const int dir) const {
  // Verify the child cells at the interface are not split
  int ni, Nj, Nk;
  switch (dir) {
    case 0: ni = N1-1; Nj = N2; Nk = N3; break;
    case 1: ni = 0;    Nj = N2; Nk = N3; break;
    case 2: ni = N2-1; Nj = N1; Nk = N3; break;
    case 3: ni = 0;    Nj = N1; Nk = N3; break;
    case 4: ni = N3-1; Nj = N1; Nk = N2; break;
    case 5: ni = 0;    Nj = N1; Nk = N2; break;
  }

  std::vector<std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>> face_child_cells;
  face_child_cells.reserve(Nj * Nk);
  for (int j{0}; j<Nj; ++j)
    for (int k{0}; k<Nk; ++k) {
      if (dir < 2)
        face_child_cells.push_back(getChildCell(coordsToSiblingNumber(ni, j, k)));
      else if (dir < 4)
        face_child_cells.push_back(getChildCell(coordsToSiblingNumber(j, ni, k)));
      else
        face_child_cells.push_back(getChildCell(coordsToSiblingNumber(j, k, ni)));
    }

  return face_child_cells;
}

template<int Nx, int Ny, int Nz, typename DataType>
const std::vector<std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>> Cell<Nx, Ny, Nz, DataType>::getDirEdgeChildCells(const int dir) const {
  // Verify the child cells at the interface are not split
  int ni, nj, Nk;
  switch (dir-number_neighbors) {
    case  0: ni = N1-1; nj = N2-1; Nk = N3; break;
    case  1: ni = 0;    nj = N2-1; Nk = N3; break;
    case  2: ni = N1-1; nj = 0;    Nk = N3; break;
    case  3: ni = 0;    nj = 0;    Nk = N3; break;
    case  4: ni = N1-1; nj = N3-1; Nk = N2; break;
    case  5: ni = 0;    nj = N3-1; Nk = N2; break;
    case  6: ni = N1-1; nj = 0;    Nk = N2; break;
    case  7: ni = 0;    nj = 0;    Nk = N2; break;
    case  8: ni = N2-1; nj = N3-1; Nk = N1; break;
    case  9: ni = 0;    nj = N3-1; Nk = N1; break;
    case 10: ni = N2-1; nj = 0;    Nk = N1; break;
    case 11: ni = 0;    nj = 0;    Nk = N1; break;
  }

  std::vector<std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>> edge_child_cells;
  edge_child_cells.reserve(Nk);
  for (int k{0}; k<Nk; ++k) {
    if (dir-number_neighbors < 4)
      edge_child_cells.push_back(getChildCell(coordsToSiblingNumber(ni, nj,  k)));
    else if (dir-number_neighbors < 8)
      edge_child_cells.push_back(getChildCell(coordsToSiblingNumber(ni,  k, nj)));
    else
      edge_child_cells.push_back(getChildCell(coordsToSiblingNumber( k, ni, nj)));
  }

  return edge_child_cells;
}

template<int Nx, int Ny, int Nz, typename DataType>
const std::vector<std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>> Cell<Nx, Ny, Nz, DataType>::getDirCornerChildCells(const int dir) const {
  // TODO: Implement this
  return {};
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
    return parent_oct->getChildCell(parent_oct->getSiblingNumber(this));
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
const std::array<std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>, Cell<Nx, Ny, Nz, DataType>::number_children>& Cell<Nx, Ny, Nz, DataType>::splitRoot(const int max_level, std::shared_ptr<Cell> root_cell, ExtrapolationFunctionType extrapolation_function) {
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
const std::array<std::shared_ptr<Cell<Nx, Ny, Nz, DataType>>, Cell<Nx, Ny, Nz, DataType>::number_children>& Cell<Nx, Ny, Nz, DataType>::split(const int max_level, ExtrapolationFunctionType extrapolation_function) {
  if (!isLeaf() || getLevel()>=max_level)
    throw std::runtime_error("Cell cannot be splitted in Cell::split()");

  // We check if the neighbors are at level superior or equal to
  // prLvl, if not we refine them first
  checkSplitNeighbors(getLevel() + 1, extrapolation_function);

  // Initialize oct and establish neighbors
  std::shared_ptr oct = std::make_shared<OctType>();
  oct->init(!isRoot() ? thisAsSmartPtr() : nullptr, getLevel() + 1);
  if (!isRoot())
    for (int dir{0}; dir<number_neighbors; ++dir)
      oct->setNeighborCell(dir, getNeighborCell(dir));

  // Make oct as child
  child_oct = oct;

  std::shared_ptr<Cell> cell;
  for (auto i{0}; i<number_children; ++i) {
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
bool Cell<Nx, Ny, Nz, DataType>::coarsen(const int min_level, InterpolationFunctionType interpolation_function) {
  if (isLeaf() || getLevel()<min_level) {
    throw std::runtime_error("Cell cannot be coarsened in Cell::coarsenn()");
  }

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

//_________________________________________________________________
//  Priority  |   Available if   |   Indexes (dir)
//____________|__________________|_________________________________
//  -X        |  Nx>0            |
//  +X        |  Nx>0            |
//     -Y     |       Ny>0       |  MIN: 0
//     +Y     |       Ny>0       |  MAX: number_neighbors-1
//        -Z  |            Nz>0  |
//        +Z  |            Nz>0  |_________________________________
//  -X -Y     |  Nx>0 Ny>0       |
//  +X -Y     |  Nx>0 Ny>0       |
//  -X +Y     |  Nx>0 Ny>0       |
//  +X +Y     |  Nx>0 Ny>0       |
//  -X    -Z  |  Nx>0      Nz>0  |
//  +X    -Z  |  Nx>0      Nz>0  |  MIN: number_neighbors
//  -X    +Z  |  Nx>0      Nz>0  |  MAX: number_plane_neighbors-1
//  +X    +Z  |  Nx>0      Nz>0  |
//     -Y -Z  |       Ny>0 Nz>0  |
//     +Y -Z  |       Ny>0 Nz>0  |
//     -Y +Z  |       Ny>0 Nz>0  |
//     +Y +Z  |       Ny>0 Nz>0  |_________________________________
//  -X -Y -Z  |  Nx>0 Ny>0 Nz>0  |
//  +X -Y -Z  |  Nx>0 Ny>0 Nz>0  |
//  -X +Y -Z  |  Nx>0 Ny>0 Nz>0  |
//  +X +Y -Z  |  Nx>0 Ny>0 Nz>0  |  MIN: number_plane_neighbors
//  -X -Y +Z  |  Nx>0 Ny>0 Nz>0  |  MAX: number_volume_neighbors-1
//  +X -Y +Z  |  Nx>0 Ny>0 Nz>0  |
//  -X +Y +Z  |  Nx>0 Ny>0 Nz>0  |
//  +X +Y +Z  |  Nx>0 Ny>0 Nz>0  |
//____________|__________________|_________________________________
// Get a pointer to a neighbor cell
template<int Nx, int Ny, int Nz, typename DataType>
std::shared_ptr<Cell<Nx, Ny, Nz, DataType>> Cell<Nx, Ny, Nz, DataType>::getNeighborCell(const int dir) const {
  if (dir<0 || dir>=number_volume_neighbors)
    throw std::runtime_error("Invalid neighbor direction in Cell::getNeighborCell()");

  if (!isLeaf())
    return child_oct->getNeighborCell(dir);
  if (isRoot())
    return nullptr;

  unsigned sibling_number = parent_oct->getSiblingNumber(this);

  // Determine if the neighbor cell is a sibling or should be deduced from parent oct neighbors
  if (dir < number_neighbors) {
    bool neighbor_is_sibling;
    unsigned neighbor_sibling_number;
    std::tie(neighbor_is_sibling, neighbor_sibling_number) = getDirectNeighborCellInfos(sibling_number, dir);

    std::shared_ptr<Cell> neighbor_cell;
    if (neighbor_is_sibling)
      neighbor_cell = this->parent_oct->getChildCell(neighbor_sibling_number);
    else {
      neighbor_cell = this->parent_oct->getNeighborCell(dir);
      if (neighbor_cell && !neighbor_cell->isLeaf())
        neighbor_cell = neighbor_cell->getChildCell(neighbor_sibling_number);
    }
    return neighbor_cell;
  } else if (dir < number_plane_neighbors) {
    const int dir1 = (dir-number_neighbors)   % 2 ? 1 : 0,
              dir2 = (dir-number_neighbors)/2 % 2 ? 3 : 2;
    return getPlaneNeighborCell(sibling_number, dir1, dir2);
  } else {
    const int dir1 = (dir-number_plane_neighbors)   % 2 ? 1 : 0,
              dir2 = (dir-number_plane_neighbors)/2 % 2 ? 3 : 2,
              dir3 = (dir-number_plane_neighbors)/4 % 2 ? 5 : 4;
    return getVolumeNeighborCell(sibling_number, dir1, dir2, dir3);
  }
}

// Loop on all neighbor cells in a specific direction and apply a function
template<int Nx, int Ny, int Nz, typename DataType>
void Cell<Nx, Ny, Nz, DataType>::applyToDirNeighborCells(const unsigned dir, const std::function<void(const std::shared_ptr<Cell>&, const std::shared_ptr<Cell>&, const unsigned&)> &&f) const {
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
  for (const auto &nbChildCell : neighbor->getDirChildCells(dir))
    f(thisAsSmartPtr(), nbChildCell, dir);
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
bool Cell<Nx, Ny, Nz, DataType>::verifySplitNeighbors(const int max_level) {
  if (isRoot() && max_level>0)
    return (max_level>0);

  for (int dir{0}; dir<number_neighbors; ++dir) {
    auto neighbor_cell = getNeighborCell(dir);
    if (neighbor_cell && neighbor_cell->getLevel()<getLevel() && neighbor_cell->isLeaf())
      return false;
  }
  return true;
}

// Split neighbors first if needed before cell splitting
template<int Nx, int Ny, int Nz, typename DataType>
void Cell<Nx, Ny, Nz, DataType>::checkSplitNeighbors(const int max_level, ExtrapolationFunctionType extrapolation_function) {
  if (isRoot())
    return;

  // Assuming 4 connexity neighbor size consistency
  for (int dir{0}; dir<number_neighbors; ++dir) {
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

  for (int i{0}; i<number_children; ++i) {
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
  for (int dir{0}; dir<number_neighbors; ++dir) {
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

// Transform sibling number to (i,j,k) coordinates
template<int Nx, int Ny, int Nz, typename DataType>
std::tuple<unsigned, unsigned, unsigned> Cell<Nx, Ny, Nz, DataType>::siblingNumberToCoords(const int sibling_number) {
  unsigned sibling_coord_1 = sibling_number % N1,
           sibling_coord_2 = (sibling_number / N1) % N2,
           sibling_coord_3 = (sibling_number / (N12)) % N3;

  return std::make_tuple(sibling_coord_1, sibling_coord_2, sibling_coord_3);
}

// Transform (i,j,k) coordinates to sibling number
template<int Nx, int Ny, int Nz, typename DataType>
inline int Cell<Nx, Ny, Nz, DataType>::coordsToSiblingNumber(const unsigned sibling_coord_1, const unsigned sibling_coord_2, const unsigned sibling_coord_3) const {
  return sibling_coord_1 + sibling_coord_2 * N1 + sibling_coord_3 * N12;
}

// For a given sibling number, determine if the neighbor cell in a given direction:
// - shares the same parent cell (true) or belongs to another cell (false)
// - it's sibling number
template<int Nx, int Ny, int Nz, typename DataType>
std::pair<bool, unsigned> Cell<Nx, Ny, Nz, DataType>::getDirectNeighborCellInfos(const int sibling_number, const int dir) const {
  unsigned sibling_coord_1, sibling_coord_2, sibling_coord_3;
  std::tie(sibling_coord_1, sibling_coord_2, sibling_coord_3) = siblingNumberToCoords(sibling_number);

  // Reference the coordinate to modify to access neighbor
  unsigned &sibling_coord = dir<2 ? sibling_coord_1 : dir < 4 ? sibling_coord_2 : sibling_coord_3;

  // Determine if the neighbor sibling number and its sibling number
  bool neighbor_is_sibling = false;
  const int Ndir = dir<2 ? N1 : dir < 4 ? N2 : N3;
  if (dir%2 == 0) {
    neighbor_is_sibling = sibling_coord > 0;
    sibling_coord = (sibling_coord+Ndir-1) % Ndir;
  } else {
    neighbor_is_sibling = sibling_coord % Ndir < (Ndir-1);
    sibling_coord = (sibling_coord+1) % Ndir;
  }
  unsigned neighbor_sibling_number = coordsToSiblingNumber(sibling_coord_1, sibling_coord_2, sibling_coord_3);

  return std::make_pair(neighbor_is_sibling, neighbor_sibling_number);
}

// Get a pointer to a neighbor cell accessible by 2 consecutive othogonal direction (corners in 2D)
template<int Nx, int Ny, int Nz, typename DataType>
std::shared_ptr<Cell<Nx, Ny, Nz, DataType>> Cell<Nx, Ny, Nz, DataType>::getPlaneNeighborCell(const int sibling_number, const int dir1, const int dir2) const {
  unsigned sibling_coord_1, sibling_coord_2, sibling_coord_3;
  std::tie(sibling_coord_1, sibling_coord_2, sibling_coord_3) = siblingNumberToCoords(sibling_number);

  // Get the neighbors infos in each directions
  bool neighbor_is_sibling1; unsigned neighbor_sibling_number1;
  std::tie(neighbor_is_sibling1, neighbor_sibling_number1) = getDirectNeighborCellInfos(sibling_number, dir1);
  bool neighbor_is_sibling2;
  std::tie(neighbor_is_sibling2, std::ignore) = getDirectNeighborCellInfos(sibling_number, dir2);

  std::shared_ptr<Cell> neighbor_cell1 = getNeighborCell(dir1),
                        neighbor_cell2 = getNeighborCell(dir2);
  if (!neighbor_cell1 && !neighbor_cell2)
    return nullptr;
  else if (neighbor_is_sibling1) // Going through the sibling first ensure the good neighbor
    return neighbor_cell1->getNeighborCell(dir2);
  else if (neighbor_is_sibling2) // Going through the sibling first ensure the good neighbor
    return neighbor_cell2->getNeighborCell(dir1);

  std::shared_ptr<Cell> neighbor_cell12 = neighbor_cell1 ? neighbor_cell1->getNeighborCell(dir2) : nullptr;
  std::shared_ptr<Cell> neighbor_cell21 = neighbor_cell2 ? neighbor_cell2->getNeighborCell(dir1) : nullptr;

  if (!neighbor_cell12 && !neighbor_cell21)
    return nullptr;

  if (neighbor_cell12.get() != neighbor_cell21.get()) // Keep the cell with the highest level
    return (neighbor_cell12->getLevel()>=neighbor_cell21->getLevel()) ? neighbor_cell12 : neighbor_cell21;

  if (neighbor_cell12->isLeaf() || neighbor_cell12->getLevel()>=getLevel())
    return neighbor_cell12;
  else {
    unsigned neighbor_sibling_number12;
    std::tie(std::ignore, neighbor_sibling_number12) = getDirectNeighborCellInfos(neighbor_sibling_number1, dir2);
    return neighbor_cell12->getChildOct()->getChildCell(neighbor_sibling_number12);
  }
}

// Get a pointer to a neighbor cell accessible by 3 consecutive othogonal direction (corners in 3D)
template<int Nx, int Ny, int Nz, typename DataType>
std::shared_ptr<Cell<Nx, Ny, Nz, DataType>> Cell<Nx, Ny, Nz, DataType>::getVolumeNeighborCell(const int sibling_number, const int dir1, const int dir2, const int dir3) const {
  throw std::runtime_error("3D implementation not done yet in Cell::getVolumeNeighborCell()");
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
