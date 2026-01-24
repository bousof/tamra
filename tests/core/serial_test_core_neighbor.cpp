#include <doctest.h>

#include <cmath>
#include <memory>
#include <vector>

#include <core/Cell.h>
#include <core/CellData.h>
#include <core/manager/CellIdManager.h>
#include <core/RootCellEntry.h>
#include <core/Tree.h>

namespace core::neighbor {
template<int Nx, int Ny, int Nz>
class TestCellData : public CellData {
 public:
  unsigned imin;   // Minimum i-coordinate
  unsigned imax;   // Maximum i-coordinate
  unsigned jmin;   // Minimum j-coordinate
  unsigned jmax;   // Maximum j-coordinate
  unsigned kmin;   // Minimum k-coordinate
  unsigned kmax;   // Maximum k-coordinate

  // Constructor
  TestCellData()
  : imin(0), imax(0), jmin(0), jmax(0), kmin(0), kmax(0) {};
  // Destructor
  ~TestCellData() = default;

  // Extrapolate from current cell to child cells
  void extrapolateToChild(TestCellData &child_cell_data, const unsigned sibling_number) const {
    const unsigned i = sibling_number % Nx,
                    j = (sibling_number / Nx) % Ny,
                    k = (sibling_number / (Nx * Ny));

    child_cell_data.imin = imin + (  i   * (imax-imin)) / Nx;
    child_cell_data.imax = imin + ((i+1) * (imax-imin)) / Nx;
    child_cell_data.jmin = jmin + (  j   * (jmax-jmin)) / Ny;
    child_cell_data.jmax = jmin + ((j+1) * (jmax-jmin)) / Ny;
    if constexpr (Nz > 0) {
      child_cell_data.kmin = kmin + (  k   * (kmax-kmin)) / Nz;
      child_cell_data.kmax = kmin + ((k+1) * (kmax-kmin)) / Nz;
    }
  }
};

template<typename CellType>
void initialize_tree_cells_limits(std::shared_ptr<CellType> cell) {
  if (cell->isLeaf())
    return;

  const auto &cell_data = cell->getCellData();
  for (unsigned n{0}; n<CellType::number_children; ++n) {
    auto &child_cell_data = cell->getChildCell(n)->getCellData();
    cell_data.extrapolateToChild(child_cell_data, n);
    initialize_tree_cells_limits<CellType>(cell->getChildCell(n));
  }
}
}

