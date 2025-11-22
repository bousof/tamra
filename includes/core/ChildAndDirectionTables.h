/*
 *
 *  Copyright (c) 2025 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Class for computing and providing directional tables.
 *  Enable fast conversion between face, edge, and corner neighbors directions.
 *  Enable fast access to directional child cells sibling numbers.
 */

#pragma once

#include<array>
#include<numeric>
#include<stdexcept>
#include<string>
#include<tuple>
#include<utility>
#include<vector>

//***********************************************************//
//  PROTOTYPES.                                              //
//***********************************************************//

template<int Nx, int Ny, int Nz>
auto compute_dir_sibling_numbers();

//***********************************************************//
//  MAIN CLASS                                               //
//***********************************************************//

template<int Nx, int Ny, int Nz>
struct ChildAndDirectionTables {
 public:
  static constexpr unsigned number_dimensions = (Nx>0) + (Ny>0) + (Nz>0);
  static constexpr unsigned number_split_dimensions = (Nx>1) + (Ny>1) + (Nz>1);
  static constexpr unsigned number_neighbors = 2 * number_dimensions;
  static constexpr unsigned number_plane_neighbors = number_dimensions==3 ? 18 : number_dimensions==2 ? 8 : 2;
  static constexpr unsigned number_volume_neighbors = number_dimensions==3 ? 26 : number_dimensions==2 ? 8 : 2;
  static constexpr unsigned number_of_directions = number_dimensions==3 ? 26 : number_dimensions==2 ? 8 : 2;
  static constexpr unsigned N1 = Nx>0 ? Nx : Ny>0 ? Ny : Nz;
  static constexpr unsigned N2 = (Nx>0 && Ny>0) ? Ny : number_dimensions>1 ? Nz : 1;
  static constexpr unsigned N3 = number_dimensions==3 ? Nz : 1;
  static constexpr unsigned number_children = N1 * N2 * N3;
  static constexpr unsigned N12 = N1 * N2;
  static constexpr unsigned N13 = N1 * N3;
  static constexpr unsigned N23 = N2 * N3;
  static constexpr unsigned max_number_neighbor_leaf_cells_1d = 2;
  static constexpr unsigned max_number_neighbor_leaf_cells_2d = 4 + 2*(N1+N2);
  static constexpr unsigned max_number_neighbor_leaf_cells_3d = 8 + 4*(N1+N2+N3) + 2*(N1*N2+N1*N3+N2*N3);
  static constexpr unsigned max_number_neighbor_leaf_cells = (number_dimensions==1) ? max_number_neighbor_leaf_cells_1d :
                                                             (number_dimensions==2) ? max_number_neighbor_leaf_cells_2d :
                                                             max_number_neighbor_leaf_cells_3d;
  inline static const std::vector<int> all_directions = (number_dimensions==1) ? std::vector<int>({ 0, 1 }) :
                                                        (number_dimensions==2) ? std::vector<int>({ 0, 1, 2, 3, 4, 5, 6, 7 }) :
                                                        std::vector<int>({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 });

  // Get child cells sibling numbers for each incoming direction
  inline static const std::array<std::vector<unsigned>, number_of_directions> dir_sibling_numbers = compute_dir_sibling_numbers<Nx, Ny, Nz>();

  //***********************************************************//
  //  STATIC METHODS                                           //
  //***********************************************************//
 public:
  // Transform sibling number to (i,j,k) coordinates
  static std::tuple<unsigned, unsigned, unsigned> siblingNumberToCoords(const unsigned sibling_number);
  // Transform (i,j,k) coordinates to sibling number
  static unsigned coordsToSiblingNumber(const unsigned sibling_coord_1, const unsigned sibling_coord_2, const unsigned sibling_coord_3);
  // For a given sibling number, determine if the neighbor cell in a given direction:
  // - shares the same parent cell (true) or belongs to another cell (false)
  // - its sibling number
  static std::pair<bool, unsigned> directNeighborCellInfos(const unsigned sibling_number, const unsigned dir);
  // Convert two direct neighbor directions to a plane direction
  static unsigned directToPlaneDir(const unsigned dir1, const unsigned dir2);
  // Convert a plane direction to a pair of direct neighbor directions
  static std::pair<unsigned, unsigned> planeToDirectDirs(const unsigned dir);
  // Convert a volume direction to a triplet of direct neighbor directions
  static std::tuple<unsigned, unsigned, unsigned> volumeToDirectDirs(const unsigned dir);
};


//***********************************************************//
//  IMPLEMEBNTATIONS                                         //
//***********************************************************//

