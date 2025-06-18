#include "BitStackedCellIdManager.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType>
BitStackedCellIdManager<CellType>::BitStackedCellIdManager(const int number_root_cells, const int max_level)
: CellIdManager<CellType>(number_root_cells, max_level) {
  bits_for_root = (number_root_cells <= 1) ? 0 : std::ceil(std::log2(number_root_cells));
  bits_for_level = std::ceil(std::log2(max_level+1));
  bits_per_child = std::ceil(std::log2(CellType::number_children));
  bits_per_unsigned = std::numeric_limits<unsigned>::digits;
  nb_child_first_unsigned = (bits_per_unsigned - bits_for_root - bits_for_level) / bits_per_child;
  nb_child_unsigned = bits_per_unsigned / bits_per_child;
  this->cell_id_size = (nb_child_first_unsigned >= max_level) ? 1 : (1 + (max_level - nb_child_first_unsigned + nb_child_unsigned - 1 ) / nb_child_unsigned);
  mask_level = (1U << bits_for_level) - 1;
  mask_root = (1U << bits_for_root) - 1;
  mask_child = (1U << bits_per_child) - 1;
}


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Extract the cell ID level
template<typename CellType>
int BitStackedCellIdManager<CellType>::getIdLevel(const std::vector<unsigned> &cell_id) const {
  // Decode the level
  return (cell_id[0] >> (bits_per_unsigned - bits_for_level)) & mask_level;
}

// Edit the cell ID level and return the old one
template<typename CellType>
void BitStackedCellIdManager<CellType>::setIdLevel(std::vector<unsigned> &cell_id, const int level) const {
  // Resets associated bits
  cell_id[0] &= ~(mask_level << (bits_per_unsigned - bits_for_level));
  // Encode the new level
  cell_id[0] |= static_cast<unsigned>(level) << (bits_per_unsigned - bits_for_level);
}

// Extract the cell_id root index
template<typename CellType>
unsigned BitStackedCellIdManager<CellType>::getIdRoot(const std::vector<unsigned> &cell_id) const {
  // Extract root index
  if (this->number_root_cells > 1)
    return (cell_id[0] >> (bits_per_unsigned - bits_for_level - bits_for_root)) & mask_root;
  return 0;
}

// Edit the cell_id root index and return the old one
template<typename CellType>
void BitStackedCellIdManager<CellType>::setIdRoot(std::vector<unsigned> &cell_id, const unsigned root_number) const {
  if (this->number_root_cells > 1) {
    // Resets associated bits
    cell_id[0] &= ~(((1U << (bits_for_root)) - 1) << (bits_per_unsigned - bits_for_level - bits_for_root));
    // Encode root index in low bits of cell_id[0]
    cell_id[0] |= static_cast<unsigned>(root_number) << (bits_per_unsigned - bits_for_level - bits_for_root);
  }
}

// Extract the cell_id child index
template<typename CellType>
unsigned BitStackedCellIdManager<CellType>::getIdChild(const std::vector<unsigned> &cell_id, const int level) const {
  // Determine the slot and offset for injection into id
  int slot = 0, offset = 0;
  if (level <= nb_child_first_unsigned) {
    offset = (nb_child_first_unsigned-level) * bits_per_child;
  } else {
    slot = 1 + (level-nb_child_first_unsigned) / nb_child_unsigned;
    offset = (level-nb_child_first_unsigned) % nb_child_unsigned * bits_per_child;
  }

  // Use masking & to extract from id
  return static_cast<unsigned>((cell_id[slot] >> offset) & mask_child);
}

// Edit the cell_id child index and return the old one
template<typename CellType>
void BitStackedCellIdManager<CellType>::setIdChild(std::vector<unsigned> &cell_id, const int level, const unsigned child_index) const {
  // Determine the slot and offset for injection into id
  int slot = 0, offset = 0;
  if (level <= nb_child_first_unsigned) {
    offset = (nb_child_first_unsigned-level) * bits_per_child;
  } else {
    slot = 1 + (level-nb_child_first_unsigned) / nb_child_unsigned;
    offset = (level-nb_child_first_unsigned) % nb_child_unsigned * bits_per_child;
  }

  // Resets associated bits
  cell_id[slot] &= ~(mask_child << offset);
  // Use bitwuise OR to inject into cell id
  cell_id[slot] |= static_cast<unsigned>(child_index) << offset;
}