// Neighbors leaf search (2D)
//                 root A   root B
//                ┌───┬─┬─┐┌─┬─┬───┐
//                │   ├─┼─┤├─┼─┤   │
// structure  ->  ├─┬─┼─┼─┤├─┴─┼─┬─┤
//                ├─┼─┼─┼─┤│   ├─┼─┤
//                └─┴─┴─┴─┘└───┴─┴─┘
//
TEST_CASE("[core][neighbor] Neighbor leafs search (quadtree)") {
  static constexpr int Nx = 2, Ny = 2, Nz = 0;
  using Cell2D = Cell<Nx,Ny,Nz, core::neighbor::TestCellData<Nx, Ny, Nz>>;
  // Create 4 root cells
  auto A = std::make_shared<Cell2D>(nullptr);
  auto B = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A}, eB{B};
  eA.setNeighbor(1, B);          // A +x -> B
  eB.setNeighbor(0, A);          // B -x -> A
  std::vector<RootCellEntry<Cell2D>> entries { eA, eB };

  // Create the tree
  unsigned max_level{2};
  Tree<Cell2D> tree(1, max_level);
  tree.createRootCells(entries);

  // Mesh until min level
  tree.meshAtMinLevel();

  //Splitting some cells
  A->getChildCell(0)->split(max_level);
  A->getChildCell(1)->split(max_level);
  A->getChildCell(3)->split(max_level);
  B->getChildCell(1)->split(max_level);
  B->getChildCell(2)->split(max_level);

  // Set root cells data
  A->getCellData().imin = 0    ; A->getCellData().imax = Nx*Nx;
  A->getCellData().jmin = 0    ; A->getCellData().jmax = Ny*Ny;
  B->getCellData().imin = Nx*Nx; B->getCellData().imax = 2*Nx*Nx;
  B->getCellData().jmin = 0    ; B->getCellData().jmax = Ny*Ny;
  core::neighbor::initialize_tree_cells_limits(A);
  core::neighbor::initialize_tree_cells_limits(B);

  // Check neighbors in all directions
  bool passed = true;
  tree.applyToOwnedLeaves(
    [&passed](const std::shared_ptr<Cell2D> &cell, const unsigned) mutable {
      cell->applyToNeighborLeafCells(
        [&passed](const std::shared_ptr<Cell2D> &c, const std::shared_ptr<Cell2D> &n, const unsigned &dir) {
          // If no neighbor, then cooredinates corresponds to boundaries
          auto &cell_data = c->getCellData();
          if (!n) {
            passed &= (dir!=0) || (cell_data.imin==0);
            passed &= (dir!=1) || (cell_data.imax==2*Nx*Nx);
            passed &= (dir!=2) || (cell_data.jmin==0);
            passed &= (dir!=3) || (cell_data.jmax==Ny*Ny);
            passed &= (dir!=4) || (cell_data.imin==0      ) || (cell_data.jmin==0);
            passed &= (dir!=5) || (cell_data.imax==2*Nx*Nx) || (cell_data.jmin==0);
            passed &= (dir!=6) || (cell_data.imin==0      ) || (cell_data.jmax==Ny*Ny);
            passed &= (dir!=7) || (cell_data.imax==2*Nx*Nx) || (cell_data.jmax==Ny*Ny);
            CHECK(passed);
            return;
          }

          // Deduce the sibling number if the two cells have the same level
          if (c->getLevel() == n->getLevel()) {
            unsigned i_c, j_c, k_c;
            std::tie(i_c, j_c, k_c) = Cell2D::siblingNumberToCoords(c->getSiblingNumber());
            if (dir == 0 || dir == 4 || dir == 6)
              i_c = (i_c + Nx - 1) % Nx;
            if (dir == 1 || dir == 5 || dir == 7)
              i_c = (i_c + 1) % Nx;
            if (dir == 2 || dir == 4 || dir == 5)
              j_c = (j_c + Ny - 1) % Ny;
            if (dir == 3 || dir == 6 || dir == 7)
              j_c = (j_c + 1) % Ny;
            unsigned i_n, j_n, k_n;
            std::tie(i_n, j_n, k_n) = Cell2D::siblingNumberToCoords(n->getSiblingNumber());
            passed &= (i_c == i_n && j_c == j_n);
          }

          // Verify the correct computation of neighbors thanks to coordinates
          auto &neighbor_data = n->getCellData();
          if (dir == 0)
            // case 1                       case 2                            case 3
            //                      ────┬───┐    ────┬───┐            ┌───┬────    ┌───┬────
            // ┌───┬───┐                │   │        │ C │            │   │        │ N │    
            // │ N │ C │     OR       N ├───┤ OR   N ├───┤     OR     ├───┼ C   OR ├───┼ C  
            // └───┴───┘                │ C │        │   │            │ N │        │   │    
            //                      ────┴───┘    ────┴───┘            └───┴────    └───┴────
            passed &= ( (cell_data.imin == neighbor_data.imax) && (cell_data.jmin == neighbor_data.jmin) && (cell_data.jmax == neighbor_data.jmax) && (c->getLevel() == n->getLevel()) ) // case 1
                    || ( (cell_data.imin == neighbor_data.imax) && (cell_data.jmin == neighbor_data.jmin || cell_data.jmax == neighbor_data.jmax) && ( (  c->getLevel()   == (n->getLevel()+1)) // case 2
                                                                                                                                                    || ((c->getLevel()+1) ==   n->getLevel()  ) ) ); // case 3
          if (dir == 1)
            // case 1                       case 2                            case 3
            //                      ┌───┬────    ┌───┬────            ────┬───┐    ────┬───┐
            // ┌───┬───┐            │   │        │ C │                    │   │        │ N │
            // │ C │ N │     OR     ├───┼ N   OR ├───┼ N       OR       C ├───┤ OR   C ├───┤
            // └───┴───┘            │ C │        │   │                    │ N │        │   │
            //                      └───┴────    └───┴────            ────┴───┘    ────┴───┘
            passed &= ( (cell_data.imax == neighbor_data.imin) && (cell_data.jmin == neighbor_data.jmin) && (cell_data.jmax == neighbor_data.jmax) && (c->getLevel() == n->getLevel()) ) // case 1
                    || ( (cell_data.imax == neighbor_data.imin) && (cell_data.jmin == neighbor_data.jmin || cell_data.jmax == neighbor_data.jmax) && ( (  c->getLevel()   == (n->getLevel()+1)) // case 2
                                                                                                                                                    || ((c->getLevel()+1) ==   n->getLevel()  ) ) ); // case 3
          if (dir == 2)
            // case 1                   case 2                            case 3
            // ┌───┐            ┌───┬───┐    ┌───┬───┐            │       │    │       │
            // │ C │            │ C │   │    │   │ C │            │   C   │    │   C   │
            // ├───┤     OR     ├───┴───┤ OR ├───┴───┤     OR     ├───┬───┤ OR ├───┬───┤
            // │ N │            │   N   │    │   N   │            │ N │   │    │   │ N │
            // └───┘            │       │    │       │            └───┴───┘    └───┴───┘
            passed &= ( (cell_data.jmin == neighbor_data.jmax) && (cell_data.imin == neighbor_data.imin) && (cell_data.imax == neighbor_data.imax) && (c->getLevel() == n->getLevel()) ) // case 1
                    || ( (cell_data.jmin == neighbor_data.jmax) && (cell_data.imin == neighbor_data.imin || cell_data.imax == neighbor_data.imax) && ( (  c->getLevel()   == (n->getLevel()+1)) // case 2
                                                                                                                                                    || ((c->getLevel()+1) ==   n->getLevel()  ) ) ); // case 3
          if (dir == 3)
            // case 1                   case 2                            case 3
            // ┌───┐            │       │    │       │            ┌───┬───┐    ┌───┬───┐
            // │ N │            │   N   │    │   N   │            │ N │   │    │   │ N │
            // ├───┤     OR     ├───┬───┤ OR ├───┬───┤     OR     ├───┴───┤ OR ├───┴───┤
            // │ C │            │ C │   │    │   │ C │            │   C   │    │   C   │
            // └───┘            └───┴───┘    └───┴───┘            │       │    │       │
            passed &= ( (cell_data.jmax == neighbor_data.jmin) && (cell_data.imin == neighbor_data.imin) && (cell_data.imax == neighbor_data.imax) && (c->getLevel() == n->getLevel()) ) // case 1
                    || ( (cell_data.jmax == neighbor_data.jmin) && (cell_data.imin == neighbor_data.imin || cell_data.imax == neighbor_data.imax) && ( (  c->getLevel()   == (n->getLevel()+1)) // case 2
                                                                                                                                                    || ((c->getLevel()+1) ==   n->getLevel()  ) ) ); // case 3
          if (dir == 4)
            // case 1             case 2             case 3
            //                    ───┬───
            //    │ C                │ C                │ C │
            // ───┼───     OR      N ├───     OR     ───┴───┤
            //  N │                  │                  N   │
            //
            passed &= ( (cell_data.imin == neighbor_data.imax) && (cell_data.jmin == neighbor_data.jmax) && (c->getLevel() <= (n->getLevel()+2)) && ((c->getLevel()+2) >= n->getLevel()) ) // case 1
                    || ( (cell_data.imin == neighbor_data.imax) && (cell_data.jmax == neighbor_data.jmax) && (c->getLevel() == (n->getLevel()+1)) ) // case 2
                    || ( (cell_data.jmin == neighbor_data.jmax) && (cell_data.imax == neighbor_data.imax) && (c->getLevel() == (n->getLevel()+1)) ); // case 3
          if (dir == 5)
            // case 1             case 2             case 3
            //                    ───┬───
            //  C │                C │               │ C │
            // ───┼───     OR     ───┤ N      OR     ├───┴───
            //    │ N                │               │   N
            //
            passed &= ( (cell_data.imax == neighbor_data.imin) && (cell_data.jmin == neighbor_data.jmax) && (c->getLevel() <= (n->getLevel()+2)) && ((c->getLevel()+2) >= n->getLevel()) ) // case 1
                    || ( (cell_data.imax == neighbor_data.imin) && (cell_data.jmax == neighbor_data.jmax) && (c->getLevel() == (n->getLevel()+1)) ) // case 2
                    || ( (cell_data.jmin == neighbor_data.jmax) && (cell_data.imin == neighbor_data.imin) && (c->getLevel() == (n->getLevel()+1)) ); // case 3
          if (dir == 6)
            // case 1             case 2             case 3
            //
            //  N │                  │                  N   │
            // ───┼───     OR      N ├───     OR     ───┬───┤
            //    │ C                │ C                │ C │
            //                    ───┴───
            passed &= ( (cell_data.imin == neighbor_data.imax) && (cell_data.jmax == neighbor_data.jmin) && (c->getLevel() <= (n->getLevel()+2)) && ((c->getLevel()+2) >= n->getLevel()) ) // case 1
                    || ( (cell_data.imin == neighbor_data.imax) && (cell_data.jmin == neighbor_data.jmin) && (c->getLevel() == (n->getLevel()+1)) ) // case 2
                    || ( (cell_data.jmax == neighbor_data.jmin) && (cell_data.imax == neighbor_data.imax) && (c->getLevel() == (n->getLevel()+1)) ); // case 3
          if (dir == 7)
            // case 1             case 2             case 3
            //
            //    │ N                │               │   N
            // ───┼───     OR     ───┤ N      OR     ├───┬───
            //  C │                C │               │ C │
            //                    ───┴───
            passed &= ( (cell_data.imax == neighbor_data.imin) && (cell_data.jmax == neighbor_data.jmin) && (c->getLevel() <= (n->getLevel()+2)) && ((c->getLevel()+2) >= n->getLevel()) ) // case 1
                    || ( (cell_data.imax == neighbor_data.imin) && (cell_data.jmin == neighbor_data.jmin) && (c->getLevel() == (n->getLevel()+1)) ) // case 2
                    || ( (cell_data.jmax == neighbor_data.jmin) && (cell_data.imin == neighbor_data.imin) && (c->getLevel() == (n->getLevel()+1)) ); // case 3
          CHECK(passed);
        },
        false, false, { 0, 1, 2, 3, 4, 5, 6, 7 }
      );
    }
  );
}