template<int Nx, int Ny, int Nz>
auto compute_dir_sibling_numbers() {
  using ChildAndDirectionTablesType = ChildAndDirectionTables<Nx, Ny, Nz>;
  static constexpr int N1 = ChildAndDirectionTablesType::N1;
  static constexpr int N2 = ChildAndDirectionTablesType::N2;
  static constexpr int N3 = ChildAndDirectionTablesType::N3;
  static constexpr unsigned number_dimensions = ChildAndDirectionTablesType::number_dimensions;
  static constexpr unsigned number_neighbors = ChildAndDirectionTablesType::number_neighbors;
  static constexpr unsigned number_plane_neighbors = ChildAndDirectionTablesType::number_plane_neighbors;
  static constexpr unsigned number_of_directions = number_dimensions==3 ? 26 : number_dimensions==2 ? 8 : 2;

  std::array<std::vector<unsigned>, number_of_directions> directional_sibling_numbers;
  for (unsigned dir{0}; dir<number_of_directions; ++dir) {
    // Face sibling numbers
    if (dir < number_neighbors) {
      unsigned ni, Nj, Nk;
      switch (dir) {
        case 0: ni = N1-1; Nj = N2; Nk = N3; break;
        case 1: ni = 0;    Nj = N2; Nk = N3; break;
        case 2: ni = N2-1; Nj = N1; Nk = N3; break;
        case 3: ni = 0;    Nj = N1; Nk = N3; break;
        case 4: ni = N3-1; Nj = N1; Nk = N2; break;
        case 5: ni = 0;    Nj = N1; Nk = N2; break;
      }

      std::vector<unsigned> face_sibling_numbers;
      face_sibling_numbers.reserve(Nj * Nk);
      for (unsigned j{0}; j<Nj; ++j)
        for (unsigned k{0}; k<Nk; ++k) {
          if (dir < 2)
            face_sibling_numbers.push_back(ChildAndDirectionTablesType::coordsToSiblingNumber(ni, j, k));
          else if (dir < 4)
            face_sibling_numbers.push_back(ChildAndDirectionTablesType::coordsToSiblingNumber(j, ni, k));
          else
            face_sibling_numbers.push_back(ChildAndDirectionTablesType::coordsToSiblingNumber(j, k, ni));
        }

      directional_sibling_numbers[dir] = face_sibling_numbers;
    }
    // Edge sibling numbers
    else if (dir >= number_neighbors && dir < number_plane_neighbors) {
      unsigned ni, nj, Nk;
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

      std::vector<unsigned> edge_sibling_numbers;
      edge_sibling_numbers.reserve(Nk);
      for (unsigned k{0}; k<Nk; ++k) {
        if (dir-number_neighbors < 4)
          edge_sibling_numbers.push_back(ChildAndDirectionTablesType::coordsToSiblingNumber(ni, nj,  k));
        else if (dir-number_neighbors < 8)
          edge_sibling_numbers.push_back(ChildAndDirectionTablesType::coordsToSiblingNumber(ni,  k, nj));
        else
          edge_sibling_numbers.push_back(ChildAndDirectionTablesType::coordsToSiblingNumber( k, ni, nj));
      }

      directional_sibling_numbers[dir] = edge_sibling_numbers;
    }
    // Corner sibling numbers
    else if (dir >= number_plane_neighbors) {
      switch (dir) {
        case 18: directional_sibling_numbers[dir] = { ChildAndDirectionTablesType::coordsToSiblingNumber(Nx-1, Ny-1, Nz-1) }; break;
        case 19: directional_sibling_numbers[dir] = { ChildAndDirectionTablesType::coordsToSiblingNumber(0   , Ny-1, Nz-1) }; break;
        case 20: directional_sibling_numbers[dir] = { ChildAndDirectionTablesType::coordsToSiblingNumber(Nx-1, 0   , Nz-1) }; break;
        case 21: directional_sibling_numbers[dir] = { ChildAndDirectionTablesType::coordsToSiblingNumber(0   , 0   , Nz-1) }; break;
        case 22: directional_sibling_numbers[dir] = { ChildAndDirectionTablesType::coordsToSiblingNumber(Nx-1, Ny-1, 0   ) }; break;
        case 23: directional_sibling_numbers[dir] = { ChildAndDirectionTablesType::coordsToSiblingNumber(0   , Ny-1, 0   ) }; break;
        case 24: directional_sibling_numbers[dir] = { ChildAndDirectionTablesType::coordsToSiblingNumber(Nx-1, 0   , 0   ) }; break;
        case 25: directional_sibling_numbers[dir] = { ChildAndDirectionTablesType::coordsToSiblingNumber(0   , 0   , 0   ) }; break;
      }
    }
  }

  return directional_sibling_numbers;
}

#include"ChildAndDirectionTables.tpp"
