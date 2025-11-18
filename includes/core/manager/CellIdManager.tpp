#include "CellIdManager.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType>
CellIdManager<CellType>::CellIdManager(const int number_root_cells, const int max_level)
: number_root_cells(number_root_cells),
  max_level(max_level) {
  cell_id_size = (number_root_cells > 1) ? (max_level+2) : (max_level+1);
}


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Generate an ID from the genealogy of a cell.
template<typename CellType>
std::vector<unsigned> CellIdManager<CellType>::orderPathToId(const std::vector<unsigned> &order_path) const {
  std::vector<unsigned> cell_id(cell_id_size, 0);

  // Encode the level (size of order_path - 1)
  setIdLevel(cell_id, order_path.size()-1);

  // Encode root order in low bits of cell_id[0]
  setIdRoot(cell_id, order_path[0]);

  // Encode the rest of the order_path
  for (unsigned l{1}; l<order_path.size(); ++l)
    // Use bitwuise OR to inject into cell id
    setIdChild(cell_id, l, order_path[l]);

  return cell_id;
}

// Generate an ID from the genealogy of a cell.
template<typename CellType>
std::vector<unsigned> CellIdManager<CellType>::idToOrderPath(const std::vector<unsigned> &cell_id) const {
  // Extract level (number of entries in order path - 1)
  unsigned level = getIdLevel(cell_id);

  std::vector<unsigned> order_path(level+1, 0);

  // Extract root order
  order_path[0] = getIdRoot(cell_id);;

  // Decode the rest of the order_path
  for (unsigned l{1}; l<order_path.size(); ++l)
    // Use masking & to extract from id
    order_path[l] = getIdChild(cell_id, l);

  return order_path;
}

// Generate the IDs of the first and last leaf cells of the
// partitions obatined by splitting into equal parts and taking
// the n-th one
template<typename CellType>
std::vector<std::vector<unsigned>> CellIdManager<CellType>::getEqualPartitions(const int level, const int size) const {
  // Compute all partitions start and end positions
  std::vector<std::vector<unsigned>> partitions(size);

  // Start order path and cell ID
  std::vector<unsigned> order_path(level+1, 0);
  // End order path and cell ID
  partitions[0] = orderPathToId(order_path);

  // Define the partitions for each processor
  for (int rank{1}; rank<size; ++rank) {
    double reminder = static_cast<double>(rank * number_root_cells) / size;
    for (int i{0}; i<=level; ++i) {
      reminder = (reminder < 0) ? 0 : reminder;
      order_path[i] = std::ceil(static_cast<unsigned>(reminder));
      order_path[i] = (order_path[i] > (CellType::number_children-1)) ? (CellType::number_children-1) : order_path[i];
      reminder = CellType::number_children * (reminder - order_path[i]);
    }

    // Compute cell Id of cell between partitions rank and rank+1
    partitions[rank] = orderPathToId(order_path);
  }

  return partitions;
}

// Extract the cell ID level
template<typename CellType>
int CellIdManager<CellType>::getIdLevel(const std::vector<unsigned> &cell_id) const {
  // Decode the level
  return cell_id[0];
}

// Edit the cell ID level and return the old one
template<typename CellType>
void CellIdManager<CellType>::setIdLevel(std::vector<unsigned> &cell_id, const int level) const {
  // Encode the new level
  cell_id[0] = static_cast<unsigned>(level);
}

// Extract the cell_id root index
template<typename CellType>
unsigned CellIdManager<CellType>::getIdRoot(const std::vector<unsigned> &cell_id) const {
  // Decode root index
  if (number_root_cells > 1)
    return cell_id[1];
  return 0;
}

// Edit the cell_id root index and return the old one
template<typename CellType>
void CellIdManager<CellType>::setIdRoot(std::vector<unsigned> &cell_id, const unsigned root_number) const {
  if (number_root_cells > 1)
    // Encode root index
    cell_id[1] = static_cast<unsigned>(root_number);
}

// Extract the cell_id child index
template<typename CellType>
unsigned CellIdManager<CellType>::getIdChild(const std::vector<unsigned> &cell_id, const int level) const {
  // Decode child index
  if (number_root_cells > 1)
    return static_cast<unsigned>(cell_id[level+1]);
  else
    return static_cast<unsigned>(cell_id[level]);
}

// Edit the cell_id child index and return the old one
template<typename CellType>
void CellIdManager<CellType>::setIdChild(std::vector<unsigned> &cell_id, const int level, const unsigned child_index) const {
  // Encode child index
  if (number_root_cells > 1)
    cell_id[level+1] = static_cast<unsigned>(child_index);
  else
    cell_id[level] = static_cast<unsigned>(child_index);
}

// Moves the cell ID to a leaf cell
template<typename CellType>
void CellIdManager<CellType>::toLeaf(std::vector<unsigned> &cell_id, const int level, const bool reverse) const {
  // Edit the cell_id level and keep the old one
  unsigned old_level = getIdLevel(cell_id);
  setIdLevel(cell_id, level);

  for (unsigned l{old_level+1}; l<=level; ++l)
    if (reverse)
      setIdChild(cell_id, l, CellType::number_children - 1);
    else
      setIdChild(cell_id, l, 0);
}

// Moves the cell ID to child cell
template<typename CellType>
void CellIdManager<CellType>::toChild(std::vector<unsigned> &cell_id, const unsigned order) const {
  // Extract level (number of entries in index path - 1)
  unsigned level = getIdLevel(cell_id);
  // Encode the level (size of index_path - 1)
  setIdLevel(cell_id, level+1);
  // Use bitwuise OR to inject into cell id
  setIdChild(cell_id, level+1, order);
}

