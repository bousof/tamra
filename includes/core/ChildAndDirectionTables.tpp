#include"ChildAndDirectionTables.h"

//***********************************************************//
//  STATIC METHODS                                           //
//***********************************************************//

// Transform sibling number to (i,j,k) coordinates
template<int Nx, int Ny, int Nz>
std::tuple<unsigned, unsigned, unsigned> ChildAndDirectionTables<Nx, Ny, Nz>::siblingNumberToCoords(const unsigned sibling_number) {
  unsigned sibling_coord_1 = sibling_number % N1,
            sibling_coord_2 = (sibling_number / N1) % N2,
            sibling_coord_3 = (sibling_number / (N12)) % N3;

  return std::make_tuple(sibling_coord_1, sibling_coord_2, sibling_coord_3);
}

// Transform (i,j,k) coordinates to sibling number
template<int Nx, int Ny, int Nz>
unsigned ChildAndDirectionTables<Nx, Ny, Nz>::coordsToSiblingNumber(const unsigned sibling_coord_1, const unsigned sibling_coord_2, const unsigned sibling_coord_3) {
  return sibling_coord_1 + sibling_coord_2 * N1 + sibling_coord_3 * N12;
}

// For a given sibling number, determine if the neighbor cell in a given direction:
// - shares the same parent cell (true) or belongs to another cell (false)
// - its sibling number
template<int Nx, int Ny, int Nz>
std::pair<bool, unsigned> ChildAndDirectionTables<Nx, Ny, Nz>::directNeighborCellInfos(const unsigned sibling_number, const unsigned dir) {
  unsigned sibling_coord_1, sibling_coord_2, sibling_coord_3;
  std::tie(sibling_coord_1, sibling_coord_2, sibling_coord_3) = siblingNumberToCoords(sibling_number);

  // Reference the coordinate to modify to access neighbor
  unsigned &sibling_coord = dir<2 ? sibling_coord_1 : dir < 4 ? sibling_coord_2 : sibling_coord_3;

  // Determine if the neighbor sibling number and its sibling number
  bool neighbor_is_sibling = false;
  const unsigned Ndir = dir<2 ? N1 : dir < 4 ? N2 : N3;
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

// Convert two direct neighbor directions to a plane direction
template<int Nx, int Ny, int Nz>
unsigned ChildAndDirectionTables<Nx, Ny, Nz>::directToPlaneDir(const unsigned dir1, const unsigned dir2) {
  if (dir1 >= number_neighbors || dir2 >= number_neighbors)
    throw std::runtime_error("Invalid direct neighbor direction in Oct::directToPlaneDir()");
  unsigned dir = number_neighbors;
  if constexpr (number_dimensions == 2) {
    if (dir1==0 && dir2==2) dir = 0;
    if (dir1==1 && dir2==2) dir = 1;
    if (dir1==0 && dir2==3) dir = 2;
    if (dir1==1 && dir2==3) dir = 3;
  } else if constexpr (number_dimensions == 3) {
    if (dir1==0 && dir2==2) dir = 0;
    if (dir1==1 && dir2==2) dir = 1;
    if (dir1==0 && dir2==3) dir = 2;
    if (dir1==1 && dir2==3) dir = 3;
    if (dir1==0 && dir2==4) dir = 4;
    if (dir1==1 && dir2==4) dir = 5;
    if (dir1==0 && dir2==5) dir = 6;
    if (dir1==1 && dir2==5) dir = 7;
    if (dir1==2 && dir2==4) dir = 8;
    if (dir1==3 && dir2==4) dir = 9;
    if (dir1==2 && dir2==5) dir = 10;
    if (dir1==3 && dir2==5) dir = 11;
  }
  return number_neighbors + dir;
}

// Convert a plane direction to a pair of direct neighbor directions
template<int Nx, int Ny, int Nz>
std::pair<unsigned, unsigned> ChildAndDirectionTables<Nx, Ny, Nz>::planeToDirectDirs(const unsigned dir) {
  if (dir < number_neighbors || dir >= number_plane_neighbors)
    throw std::runtime_error("Invalid plane neighbor direction in Oct::planeToDirectDirs()");
  const unsigned plane_dir = dir - number_neighbors;
  unsigned dir1, dir2;
  if (plane_dir < 4 && Nx>0 && Ny>0) {
    dir1 = (plane_dir  ) % 2 ? 1 : 0,
    dir2 = (plane_dir/2) % 2 ? 3 : 2;
  } else if (plane_dir < 8 && Nx>0) {
    dir1 = (plane_dir  ) % 2 ? 1 : 0,
    dir2 = (plane_dir/2) % 2 ? 5 : 4;
  } else {
    dir1 = (plane_dir  ) % 2 ? 3 : 2,
    dir2 = (plane_dir/2) % 2 ? 5 : 4;
  }
  return std::make_pair(dir1, dir2);
}

// Convert a volume direction to a triplet of direct neighbor directions
template<int Nx, int Ny, int Nz>
std::tuple<unsigned, unsigned, unsigned> ChildAndDirectionTables<Nx, Ny, Nz>::volumeToDirectDirs(const unsigned dir) {
  if (dir < number_plane_neighbors || dir >= number_volume_neighbors)
    throw std::runtime_error("Invalid volume neighbor direction in Oct::volumeToDirectDirs()");

  const unsigned volume_dir = dir - number_plane_neighbors;
  unsigned dir1 = (volume_dir  ) % 2 ? 1 : 0,
           dir2 = (volume_dir/2) % 2 ? 3 : 2,
           dir3 = (volume_dir/4) % 2 ? 5 : 4;
  return std::make_tuple(dir1, dir2, dir3);
}
