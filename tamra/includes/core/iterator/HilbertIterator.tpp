#include "HilbertIterator.h"

//***********************************************************//
//  PROTOTYPES.                                              //
//***********************************************************//

template<typename CellType>
auto compute_orderings();

std::tuple<std::vector<unsigned>,
          std::vector<std::vector<unsigned>>,
          std::vector<std::vector<unsigned>>> compute_orderings_vec(const unsigned number_split_dimensions, const unsigned Nx, const unsigned Ny, const unsigned Nz);

std::tuple<std::vector<unsigned>,
          std::vector<std::vector<unsigned>>,
          std::vector<std::vector<unsigned>>> compute_orderings_1d(const unsigned Ni);

std::tuple<std::vector<unsigned>,
           std::vector<std::vector<unsigned>>,
           std::vector<std::vector<unsigned>>> compute_orderings_quad_tree();

std::tuple<std::vector<unsigned>,
          std::vector<std::vector<unsigned>>,
          std::vector<std::vector<unsigned>>> compute_orderings_oct_tree();

template<unsigned NUMBER_CORNERS, unsigned NUMBER_ORIENTATIONS>
std::array<std::array<unsigned, NUMBER_CORNERS>, NUMBER_ORIENTATIONS> reverse_orderings(const std::array<std::array<unsigned, NUMBER_CORNERS>, NUMBER_ORIENTATIONS> &orderings) {
  std::array<std::array<unsigned, NUMBER_CORNERS>, NUMBER_ORIENTATIONS> reverse_orderings;
  for (int i{0}; i<NUMBER_ORIENTATIONS; ++i) {
    for (int j{0}; j<NUMBER_CORNERS; ++j)
      reverse_orderings[i][orderings[i][j]] = j;
  }
  return reverse_orderings;
}

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//
// Constructor
template <typename CellType>
HilbertIterator<CellType>::HilbertIterator(const std::vector<std::shared_ptr<CellType>> &root_cells, const int max_level)
: AbstractTreeIterator<CellType>(root_cells, max_level),
  orderings_tuple(compute_orderings<CellType>()),
  possible_leaf_orientations(std::get<0>(orderings_tuple)),
  child_orderings(std::get<1>(orderings_tuple)),
  child_orientations(std::get<2>(orderings_tuple)),
  reverse_child_orderings(reverse_orderings<number_of_corners, number_of_orientations>(child_orderings)),
  default_leaf_orientation(possible_leaf_orientations[0]) {

  root_cell_orientations = std::vector<unsigned>(root_cells.size(), default_leaf_orientation);
  orientation_path.reserve(max_level+1);
  orientation_path.resize(1);
  orientation_path[0] = root_cell_orientations[index_path[0]];
};

//***********************************************************//
//  ACCESSORS                                                //
//***********************************************************//

//***********************************************************//
//  METHODS                                                  //
//***********************************************************//

// Converts the genealogy of a cell
template <typename CellType>
std::vector<unsigned> HilbertIterator<CellType>::indexToOrderPath(const std::vector<unsigned> &index_path) const {
  std::vector<unsigned> order_path(index_path.size());
  order_path[0] = index_path[0];
  unsigned orientation = root_cell_orientations[index_path[0]];
  for (int i{1}; i<index_path.size(); ++i) {
    order_path[i] = reverse_child_orderings[orientation][index_path[i]];
    orientation = child_orientations[orientation][order_path[i]];
  }
  return order_path;
}

// Return the sibling number from the order (number along
// the curve) with respect to the mother orientation.
// For Mortong Z curve sibling_number = order
template <typename CellType>
unsigned HilbertIterator<CellType>::orderToSiblingNumber(unsigned order, const bool compute_orientation) const {
  if (compute_orientation)
    return child_orderings[nextOrientation(order)][order];
  else
    return child_orderings[orientation_path.back()][order];
}

// Go to child cell
template <typename CellType>
void HilbertIterator<CellType>::toChild(const unsigned order) {
  orientation_path.push_back(nextOrientation(order));
  AbstractTreeIterator<CellType>::toChild(order);
}

// Go to parent cell
template <typename CellType>
void HilbertIterator<CellType>::toParent() {
  orientation_path.pop_back();
  AbstractTreeIterator<CellType>::toParent();
}