// Moves the cell ID to parent cell
template<typename CellType>
void CellIdManager<CellType>::toParent(std::vector<unsigned> &cell_id) const {
  // Extract level (number of entries in index path - 1)
  unsigned level = getIdLevel(cell_id);
  // Encode the level (size of index_path - 1)
  setIdLevel(cell_id, level-1);
  // Use bitwuise OR to inject into cell id
  setIdChild(cell_id, level, 0); // TODO: To keep unsued bits clean, should not be necessary
}

// Moves the cell ID to root cell
template<typename CellType>
void CellIdManager<CellType>::toRoot(std::vector<unsigned> &cell_id, const unsigned root_number) const {
  // Set ID level to zero
  setIdLevel(cell_id, 0);
  // Reset cell ID first root
  resetCellID(cell_id); // TODO: To keep unsued bits clean, should not be necessary
  // Set root to desired root number
  setIdRoot(cell_id, root_number);
}

// Reset cell ID
template<typename CellType>
void CellIdManager<CellType>::resetCellID(std::vector<unsigned> &cell_id) const {
  // Reset cell ID first root
  cell_id.assign(cell_id_size, 0);
}

// Check if a cell ID is greater than
template<typename CellType>
bool CellIdManager<CellType>::cellIdGt(const std::vector<unsigned> &cell_id_1, const std::vector<unsigned> &cell_id_2, bool &sure) const {
  sure = true;
  // If root is different easy to conclude
  if (getIdRoot(cell_id_1) > getIdRoot(cell_id_2))
    return true;
  if (getIdRoot(cell_id_1) < getIdRoot(cell_id_2))
    return false;

  // Get cell ID levels
  unsigned level_1 = getIdLevel(cell_id_1),
           level_2 = getIdLevel(cell_id_2),
           min_level = level_1 < level_2 ? level_1 : level_2;

  // Decode the rest of the index_path
  for (unsigned l{1}; l<=min_level; ++l) {
    if (getIdChild(cell_id_1, l) > getIdChild(cell_id_2, l))
      return true;
    if (getIdChild(cell_id_1, l) < getIdChild(cell_id_2, l))
      return false;
  }

  sure = false;
  return false;
}
template<typename CellType>
bool CellIdManager<CellType>::cellIdGt(const std::vector<unsigned> &cell_id_1, const std::vector<unsigned> &cell_id_2) const {
  bool sure;
  return cellIdGt(cell_id_1, cell_id_2, sure);
}

// Check if a cell ID is greater than or equal another ID
template<typename CellType>
bool CellIdManager<CellType>::cellIdGte(const std::vector<unsigned> &cell_id_1, const std::vector<unsigned> &cell_id_2) const {
  bool sure, greater = cellIdGt(cell_id_1, cell_id_2, sure);
  if (sure)
    return greater;

  // Get cell ID levels
  unsigned level_1 = getIdLevel(cell_id_1),
           level_2 = getIdLevel(cell_id_2);

  if (level_1 < level_2)
    for (unsigned l{level_1+1}; l<=level_2; ++l)
      if (getIdChild(cell_id_2, l) != 0)
        return false;
  if (level_1 == level_2)
    return getIdChild(cell_id_1, level_1) >= getIdChild(cell_id_2, level_2);

  return true;
}

// Check if a cell ID is smaller than
template<typename CellType>
bool CellIdManager<CellType>::cellIdLt(const std::vector<unsigned> &cell_id_1, const std::vector<unsigned> &cell_id_2, bool &sure) const {
  sure = true;
  // If root is different easy to conclude
  if (getIdRoot(cell_id_1) < getIdRoot(cell_id_2))
    return true;
  if (getIdRoot(cell_id_1) > getIdRoot(cell_id_2))
    return false;

  // Get cell ID levels
  unsigned level_1 = getIdLevel(cell_id_1),
           level_2 = getIdLevel(cell_id_2),
           min_level = level_1 < level_2 ? level_1 : level_2;

  // Decode the rest of the index_path
  for (unsigned l{1}; l<=min_level; ++l) {
    if (getIdChild(cell_id_1, l) < getIdChild(cell_id_2, l))
      return true;
    if (getIdChild(cell_id_1, l) > getIdChild(cell_id_2, l))
      return false;
  }

  sure = false;
  return false;
}
template<typename CellType>
bool CellIdManager<CellType>::cellIdLt(const std::vector<unsigned> &cell_id_1, const std::vector<unsigned> &cell_id_2) const {
  bool sure;
  return cellIdLt(cell_id_1, cell_id_2, sure);
}

// Check if a cell ID is smaller than or equal another ID
template<typename CellType>
bool CellIdManager<CellType>::cellIdLte(const std::vector<unsigned> &cell_id_1, const std::vector<unsigned> &cell_id_2) const {
  bool sure, lower = cellIdLt(cell_id_1, cell_id_2, sure);
  if (sure)
    return lower;

  // Get cell ID levels
  unsigned level_1 = getIdLevel(cell_id_1),
           level_2 = getIdLevel(cell_id_2);

  if (level_1 > level_2)
    for (unsigned l{level_2+1}; l<=level_1; ++l)
      if (getIdChild(cell_id_1, l) != (CellType::number_children - 1))
        return false;
  if (level_1 == level_2)
    return getIdChild(cell_id_1, level_1) <= getIdChild(cell_id_2, level_2);

  return true;
}