// Check octree cell neighbors
template<typename CellType>
bool check_octree_cell(const std::shared_ptr<CellType> &cell, const unsigned imax, const unsigned jmax, const unsigned kmax) {
  static constexpr int Nx = CellType::Nx, Ny = CellType::Ny, Nz = CellType::Nz;
  bool passed{true};

  // Extract cell data
  auto &cell_data = cell->getCellData();
  for (unsigned dir{0}; dir<CellType::number_volume_neighbors; ++dir) { // TODO USE VOLUME NEIGHBORS
    if (!passed) break;

    // Get neighbor and extract cell data
    auto neighbor_cell = cell->getNeighborCell(dir);

    if (!neighbor_cell) {
      passed &= (dir!=0)  || (cell_data.imin==0   );
      passed &= (dir!=1)  || (cell_data.imax==imax);
      passed &= (dir!=2)  || (cell_data.jmin==0   );
      passed &= (dir!=3)  || (cell_data.jmax==jmax);
      passed &= (dir!=4)  || (cell_data.kmin==0   );
      passed &= (dir!=5)  || (cell_data.kmax==kmax);
      passed &= (dir!=6)  || (cell_data.imin==0   ) || (cell_data.jmin==0   );
      passed &= (dir!=7)  || (cell_data.imax==imax) || (cell_data.jmin==0   );
      passed &= (dir!=8)  || (cell_data.imin==0   ) || (cell_data.jmax==jmax);
      passed &= (dir!=9)  || (cell_data.imax==imax) || (cell_data.jmax==jmax);
      passed &= (dir!=10) || (cell_data.imin==0   ) || (cell_data.kmin==0   );
      passed &= (dir!=11) || (cell_data.imax==imax) || (cell_data.kmin==0   );
      passed &= (dir!=12) || (cell_data.imin==0   ) || (cell_data.kmax==kmax);
      passed &= (dir!=13) || (cell_data.imax==imax) || (cell_data.kmax==kmax);
      passed &= (dir!=14) || (cell_data.jmin==0   ) || (cell_data.kmin==0   );
      passed &= (dir!=15) || (cell_data.jmax==jmax) || (cell_data.kmin==0   );
      passed &= (dir!=16) || (cell_data.jmin==0   ) || (cell_data.kmax==kmax);
      passed &= (dir!=17) || (cell_data.jmax==jmax) || (cell_data.kmax==kmax);
      continue;
    }

    // Deduce the sibling number if the two cells have the same level
    if (cell->getLevel() == neighbor_cell->getLevel()) {
      unsigned i_c, j_c, k_c;
      std::tie(i_c, j_c, k_c) = CellType::siblingNumberToCoords(cell->getSiblingNumber());
      if (dir == 0 || dir == 6 || dir == 8 || dir == 10 || dir == 12 || dir == 18 || dir == 20 || dir == 22 || dir == 24)
        i_c = (i_c + Nx - 1) % Nx;
      if (dir == 1 || dir == 7 || dir == 9 || dir == 11 || dir == 13 || dir == 19 || dir == 21 || dir == 23 || dir == 25)
        i_c = (i_c + 1) % Nx;
      if (dir == 2 || dir == 6 || dir == 7 || dir == 14 || dir == 16 || dir == 18 || dir == 19 || dir == 22 || dir == 23)
        j_c = (j_c + Ny - 1) % Ny;
      if (dir == 3 || dir == 8 || dir == 9 || dir == 15 || dir == 17 || dir == 20 || dir == 21 || dir == 24 || dir == 25)
        j_c = (j_c + 1) % Ny;
      if (dir == 4 || dir == 10 || dir == 11 || dir == 14 || dir == 15 || dir == 18 || dir == 19 || dir == 20 || dir == 21)
        k_c = (k_c + Nz - 1) % Nz;
      if (dir == 5 || dir == 12 || dir == 13 || dir == 16 || dir == 17 || dir == 22 || dir == 23 || dir == 24 || dir == 25)
        k_c = (k_c + 1) % Nz;
      unsigned i_n, j_n, k_n;
      std::tie(i_n, j_n, k_n) = CellType::siblingNumberToCoords(neighbor_cell->getSiblingNumber());
      passed &= (i_c == i_n && j_c == j_n && k_c == k_n);
    }

    if (!passed) break;

    // Verify the correct computation of neighbors thanks to coordinates
    auto &neighbor_data = neighbor_cell->getCellData();

    // Determine the offset vector fro cell translation
    int i_trans{0}, j_trans{0}, k_trans{0};
    if (dir == 0 || dir == 6 || dir == 8 || dir == 10 || dir == 12 || dir == 18 || dir == 20 || dir == 22 || dir == 24)
      i_trans = -(cell_data.imax - cell_data.imin);
    if (dir == 1 || dir == 7 || dir == 9 || dir == 11 || dir == 13 || dir == 19 || dir == 21 || dir == 23 || dir == 25)
      i_trans =  (cell_data.imax - cell_data.imin);
    if (dir == 2 || dir == 6 || dir == 7 || dir == 14 || dir == 16 || dir == 18 || dir == 19 || dir == 22 || dir == 23)
      j_trans = -(cell_data.jmax - cell_data.jmin);
    if (dir == 3 || dir == 8 || dir == 9 || dir == 15 || dir == 17 || dir == 20 || dir == 21 || dir == 24 || dir == 25)
      j_trans =  (cell_data.jmax - cell_data.jmin);
    if (dir == 4 || dir == 10 || dir == 11 || dir == 14 || dir == 15 || dir == 18 || dir == 19 || dir == 20 || dir == 21)
      k_trans = -(cell_data.kmax - cell_data.kmin);
    if (dir == 5 || dir == 12 || dir == 13 || dir == 16 || dir == 17 || dir == 22 || dir == 23 || dir == 24 || dir == 25)
      k_trans =  (cell_data.kmax - cell_data.kmin);

    // Translated cell should be included in neighbor cell
    passed &= ( (cell_data.imin+i_trans) >= neighbor_data.imin )
           || ( (cell_data.imax+i_trans) <= neighbor_data.imax )
           || ( (cell_data.jmin+j_trans) >= neighbor_data.jmin )
           || ( (cell_data.jmax+j_trans) <= neighbor_data.jmax )
           || ( (cell_data.kmin+k_trans) >= neighbor_data.jmin )
           || ( (cell_data.kmax+k_trans) <= neighbor_data.jmax );

    if (!passed) break;

    // Transform coordinates to change coordinates system
    const unsigned c_imin = cell_data.imin, c_imax = cell_data.imax,
                   c_jmin = cell_data.jmin, c_jmax = cell_data.jmax,
                   c_kmin = cell_data.kmin, c_kmax = cell_data.kmax,
                   n_imin = neighbor_data.imin, n_imax = neighbor_data.imax,
                   n_jmin = neighbor_data.jmin, n_jmax = neighbor_data.jmax,
                   n_kmin = neighbor_data.kmin, n_kmax = neighbor_data.kmax,
                   c_level = cell->getLevel(), n_level = neighbor_cell->getLevel();

    // Directions 0 to 5
    // case 1                                         case 2
    //                           ________      ________      ________      ________
    //    _______                   /__ /│        /__ /│        /__ /│        /__ /│
    //  /__ /__ /│            ___ /__ /│ │  ___ /__ /│ │  ___ /__ /│ │  ___ /__ /│C│
    // │ N │ C │ │     OR        │   │ │/│     │   │ │/│     │ C │ │/│     │   │ │/│
    // │___│___│/              N │___│/│ │   N │___│/│C│   N │___│/│ │   N │___│/│ │
    //                           │ C │ │/      │   │ │/      │   │ │/      │   │ │/
    //                        ___│___│/     ___│___│/     ___│___│/     ___│___│/
    //
    // Difference of levels should be smaller or equal to 1
    if (dir == 0)
      //  AXIS      X
      // ──────>   ───>
      passed &= ( (c_imin == n_imax) && (c_jmin == n_jmin) && (c_jmax == n_jmax) && (c_kmin == n_kmin) && (c_kmax == n_kmax) && (c_level == n_level) ) // case 1
             || ( (c_imin == n_imax) && (c_jmin == n_jmin  ||  c_jmax == n_jmax  ||  c_kmin == n_kmin  ||  c_kmax == n_kmax) && (c_level == (n_level+1)) ); // case 2
    if (dir == 1)
      //  AXIS      -X
      // ──────>   ───>
      passed &= ( (c_imax == n_imin) && (c_jmin == n_jmin) && (c_jmax == n_jmax) && (c_kmin == n_kmin) && (c_kmax == n_kmax) && (c_level == n_level) ) // case 1
             || ( (c_imax == n_imin) && (c_jmin == n_jmin  ||  c_jmax == n_jmax  ||  c_kmin == n_kmin  ||  c_kmax == n_kmax) && (c_level == (n_level+1)) ); // case 2
    if (dir == 2)
      //  AXIS      Y
      // ──────>   ───>
      passed &= ( (c_jmin == n_jmax) && (c_imin == n_imin) && (c_imax == n_imax) && (c_kmin == n_kmin) && (c_kmax == n_kmax) && (c_level == n_level) ) // case 1
             || ( (c_jmin == n_jmax) && (c_imin == n_imin  ||  c_imax == n_imax  ||  c_kmin == n_kmin  ||  c_kmax == n_kmax) && (c_level == (n_level+1)) ); // case 2
    if (dir == 3)
      //  AXIS      -Y
      // ──────>   ───>
      passed &= ( (c_jmax == n_jmin) && (c_imin == n_imin) && (c_imax == n_imax) && (c_kmin == n_kmin) && (c_kmax == n_kmax) && (c_level == n_level) ) // case 1
             || ( (c_jmax == n_jmin) && (c_imin == n_imin  ||  c_imax == n_imax  ||  c_kmin == n_kmin  ||  c_kmax == n_kmax) && (c_level == (n_level+1)) ); // case 2
    if (dir == 4)
      //  AXIS      Z
      // ──────>   ───>
      passed &= ( (c_kmin == n_kmax) && (c_jmin == n_jmin) && (c_jmax == n_jmax) && (c_imin == n_imin) && (c_imax == n_imax) && (c_level == n_level) ) // case 1
             || ( (c_kmin == n_kmax) && (c_jmin == n_jmin  ||  c_jmax == n_jmax  ||  c_imin == n_imin  ||  c_imax == n_imax) && (c_level == (n_level+1)) ); // case 2
    if (dir == 5)
      //  AXIS      -Z
      // ──────>   ───>
      passed &= ( (c_kmax == n_kmin) && (c_jmin == n_jmin) && (c_jmax == n_jmax) && (c_imin == n_imin) && (c_imax == n_imax) && (c_level == n_level) ) // case 1
             || ( (c_kmax == n_kmin) && (c_jmin == n_jmin  ||  c_jmax == n_jmax  ||  c_imin == n_imin  ||  c_imax == n_imax) && (c_level == (n_level+1)) ); // case 2

    // Directions 6 to 17
    // case 1                case 2                case 3
    //       ___                   ___                   ___
    //     /__ /│            ___ /__ /│                /__ /│     ^
    //    │ C │ │               │ C │ │               │ C │ │     │
    // ___│___│/      OR      N │___│/      OR     ___│___│/      │ AXES   
    //  N │                     │                     N   |       └──────>
    //
    // Difference of levels should be smaller or equal to 2
    if (dir == 6)
      // ^            ^
      // │            │ Y
      // │ AXES       │
      // └─────>      └─────> X
      passed &= ( (c_imin == n_imax) && (c_jmin == n_jmax) && (c_kmin >= n_kmin) && (c_kmax <= n_kmax) && (c_level <= (n_level+2)) && ((c_level+2) >= n_level) ) // case 1
             || ( (c_imin == n_imax) && (c_jmin >  n_jmin) && (c_jmax <= n_jmax) && (c_kmin >= n_kmin) && (c_kmax <= n_kmax ) && (c_level == (n_level+1)) ) // case 2
             || ( (c_jmin == n_jmax) && (c_imin >  n_imin) && (c_imax <= n_imax) && (c_kmin >= n_kmin) && (c_kmax <= n_kmax ) && (c_level == (n_level+1)) ); // case 3
    if (dir == 7)
      // ^            ^
      // │            │ Y
      // │ AXES       │
      // └─────>      └─────> -X
      passed &= ( (c_imax == n_imin) && (c_jmin == n_jmax) && (c_kmin >= n_kmin) && (c_kmax <= n_kmax) && (c_level <= (n_level+2)) && ((c_level+2) >= n_level) ) // case 1
             || ( (c_imax == n_imin) && (c_jmin >  n_jmin) && (c_jmax <= n_jmax) && (c_kmin >= n_kmin) && (c_kmax <= n_kmax ) && (c_level == (n_level+1)) ) // case 2
             || ( (c_jmin == n_jmax) && (c_imax <  n_imax) && (c_imin >= n_imin) && (c_kmin >= n_kmin) && (c_kmax <= n_kmax ) && (c_level == (n_level+1)) ); // case 3
    if (dir == 8)
      // ^            ^
      // │            │ -Y
      // │ AXES       │
      // └─────>      └─────> X
      passed &= ( (c_imin == n_imax) && (c_jmax == n_jmin) && (c_kmin >= n_kmin) && (c_kmax <= n_kmax) && (c_level <= (n_level+2)) && ((c_level+2) >= n_level) ) // case 1
             || ( (c_imin == n_imax) && (c_jmax <  n_jmax) && (c_jmin >= n_jmin) && (c_kmin >= n_kmin) && (c_kmax <= n_kmax ) && (c_level == (n_level+1)) ) // case 2
             || ( (c_jmax == n_jmin) && (c_imin >  n_imin) && (c_imax <= n_imax) && (c_kmin >= n_kmin) && (c_kmax <= n_kmax ) && (c_level == (n_level+1)) ); // case 3
    if (dir == 9)
      // ^            ^
      // │            │ -Y
      // │ AXES       │
      // └─────>      └─────> -X
      passed &= ( (c_imax == n_imin) && (c_jmax == n_jmin) && (c_kmin >= n_kmin) && (c_kmax <= n_kmax) && (c_level <= (n_level+2)) && ((c_level+2) >= n_level) ) // case 1
             || ( (c_imax == n_imin) && (c_jmax <  n_jmax) && (c_jmin >= n_jmin) && (c_kmin >= n_kmin) && (c_kmax <= n_kmax ) && (c_level == (n_level+1)) ) // case 2
             || ( (c_jmax == n_jmin) && (c_imax <  n_imax) && (c_imin >= n_imin) && (c_kmin >= n_kmin) && (c_kmax <= n_kmax ) && (c_level == (n_level+1)) ); // case 3
    if (dir == 10)
      // ^            ^
      // │            │ Z
      // │ AXES       │
      // └─────>      └─────> X
      passed &= ( (c_imin == n_imax) && (c_kmin == n_kmax) && (c_jmin >= n_jmin) && (c_jmax <= n_jmax) && (c_level <= (n_level+2)) && ((c_level+2) >= n_level) ) // case 1
             || ( (c_imin == n_imax) && (c_kmin >  n_kmin) && (c_kmax <= n_kmax) && (c_jmin >= n_jmin) && (c_jmax <= n_jmax ) && (c_level == (n_level+1)) ) // case 2
             || ( (c_kmin == n_kmax) && (c_imin >  n_imin) && (c_imax <= n_imax) && (c_jmin >= n_jmin) && (c_jmax <= n_jmax ) && (c_level == (n_level+1)) ); // case 3
    if (dir == 11)
      // ^            ^
      // │            │ Z
      // │ AXES       │
      // └─────>      └─────> -X
      passed &= ( (c_imax == n_imin) && (c_kmin == n_kmax) && (c_jmin >= n_jmin) && (c_jmax <= n_jmax) && (c_level <= (n_level+2)) && ((c_level+2) >= n_level) ) // case 1
             || ( (c_imax == n_imin) && (c_kmin >  n_kmin) && (c_kmax <= n_kmax) && (c_jmin >= n_jmin) && (c_jmax <= n_jmax ) && (c_level == (n_level+1)) ) // case 2
             || ( (c_kmin == n_kmax) && (c_imax <  n_imax) && (c_imin >= n_imin) && (c_jmin >= n_jmin) && (c_jmax <= n_jmax ) && (c_level == (n_level+1)) ); // case 3
    if (dir == 12)
      // ^            ^
      // │            │ -Z
      // │ AXES       │
      // └─────>      └─────> X
      passed &= ( (c_imin == n_imax) && (c_kmax == n_kmin) && (c_jmin >= n_jmin) && (c_jmax <= n_jmax) && (c_level <= (n_level+2)) && ((c_level+2) >= n_level) ) // case 1
             || ( (c_imin == n_imax) && (c_kmax <  n_kmax) && (c_kmin >= n_kmin) && (c_jmin >= n_jmin) && (c_jmax <= n_jmax ) && (c_level == (n_level+1)) ) // case 2
             || ( (c_kmax == n_kmin) && (c_imin >  n_imin) && (c_imax <= n_imax) && (c_jmin >= n_jmin) && (c_jmax <= n_jmax ) && (c_level == (n_level+1)) ); // case 3
    if (dir == 13)
      // ^            ^
      // │            │ -Z
      // │ AXES       │
      // └─────>      └─────> -X
      passed &= ( (c_imax == n_imin) && (c_kmax == n_kmin) && (c_jmin >= n_jmin) && (c_jmax <= n_jmax) && (c_level <= (n_level+2)) && ((c_level+2) >= n_level) ) // case 1
             || ( (c_imax == n_imin) && (c_kmax <  n_kmax) && (c_kmin >= n_kmin) && (c_jmin >= n_jmin) && (c_jmax <= n_jmax ) && (c_level == (n_level+1)) ) // case 2
             || ( (c_kmax == n_kmin) && (c_imax <  n_imax) && (c_imin >= n_imin) && (c_jmin >= n_jmin) && (c_jmax <= n_jmax ) && (c_level == (n_level+1)) ); // case 3
    if (dir == 14)
      // ^            ^
      // │            │ Z
      // │ AXES       │
      // └─────>      └─────> Y
      passed &= ( (c_jmin == n_jmax) && (c_kmin == n_kmax) && (c_imin >= n_imin) && (c_imax <= n_imax) && (c_level <= (n_level+2)) && ((c_level+2) >= n_level) ) // case 1
             || ( (c_jmin == n_jmax) && (c_kmin >  n_kmin) && (c_kmax <= n_kmax) && (c_imin >= n_imin) && (c_imax <= n_imax ) && (c_level == (n_level+1)) ) // case 2
             || ( (c_kmin == n_kmax) && (c_jmin >  n_jmin) && (c_jmax <= n_jmax) && (c_imin >= n_imin) && (c_imax <= n_imax ) && (c_level == (n_level+1)) ); // case 3
    if (dir == 15)
      // ^            ^
      // │            │ Z
      // │ AXES       │
      // └─────>      └─────> -Y
      passed &= ( (c_jmax == n_jmin) && (c_kmin == n_kmax) && (c_imin >= n_imin) && (c_imax <= n_imax) && (c_level <= (n_level+2)) && ((c_level+2) >= n_level) ) // case 1
             || ( (c_jmax == n_jmin) && (c_kmin >  n_kmin) && (c_kmax <= n_kmax) && (c_imin >= n_imin) && (c_imax <= n_imax ) && (c_level == (n_level+1)) ) // case 2
             || ( (c_kmin == n_kmax) && (c_jmax <  n_jmax) && (c_jmin >= n_jmin) && (c_imin >= n_imin) && (c_imax <= n_imax ) && (c_level == (n_level+1)) ); // case 3
    if (dir == 16)
      // ^            ^
      // │            │ -Z
      // │ AXES       │
      // └─────>      └─────> Y
      passed &= ( (c_jmin == n_jmax) && (c_kmax == n_kmin) && (c_imin >= n_imin) && (c_imax <= n_imax) && (c_level <= (n_level+2)) && ((c_level+2) >= n_level) ) // case 1
             || ( (c_jmin == n_jmax) && (c_kmax <  n_kmax) && (c_kmin >= n_kmin) && (c_imin >= n_imin) && (c_imax <= n_imax ) && (c_level == (n_level+1)) ) // case 2
             || ( (c_kmax == n_kmin) && (c_jmin >  n_jmin) && (c_jmax <= n_jmax) && (c_imin >= n_imin) && (c_imax <= n_imax ) && (c_level == (n_level+1)) ); // case 3
    if (dir == 17)
      // ^            ^
      // │            │ -Z
      // │ AXES       │
      // └─────>      └─────> -Y
      passed &= ( (c_jmax == n_jmin) && (c_kmax == n_kmin) && (c_imin >= n_imin) && (c_imax <= n_imax) && (c_level <= (n_level+2)) && ((c_level+2) >= n_level) ) // case 1
             || ( (c_jmax == n_jmin) && (c_kmax <  n_kmax) && (c_kmin >= n_kmin) && (c_imin >= n_imin) && (c_imax <= n_imax ) && (c_level == (n_level+1)) ) // case 2
             || ( (c_kmax == n_kmin) && (c_jmax <  n_jmax) && (c_jmin >= n_jmin) && (c_imin >= n_imin) && (c_imax <= n_imax ) && (c_level == (n_level+1)) ); // case 3

    // Directions 18 to 25
    // case 1                   case 2                  case 3                     case 4
    //          ___             ___________                    _______                  _______
    //        /__ /│              N   /__ /│                 /___/__ /│               /___/__ /│
    //       │ C │ │            ___ /│ C │ │               /   N   /│C│              │   │ C │ │
    //    ___│___│/      OR        │ │___│/      OR      /______ /  │/      OR       │___│___│/│
    //  /__ /│                     │ N │                │       │ N │               /_______/  │
    // │ N │ │                     │  /                 │   N   │   │              │       │ N │
    //                          ___│/                   │       │  /               │   N   │   │
    //
    // case 5   /__ /│          case 6     /__ /│            case 7     ____
    //         │ C │ │              ______│ C │ │             ________ /__ /│       ^
    //     ____│___│/      OR      /  N   │___│/      OR     /       /│ C │ │       │  ─┐
    //    /   N   /│             /______ /  │                _____ /  │___│/        │ /   AXES
    //  /_______/  │            │       │   │                     │ N │             └───────>
    // │        │  │            │       │   │                     │   │
    //
    // Difference of levels should be smaller or equal to 2
    if (dir == 18)
      // ^              ^ Y
      // │  ─┐          │  ─┐ Z
      // │ /   AXES     │ /
      // └───────>      └─────> X
      passed &= ( (c_imin == n_imax) && (c_jmin == n_jmax) && (c_kmin == n_kmax) && (c_level <= (n_level+3)) && ((c_level+3) >= n_level) ) // case 1
             || ( (c_imin == n_imax) && (c_jmax <= n_jmax) && (c_jmin >  n_jmin) && (c_kmax <= n_kmax) && (c_kmin > n_kmin) && (c_level == (n_level+1)) ) // case 2
             || ( (c_jmin == n_jmax) && (c_imax <= n_imax) && (c_imin >  n_imin) && (c_kmax <= n_kmax) && (c_kmin > n_kmin) && (c_level == (n_level+1)) ) // case 3
             || ( (c_kmin == n_kmax) && (c_imax <= n_imax) && (c_imin >  n_imin) && (c_jmax <= n_jmax) && (c_jmin > n_jmin) && (c_level == (n_level+1)) ) // case 4
             || ( (c_imax <= n_imax) && (c_imin >  n_imin) && (c_jmin == n_jmax) && (c_kmin == n_kmax) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 5
             || ( (c_jmax <= n_jmax) && (c_jmin >  n_jmin) && (c_imin == n_imax) && (c_kmin == n_kmax) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 6
             || ( (c_kmax <= n_kmax) && (c_kmin >  n_kmin) && (c_imin == n_imax) && (c_jmin == n_jmax) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ); // case 7
    if (dir == 19)
      // ^              ^ Y
      // │  ─┐          │  ─┐ Z
      // │ /   AXES     │ /
      // └───────>      └─────> -X
      passed &= ( (c_imax == n_imin) && (c_jmin == n_jmax) && (c_kmin == n_kmax) && (c_level <= (n_level+3)) && ((c_level+3) >= n_level) ) // case 1
             || ( (c_imax == n_imin) && (c_jmax <= n_jmax) && (c_jmin >  n_jmin) && (c_kmax <= n_kmax) && (c_kmin > n_kmin) && (c_level == (n_level+1)) ) // case 2
             || ( (c_jmin == n_jmax) && (c_imin >= n_imin) && (c_imax <  n_imax) && (c_kmax <= n_kmax) && (c_kmin > n_kmin) && (c_level == (n_level+1)) ) // case 3
             || ( (c_kmin == n_kmax) && (c_imin >= n_imin) && (c_imax <  n_imax) && (c_jmax <= n_jmax) && (c_jmin > n_jmin) && (c_level == (n_level+1)) ) // case 4
             || ( (c_imin >= n_imin) && (c_imax <  n_imax) && (c_jmin == n_jmax) && (c_kmin == n_kmax) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 5
             || ( (c_jmax <= n_jmax) && (c_jmin >  n_jmin) && (c_imax == n_imin) && (c_kmin == n_kmax) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 6
             || ( (c_kmax <= n_kmax) && (c_kmin >  n_kmin) && (c_imax == n_imin) && (c_jmin == n_jmax) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ); // case 7
    if (dir == 20)
      // ^              ^ -Y
      // │  ─┐          │  ─┐ Z
      // │ /   AXES     │ /
      // └───────>      └─────> X
      passed &= ( (c_imin == n_imax) && (c_jmax == n_jmin) && (c_kmin == n_kmax) && (c_level <= (n_level+3)) && ((c_level+3) >= n_level) ) // case 1
             || ( (c_imin == n_imax) && (c_jmin >= n_jmin) && (c_jmax <  n_jmax) && (c_kmax <= n_kmax) && (c_kmin > n_kmin) && (c_level == (n_level+1)) ) // case 2
             || ( (c_jmax == n_jmin) && (c_imax <= n_imax) && (c_imin >  n_imin) && (c_kmax <= n_kmax) && (c_kmin > n_kmin) && (c_level == (n_level+1)) ) // case 3
             || ( (c_kmin == n_kmax) && (c_imax <= n_imax) && (c_imin >  n_imin) && (c_jmin >= n_jmin) && (c_jmax < n_jmax) && (c_level == (n_level+1)) ) // case 4
             || ( (c_imax <= n_imax) && (c_imin >  n_imin) && (c_jmax == n_jmin) && (c_kmin == n_kmax) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 5
             || ( (c_jmin >= n_jmin) && (c_jmax <  n_jmax) && (c_imin == n_imax) && (c_kmin == n_kmax) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 6
             || ( (c_kmax <= n_kmax) && (c_kmin >  n_kmin) && (c_imin == n_imax) && (c_jmax == n_jmin) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ); // case 7
    if (dir == 21)
      // ^              ^ -Y
      // │  ─┐          │  ─┐ Z
      // │ /   AXES     │ /
      // └───────>      └─────> -X
      passed &= ( (c_imax == n_imin) && (c_jmax == n_jmin) && (c_kmin == n_kmax) && (c_level <= (n_level+3)) && ((c_level+3) >= n_level) ) // case 1
             || ( (c_imax == n_imin) && (c_jmin >= n_jmin) && (c_jmax <  n_jmax) && (c_kmax <= n_kmax) && (c_kmin > n_kmin) && (c_level == (n_level+1)) ) // case 2
             || ( (c_jmax == n_jmin) && (c_imin >= n_imin) && (c_imax <  n_imax) && (c_kmax <= n_kmax) && (c_kmin > n_kmin) && (c_level == (n_level+1)) ) // case 3
             || ( (c_kmin == n_kmax) && (c_imin >= n_imin) && (c_imax <  n_imax) && (c_jmin >= n_jmin) && (c_jmax < n_jmax) && (c_level == (n_level+1)) ) // case 4
             || ( (c_imin >= n_imin) && (c_imax <  n_imax) && (c_jmax == n_jmin) && (c_kmin == n_kmax) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 5
             || ( (c_jmin >= n_jmin) && (c_jmax <  n_jmax) && (c_imax == n_imin) && (c_kmin == n_kmax) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 6
             || ( (c_kmax <= n_kmax) && (c_kmin >  n_kmin) && (c_imax == n_imin) && (c_jmax == n_jmin) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ); // case 7
    if (dir == 22)
      // ^              ^ Y
      // │  ─┐          │  ─┐ -Z
      // │ /   AXES     │ /
      // └───────>      └─────> X
      passed &= ( (c_imin == n_imax) && (c_jmin == n_jmax) && (c_kmax == n_kmin) && (c_level <= (n_level+3)) && ((c_level+3) >= n_level) ) // case 1
             || ( (c_imin == n_imax) && (c_jmax <= n_jmax) && (c_jmin >  n_jmin) && (c_kmin >= n_kmin) && (c_kmax < n_kmax) && (c_level == (n_level+1)) ) // case 2
             || ( (c_jmin == n_jmax) && (c_imax <= n_imax) && (c_imin >  n_imin) && (c_kmin >= n_kmin) && (c_kmax < n_kmax) && (c_level == (n_level+1)) ) // case 3
             || ( (c_kmax == n_kmin) && (c_imax <= n_imax) && (c_imin >  n_imin) && (c_jmax <= n_jmax) && (c_jmin > n_jmin) && (c_level == (n_level+1)) ) // case 4
             || ( (c_imax <= n_imax) && (c_imin >  n_imin) && (c_jmin == n_jmax) && (c_kmax == n_kmin) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 5
             || ( (c_jmax <= n_jmax) && (c_jmin >  n_jmin) && (c_imin == n_imax) && (c_kmax == n_kmin) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 6
             || ( (c_kmin >= n_kmin) && (c_kmax <  n_kmax) && (c_imin == n_imax) && (c_jmin == n_jmax) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ); // case 7
    if (dir == 23)
      // ^              ^ Y
      // │  ─┐          │  ─┐ -Z
      // │ /   AXES     │ /
      // └───────>      └─────> -X
      passed &= ( (c_imax == n_imin) && (c_jmin == n_jmax) && (c_kmax == n_kmin) && (c_level <= (n_level+3)) && ((c_level+3) >= n_level) ) // case 1
             || ( (c_imax == n_imin) && (c_jmax <= n_jmax) && (c_jmin >  n_jmin) && (c_kmin >= n_kmin) && (c_kmax < n_kmax) && (c_level == (n_level+1)) ) // case 2
             || ( (c_jmin == n_jmax) && (c_imin >= n_imin) && (c_imax <  n_imax) && (c_kmin >= n_kmin) && (c_kmax < n_kmax) && (c_level == (n_level+1)) ) // case 3
             || ( (c_kmax == n_kmin) && (c_imin >= n_imin) && (c_imax <  n_imax) && (c_jmax <= n_jmax) && (c_jmin > n_jmin) && (c_level == (n_level+1)) ) // case 4
             || ( (c_imin >= n_imin) && (c_imax <  n_imax) && (c_jmin == n_jmax) && (c_kmax == n_kmin) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 5
             || ( (c_jmax <= n_jmax) && (c_jmin >  n_jmin) && (c_imax == n_imin) && (c_kmax == n_kmin) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 6
             || ( (c_kmin >= n_kmin) && (c_kmax <  n_kmax) && (c_imax == n_imin) && (c_jmin == n_jmax) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ); // case 7
    if (dir == 24)
      // ^              ^ -Y
      // │  ─┐          │  ─┐ -Z
      // │ /   AXES     │ /
      // └───────>      └─────> X
      passed &= ( (c_imin == n_imax) && (c_jmax == n_jmin) && (c_kmax == n_kmin) && (c_level <= (n_level+3)) && ((c_level+3) >= n_level) ) // case 1
             || ( (c_imin == n_imax) && (c_jmin >= n_jmin) && (c_jmax <  n_jmax) && (c_kmin >= n_kmin) && (c_kmax < n_kmax) && (c_level == (n_level+1)) ) // case 2
             || ( (c_jmax == n_jmin) && (c_imax <= n_imax) && (c_imin >  n_imin) && (c_kmin >= n_kmin) && (c_kmax < n_kmax) && (c_level == (n_level+1)) ) // case 3
             || ( (c_kmax == n_kmin) && (c_imax <= n_imax) && (c_imin >  n_imin) && (c_jmin >= n_jmin) && (c_jmax < n_jmax) && (c_level == (n_level+1)) ) // case 4
             || ( (c_imax <= n_imax) && (c_imin >  n_imin) && (c_jmax == n_jmin) && (c_kmax == n_kmin) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 5
             || ( (c_jmin >= n_jmin) && (c_jmax <  n_jmax) && (c_imin == n_imax) && (c_kmax == n_kmin) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 6
             || ( (c_kmin >= n_kmin) && (c_kmax <  n_kmax) && (c_imin == n_imax) && (c_jmax == n_jmin) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ); // case 7
    if (dir == 25)
      // ^              ^ -Y
      // │  ─┐          │  ─┐ -Z
      // │ /   AXES     │ /
      // └───────>      └─────> -X
      passed &= ( (c_imax == n_imin) && (c_jmax == n_jmin) && (c_kmax == n_kmin) && (c_level <= (n_level+3)) && ((c_level+3) >= n_level) ) // case 1
             || ( (c_imax == n_imin) && (c_jmin >= n_jmin) && (c_jmax <  n_jmax) && (c_kmin >= n_kmin) && (c_kmax < n_kmax) && (c_level == (n_level+1)) ) // case 2
             || ( (c_jmax == n_jmin) && (c_imin >= n_imin) && (c_imax <  n_imax) && (c_kmin >= n_kmin) && (c_kmax < n_kmax) && (c_level == (n_level+1)) ) // case 3
             || ( (c_kmax == n_kmin) && (c_imin >= n_imin) && (c_imax <  n_imax) && (c_jmin >= n_jmin) && (c_jmax < n_jmax) && (c_level == (n_level+1)) ) // case 4
             || ( (c_imin >= n_imin) && (c_imax <  n_imax) && (c_jmax == n_jmin) && (c_kmax == n_kmin) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 5
             || ( (c_jmin >= n_jmin) && (c_jmax <  n_jmax) && (c_imax == n_imin) && (c_kmax == n_kmin) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ) // case 6
             || ( (c_kmin >= n_kmin) && (c_kmax <  n_kmax) && (c_imax == n_imin) && (c_jmax == n_jmin) && (c_level >= (n_level+1)) && (c_level <= (n_level+2)) ); // case 7
    if (dir >= 18 && dir<=25)
      passed &= ( n_level <= c_level )
             && ( c_level <= (n_level+3) );
  }
 
  return passed;
}

// Neighbors search (3D)
//                     root A       root B
//                     _______      _______
//                   /__ /__ /│   /__ /__ /│
// structure  ->   /__ /__ /│7│ /__ /__ /│7│
//                │ 4 │ 5 │ │/││ 4 │ 5 │ │/│
//                │___│___│/│3││___│___│/│3│
//                │ 0 │ 1 │ │/ │ 0 │ 1 │ │/
//                │___│___│/   │___│___│/
//
// Each root has 8 child cells and we split
// - root A, child 0
// - root A, child 3
// - root A, child 5
// - root A, child 7
// - root B, child 0
// - root B, child 2
// - root B, child 5
// - root B, child 7
TEST_CASE("[core][neighbor] Neighbors search (octree, 1 level)") {
  static constexpr int Nx = 2, Ny = 2, Nz = 2;
  using Cell3D = Cell<Nx,Ny,Nz, core::neighbor::TestCellData<Nx, Ny, Nz>>;
  // Create 4 root cells
  auto A = std::make_shared<Cell3D>(nullptr);
  auto B = std::make_shared<Cell3D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell3D> eA{A}, eB{B};
  eA.setNeighbor(1, B);          // A +x -> B
  eB.setNeighbor(0, A);          // B -x -> A
  std::vector<RootCellEntry<Cell3D>> entries { eA, eB };

  // Create the tree
  unsigned max_level{2};
  Tree<Cell3D> tree(1, max_level);
  tree.createRootCells(entries);

  // Mesh until min level
  tree.meshAtMinLevel();

  // Splitting some cells
  A->getChildCell(0)->split(max_level);
  A->getChildCell(3)->split(max_level);
  A->getChildCell(5)->split(max_level);
  A->getChildCell(7)->split(max_level);
  B->getChildCell(0)->split(max_level);
  B->getChildCell(3)->split(max_level);
  B->getChildCell(4)->split(max_level);
  B->getChildCell(7)->split(max_level);

  // Set root cells data
  A->getCellData().imin = 0    ; A->getCellData().imax = Nx*Nx;
  A->getCellData().jmin = 0    ; A->getCellData().jmax = Ny*Ny;
  A->getCellData().kmin = 0    ; A->getCellData().kmax = Nz*Nz;
  B->getCellData().imin = Nx*Nx; B->getCellData().imax = 2*Nx*Nx;
  B->getCellData().jmin = 0    ; B->getCellData().jmax = Ny*Ny;
  B->getCellData().kmin = 0    ; B->getCellData().kmax = Nz*Nz;
  core::neighbor::initialize_tree_cells_limits(A);
  core::neighbor::initialize_tree_cells_limits(B);

  // Fix bug
  auto c = A->getChildCell(0)->getChildCell(5);

  // Check neighbors in all directions
  tree.applyToOwnedLeaves(
    [](const std::shared_ptr<Cell3D> &cell, const unsigned) mutable {
      CHECK(check_octree_cell<Cell3D>(cell, 2*Nx*Nx, Ny*Ny, Nz*Nz));
    }
  );
}

// Neighbors leaf search (3D)
//                     root A       root B
//                     _______      _______
//                   /__ /__ /│   /__ /__ /│
// structure  ->   /__ /__ /│7│ /__ /__ /│7│
//                │ 4 │ 5 │ │/││ 4 │ 5 │ │/│
//                │___│___│/│3││___│___│/│3│
//                │ 0 │ 1 │ │/ │ 0 │ 1 │ │/
//                │___│___│/   │___│___│/
//
// Each root has 8 child cells and we split
// - root A, child 0
// - root A, child 3
// - root A, child 5 child 5 child 5
// - root A, child 7
// - root A, child 7 child 0
// - root A, child 7 child 2
// - root B, child 0 child 0 child 0
// - root B, child 2
// - root B, child 5
// - root B, child 7
TEST_CASE("[core][neighbor] Neighbors search (octree, deep)") {
  static constexpr int Nx = 2, Ny = 2, Nz = 2;
  using Cell3D = Cell<Nx,Ny,Nz, core::neighbor::TestCellData<Nx, Ny, Nz>>;
  // Create 4 root cells
  auto A = std::make_shared<Cell3D>(nullptr);
  auto B = std::make_shared<Cell3D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell3D> eA{A}, eB{B};
  eA.setNeighbor(1, B);          // A +x -> B
  eB.setNeighbor(0, A);          // B -x -> A
  std::vector<RootCellEntry<Cell3D>> entries { eA, eB };

  // Create the tree
  unsigned max_level{4};
  Tree<Cell3D> tree(1, max_level);
  tree.createRootCells(entries);

  // Mesh until min level
  tree.meshAtMinLevel();

  // Splitting some cells
  A->getChildCell(0)->split(max_level);
  A->getChildCell(3)->split(max_level);
  A->getChildCell(5)->split(max_level);
  A->getChildCell(5)->getChildCell(5)->split(max_level);
  A->getChildCell(5)->getChildCell(5)->getChildCell(5)->split(max_level);
  A->getChildCell(7)->split(max_level);
  A->getChildCell(7)->getChildCell(0)->split(max_level);
  A->getChildCell(7)->getChildCell(2)->split(max_level);
  B->getChildCell(0)->split(max_level);
  B->getChildCell(0)->getChildCell(0)->split(max_level);
  B->getChildCell(0)->getChildCell(0)->getChildCell(0)->split(max_level);
  B->getChildCell(3)->split(max_level);
  //B->getChildCell(4)->split(max_level); // Already split due to mesh conformity constraint
  B->getChildCell(7)->split(max_level);

  // Set root cells data
  A->getCellData().imin = 0          ; A->getCellData().imax = Nx*Nx*Nx*Nx;
  A->getCellData().jmin = 0          ; A->getCellData().jmax = Ny*Ny*Ny*Ny;
  A->getCellData().kmin = 0          ; A->getCellData().kmax = Nz*Nz*Nz*Nz;
  B->getCellData().imin = Nx*Nx*Nx*Nx; B->getCellData().imax = 2*Nx*Nx*Nx*Nx;
  B->getCellData().jmin = 0          ; B->getCellData().jmax = Ny*Ny*Ny*Ny;
  B->getCellData().kmin = 0          ; B->getCellData().kmax = Nz*Nz*Nz*Nz;
  core::neighbor::initialize_tree_cells_limits(A);
  core::neighbor::initialize_tree_cells_limits(B);

  // Fix bug
  auto c = A->getChildCell(0)->getChildCell(5);

  // Check neighbors in all directions
  tree.applyToOwnedLeaves(
    [](const std::shared_ptr<Cell3D> &cell, const unsigned) mutable {
      CHECK(check_octree_cell<Cell3D>(cell, 2*Nx*Nx*Nx*Nx, Ny*Ny*Ny*Ny, Nz*Nz*Nz*Nz));
    }
  );
}