// Go to parent cell
template <typename CellType>
void HilbertIterator<CellType>::toRoot(const unsigned root_number) {
  orientation_path.clear();
  orientation_path.push_back(root_cell_orientations[root_number]);
  AbstractTreeIterator<CellType>::toRoot(root_number);
}

// Next orientation
template <typename CellType>
unsigned HilbertIterator<CellType>::nextOrientation(unsigned order) const {
  return child_orientations[orientation_path.back()][order];
}

//***********************************************************//
//  IMPLEMEBNTATIONS                                         //
//***********************************************************//

template<typename CellType>
auto compute_orderings() {
  static constexpr unsigned number_of_corners = 1u << CellType::number_split_dimensions;
  static constexpr unsigned number_of_orientations = CellType::number_split_dimensions * number_of_corners;

  std::tuple<std::vector<unsigned>,
             std::vector<std::vector<unsigned>>,
             std::vector<std::vector<unsigned>>> orderings = compute_orderings_vec(CellType::number_split_dimensions, CellType::Nx, CellType::Ny, CellType::Nz);

  std::vector<unsigned> possible_leaf_orientations = std::get<0>(orderings);
  // Cast child orderings to array of arrays for faster access
  std::array<std::array<unsigned, number_of_corners>, number_of_orientations> child_orderings;
  std::vector<std::vector<unsigned>> child_orderings_vec = std::get<1>(orderings);
  for (int i{0}; i<number_of_orientations; ++i)
    for (int j{0}; j<number_of_corners; ++j)
      child_orderings[i][j] = child_orderings_vec[i][j];
  // Cast child orientations to array of arrays for faster access
  std::array<std::array<unsigned, number_of_corners>, number_of_orientations> child_orientations;
  std::vector<std::vector<unsigned>> child_orientations_vec = std::get<2>(orderings);
  for (int i{0}; i<number_of_orientations; ++i)
    for (int j{0}; j<number_of_corners; ++j)
      child_orientations[i][j] = child_orientations_vec[i][j];

  return std::make_tuple(possible_leaf_orientations, child_orderings, child_orientations);
}

std::tuple<std::vector<unsigned>,
          std::vector<std::vector<unsigned>>,
          std::vector<std::vector<unsigned>>> compute_orderings_vec(const unsigned number_split_dimensions, const unsigned Nx, const unsigned Ny, const unsigned Nz) {
  const unsigned Nx1 = Nx > 1 ? Nx : 1,
                 Ny1 = Ny > 1 ? Ny : 1,
                 Nz1 = Nz > 1 ? Nz : 1,
                 Ni = Nx1 > 1 ? Nx1 : Ny1 > 1 ? Ny1 : Nz1,
                 Nj = (Nx1 > 1 && Ny1 > 1) ? Ny1 : Nz1,
                 Nk = Nz1;
  if (number_split_dimensions == 1)
    return compute_orderings_1d(Ni);
  else if ((number_split_dimensions == 2) && (Ni == 2) && (Nj == 2))
    return compute_orderings_quad_tree();
  else if ((number_split_dimensions == 2) && (Ni == 2) && (Nj == 2) && (Nk == 2))
    return compute_orderings_oct_tree();
  throw std::runtime_error(
    "HilbertIterator does not handle arbitrary cell splitting "
    + ("(" + std::to_string(Nx) + ", " + std::to_string(Ny) + ", " + std::to_string(Nz) + ").") +
    "Only 1D, 2D quad-tree, and 3D oct-tree structures are supported. "
    "Consider using MortonIterator for arbitrary splitting for the moment."
  );
}

std::tuple<std::vector<unsigned>,
          std::vector<std::vector<unsigned>>,
          std::vector<std::vector<unsigned>>> compute_orderings_1d(const unsigned Ni) {
  std::vector<unsigned> increasing(Ni), decreasing(Ni);
  std::iota(increasing.begin(), increasing.end(), 0);  // 0, 1, 2, ..., Ni-1
  std::iota(decreasing.rbegin(), decreasing.rend(), 0); // Ni-1, ..., 2, 1, 0
  return std::make_tuple<std::vector<unsigned>, std::vector<std::vector<unsigned>>, std::vector<std::vector<unsigned>>>({
      0, 1
    }, {
      increasing,
      decreasing
    }, {
      std::vector<unsigned>(Ni, 0),
      std::vector<unsigned>(Ni, 1)
    }
  );
}

// Creation of tables needed for ordering in 2D:
// Numerotation of child cells (cell index) :
//
//     (0,1,0) C2 ──── C3 (1,1,0)
//              │      │
//              │      │
//     (0,0,0) C0 ──── C1 (1,0,0)
//
// We can start from each corner and end in the others so there is dimension times number of corners orientations.
// 8 possible orientations in 2D (24 orientrations in 3D):
//
//  O0 : C0 -> C1      O0     O1     O2     O3
//  O1 : C0 -> C2     ┌──┐   <──┐   ┌──┐   ┌──>
//  O2 : C1 -> C0     │  v   ───┘   v  │   └───
//  O3 : C1 -> C3
//  O4 : C2 -> C0
//  O5 : C2 -> C3      O4     O5     O6     O7
//  O6 : C3 -> C1     ───┐   │  ^   ┌───   ^  │
//  O7 : C3 -> C2     <──┘   └──┘   └──>   └──┘
//
std::tuple<std::vector<unsigned>,
          std::vector<std::vector<unsigned>>,
          std::vector<std::vector<unsigned>>> compute_orderings_quad_tree() {
  return std::make_tuple<std::vector<unsigned>, std::vector<std::vector<unsigned>>, std::vector<std::vector<unsigned>>>({
      0, 1, 2, 3, 4, 5, 6, 7
    }, {
      { 0, 2, 3, 1 }, //  O0 : C0 -> C1
      { 0, 1, 3, 2 }, //  O1 : C0 -> C2
      { 1, 3, 2, 0 }, //  O2 : C1 -> C0     (0,1,0) C2 ──── C3 (1,1,0)
      { 1, 0, 2, 3 }, //  O3 : C1 -> C3              │      │
      { 2, 3, 1, 0 }, //  O4 : C2 -> C0              │      │
      { 2, 0, 1, 3 }, //  O5 : C2 -> C3     (0,0,0) C0 ──── C1 (1,0,0)
      { 3, 2, 0, 1 }, //  O6 : C3 -> C1
      { 3, 1, 0, 2 }  //  O7 : C3 -> C2
    }, {
      { 1, 0, 0, 6 }, //  O0 : C0 -> C1
      { 0, 1, 1, 7 }, //  O1 : C0 -> C2
      { 3, 2, 2, 4 }, //  O2 : C1 -> C0
      { 2, 3, 3, 5 }, //  O3 : C1 -> C3
      { 5, 4, 4, 2 }, //  O4 : C2 -> C0
      { 4, 5, 5, 3 }, //  O5 : C2 -> C3
      { 7, 6, 6, 0 }, //  O6 : C3 -> C1
      { 6, 7, 7, 1 }  //  O7 : C3 -> C2
    }
  );
}

std::tuple<std::vector<unsigned>,
          std::vector<std::vector<unsigned>>,
          std::vector<std::vector<unsigned>>> compute_orderings_oct_tree() {
  return std::make_tuple<std::vector<unsigned>, std::vector<std::vector<unsigned>>, std::vector<std::vector<unsigned>>>({
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
    }, {
      { 0, 2, 6, 4, 5, 7, 3, 1 }, //  O0 : C0 -> C1
      { 0, 4, 5, 1, 3, 7, 6, 2 }, //  O1 : C0 -> C2
      { 0, 1, 3, 2, 6, 7, 5, 4 }, //  O2 : C0 -> C4
      { 1, 5, 7, 3, 2, 6, 4, 0 }, //  O3 : C1 -> C0
      { 1, 0, 4, 5, 7, 6, 2, 3 }, //  O4 : C1 -> C3
      { 1, 3, 2, 0, 4, 6, 7, 5 }, //  O5 : C1 -> C5
      { 2, 3, 7, 6, 4, 5, 1, 0 }, //  O6 : C2 -> C0
      { 2, 6, 4, 0, 1, 5, 7, 3 }, //  O7 : C2 -> C3
      { 2, 0, 1, 3, 7, 5, 4, 6 }, //  O8 : C2 -> C6        (0,1,1) C6 -------- C7 (1,1,1)
      { 3, 7, 6, 2, 0, 4, 5, 1 }, //  O9 : C3 -> C1                /|         /|
      { 3, 1, 5, 7, 6, 4, 0, 2 }, // O10 : C3 -> C2               / |        / |
      { 3, 2, 0, 1, 5, 4, 6, 7 }, // O11 : C3 -> C7     (0,0,1) C4 -------- C5 | (1,0,1)
      { 4, 6, 7, 5, 1, 3, 2, 0 }, // O12 : C4 -> C0              |  |       |  |
      { 4, 0, 2, 6, 7, 3, 1, 5 }, // O13 : C4 -> C5      (0,1,0) | C2 ------|- C3 (1,1,0)
      { 4, 5, 1, 0, 2, 3, 7, 6 }, // O14 : C4 -> C6              | /        | /
      { 5, 4, 6, 7, 3, 2, 0, 1 }, // O15 : C5 -> C1     (0,0,0) C0 -------- C1 (1,0,0)
      { 5, 7, 3, 1, 0, 2, 6, 4 }, // O16 : C5 -> C4
      { 5, 1, 0, 4, 6, 2, 3, 7 }, // O17 : C5 -> C7
      { 6, 7, 5, 4, 0, 1, 3, 2 }, // O18 : C6 -> C2
      { 6, 2, 3, 7, 5, 1, 0, 4 }, // O19 : C6 -> C4
      { 6, 4, 0, 2, 3, 1, 5, 7 }, // O20 : C6 -> C7
      { 7, 5, 4, 6, 2, 0, 1, 3 }, // O21 : C7 -> C3
      { 7, 6, 2, 3, 1, 0, 4, 5 }, // O22 : C7 -> C5
      { 7, 3, 1, 5, 4, 0, 2, 6 }  // O23 : C7 -> C6
    }, {
      {  1,  2,  2, 20, 20, 15, 15,  9 }, //  O0 : C0 -> C1
      {  2,  0,  0, 17, 17, 10, 10, 18 }, //  O1 : C0 -> C2
      {  0,  1,  1, 11, 11, 19, 19, 16 }, //  O2 : C0 -> C4
      {  5,  4,  4, 23, 23,  6,  6, 12 }, //  O3 : C1 -> C0
      {  3,  5,  5, 14, 14, 21, 21,  7 }, //  O4 : C1 -> C3
      {  4,  3,  3,  8,  8, 13, 13, 22 }, //  O5 : C1 -> C5
      {  7,  8,  8, 22, 22, 12, 12,  3 }, //  O6 : C2 -> C0
      {  8,  6,  6, 13, 13,  4,  4, 21 }, //  O7 : C2 -> C3
      {  6,  7,  7,  5,  5, 23, 23, 14 }, //  O8 : C2 -> C6
      { 11, 10, 10, 19, 19,  0,  0, 15 }, //  O9 : C3 -> C1
      {  9, 11, 11, 16, 16, 18, 18,  1 }, // O10 : C3 -> C2
      { 10,  9,  9,  2,  2, 17, 17, 20 }, // O11 : C3 -> C7
      { 14, 13, 13, 21, 21,  3,  3,  6 }, // O12 : C4 -> C0
      { 12, 14, 14,  7,  7, 22, 22,  5 }, // O13 : C4 -> C5
      { 13, 12, 12,  4,  4,  8,  8, 23 }, // O14 : C4 -> C6
      { 16, 17, 17, 18, 18,  9,  9,  0 }, // O15 : C5 -> C1
      { 17, 15, 15, 10, 10,  2,  2, 19 }, // O16 : C5 -> C4
      { 15, 16, 16,  1,  1, 20, 20, 11 }, // O17 : C5 -> C7
      { 20, 19, 19, 15, 15,  1,  1, 10 }, // O18 : C6 -> C2
      { 18, 20, 20,  9,  9, 16, 16,  2 }, // O19 : C6 -> C4
      { 19, 18, 18,  0,  0, 11, 11, 17 }, // O20 : C6 -> C7
      { 22, 23, 23, 12, 12,  7,  7,  4 }, // O21 : C7 -> C3
      { 23, 21, 21,  6,  6,  5,  5, 13 }, // O22 : C7 -> C5
      { 21, 22, 22,  3,  3, 14, 14,  8 }  // O23 : C7 -> C6 
    }
  );
}
