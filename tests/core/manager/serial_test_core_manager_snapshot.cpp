#include <doctest.h>

#include <memory>
#include <vector>

#include <core/Cell.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>
#include <core/manager/SnapshotManager.h>

namespace core::manager::snapshot {
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
  // Conversion as vector of double for data communication
  void fromVectorOfData(const std::vector<double> &buffer) override {
    std::memcpy(&imin, &buffer[0], sizeof(double));
    std::memcpy(&imax, &buffer[1], sizeof(double));
    std::memcpy(&jmin, &buffer[2], sizeof(double));
    std::memcpy(&jmax, &buffer[3], sizeof(double));
    if constexpr (Nz > 0) {
      std::memcpy(&kmin, &buffer[4], sizeof(double));
      std::memcpy(&kmax, &buffer[5], sizeof(double));
    }
  }
  std::vector<double> toVectorOfData() const override {
    std::vector<double> data(getDataSize());
    std::memcpy(&data[0], &imin, sizeof(double));
    std::memcpy(&data[1], &imax, sizeof(double));
    std::memcpy(&data[2], &jmin, sizeof(double));
    std::memcpy(&data[3], &jmax, sizeof(double));
    if constexpr (Nz > 0) {
      std::memcpy(&data[4], &kmin, sizeof(double));
      std::memcpy(&data[5], &kmax, sizeof(double));
    }
    return data;
  }
  unsigned getDataSize() const override {
    if constexpr (Nz > 0)
      return 6;
    else
      return 4;
  }
  // Dump the cell data to an output stream
  void dump(std::ostream& os, const bool binary) const override {
    if (!binary) {
      os << " " << imin << " " << imax;
      os << " " << jmin << " " << jmax;
      if constexpr (Nz > 0)
        os << " " << kmin << " " << kmax;
    } else {
      os.write(reinterpret_cast<const char*>(&imin), sizeof(unsigned));
      os.write(reinterpret_cast<const char*>(&imax), sizeof(unsigned));
      os.write(reinterpret_cast<const char*>(&jmin), sizeof(unsigned));
      os.write(reinterpret_cast<const char*>(&jmax), sizeof(unsigned));
      if constexpr (Nz > 0) {
        os.write(reinterpret_cast<const char*>(&kmin), sizeof(unsigned));
        os.write(reinterpret_cast<const char*>(&kmax), sizeof(unsigned));
      }
    }
  }
  // Restore the cell data from an input stream
  void restore(std::istream& is, const bool binary) override {
    if (!binary) {
      is >> imin >> imax;
      is >> jmin >> jmax;
      if constexpr (Nz > 0)
        is >> kmin >> kmax;
    } else {
      is.read(reinterpret_cast<char*>(&imin), sizeof(unsigned));
      is.read(reinterpret_cast<char*>(&imax), sizeof(unsigned));
      is.read(reinterpret_cast<char*>(&jmin), sizeof(unsigned));
      is.read(reinterpret_cast<char*>(&jmax), sizeof(unsigned));
      if constexpr (Nz > 0) {
        is.read(reinterpret_cast<char*>(&kmin), sizeof(unsigned));
        is.read(reinterpret_cast<char*>(&kmax), sizeof(unsigned));
      }
    }
  }
  bool isEqual(const TestCellData &cell_data) const {
    bool equal = true;
    equal &= (imin == cell_data.imin);
    equal &= (imax == cell_data.imax);
    equal &= (jmin == cell_data.jmin);
    equal &= (jmax == cell_data.jmax);
    if constexpr (Nz > 0) {
      equal &= (kmin == cell_data.kmin);
      equal &= (kmax == cell_data.kmax);
    }
    return equal;
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

template<typename CellType>
void check_cells_data(std::shared_ptr<CellType> cell, std::shared_ptr<CellType> restored_cell) {
  // Verify the coordinates
  CHECK(cell->getCellData().isEqual(restored_cell->getCellData()));
  CHECK(cell->isLeaf() == restored_cell->isLeaf()); // Both split or both leaves

  if (cell->isLeaf())
    return;

  const auto &cell_data = cell->getCellData();
  for (unsigned n{0}; n<CellType::number_children; ++n) {
    auto &child_cell_data = cell->getChildCell(n)->getCellData();
    cell_data.extrapolateToChild(child_cell_data, n);
    check_cells_data<CellType>(cell->getChildCell(n), restored_cell->getChildCell(n));
  }
}

void printSnapshotDebug(const std::string& snapshot) {
  const bool binary = snapshot.find("BINARY 1") != std::string::npos;

  constexpr const char* tag = "CELL_DATA";
  const auto pos = snapshot.find(tag);
  if (pos == std::string::npos) {
    std::cout << snapshot << std::endl;
    return;
  }

  const auto payload_pos = snapshot.find('\n', pos);
  if (payload_pos == std::string::npos) {
    std::cout << snapshot << std::endl;
    return;
  }

  // Print everything up to and including CELL_DATA line
  std::cout.write(snapshot.data(), payload_pos + 1);
  if (!binary) {
    std::cout << "\n[TEXT PAYLOAD]\n";
    std::cout.write(
      snapshot.data() + payload_pos + 1,
      snapshot.size() - payload_pos - 1
    );

    std::cout << std::endl;
    return;
  }

  std::cout << "\n[BINARY PAYLOAD]\n";
  const unsigned char* data = reinterpret_cast<const unsigned char*>(snapshot.data());

  for (std::size_t i = payload_pos + 1; i < snapshot.size(); ++i) {
    std::cout
      << std::hex
      << std::setw(2)
      << std::setfill('0')
      << static_cast<unsigned>(data[i])
      << ' ';

    if ((i - payload_pos - 1) % 16 == 15)
      std::cout << '\n';
  }

  std::cout << std::dec << std::endl;
}
}

// Snapshot of a binary tree with 1 root cell and 2 leaf cells
//                │   A   │
// structure  ->  └───┴───┘
TEST_CASE("[core][manager][snapshot] Snapshot of a binary tree with 1 root cell and 2 leaf cells") {
  using Cell1D = Cell<2>;
  using BinaryTree = Tree<Cell1D>;

  // Create root cell
  auto A = std::make_shared<Cell1D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell1D> eA{A};
  std::vector<RootCellEntry<Cell1D>> entries { eA };

  // Construction of the tree
  unsigned min_level{1}, max_level{2};
  BinaryTree tree(min_level, max_level);
  tree.createRootCells(entries);

  // Count number of leaf cells
  int number_leaf_cells = A->countLeaves();
  CHECK(number_leaf_cells == 2);

  // Create a snapshot manager
  SnapshotManager<BinaryTree> snapshot_manager(tree.getMinLevel(), tree.getMaxLevel(), 0, 1);
  // Dump the tree to a string representation
  std::string snapshot_string = snapshot_manager.dumpMetaAndTreeToString(tree);
  // Restore the tree from the snapshot string
  SnapshotManager<BinaryTree> restore_manager(tree.getMinLevel(), tree.getMaxLevel(), 0, 1);
  BinaryTree restored_tree = restore_manager.readMetaAndRestoreFromString(snapshot_string);

  CHECK(tree.getRootCells().size() == restored_tree.getRootCells().size());
  CHECK(tree.countOwnedLeaves() == restored_tree.countOwnedLeaves());
}

// Snapshot of a quadtree with 1 root cell and 10 leaf cells
//                ┌───┬───┬───────┐
//                │   │   │       │
//                ├───┼───┤       │
//                │   │   │       │
// structure  ->  ├───┴───┼───┬───┤
//                │       │   │   │
//                │       ├───┼───┤
//                │       │   │   │
//                └───────┴───┴───┘
// Number of leaf cells should be 10
TEST_CASE("[core][manager][snapshot] Snapshot of a quadtree with 1 root cell and 10 leaf cells") {
  using Cell2D = Cell<2,2>;
  using QuadTree = Tree<Cell2D>;

  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector<RootCellEntry<Cell2D>> entries { eA };

  // Construction of the tree
  unsigned min_level{1}, max_level{2};
  QuadTree tree(min_level, max_level);
  tree.createRootCells(entries);

  //Splitting some cells
  A->getChildCell(1)->split(max_level);
  A->getChildCell(2)->split(max_level);

  // Count number of leaf cells
  int number_leaf_cells = tree.countOwnedLeaves();
  CHECK(number_leaf_cells == 10);

  // Create a snapshot manager
  SnapshotManager<QuadTree> snapshot_manager(tree.getMinLevel(), tree.getMaxLevel(), 0, 1);
  // Dump the tree to a string representation
  std::string snapshot_string = snapshot_manager.dumpMetaAndTreeToString(tree);
  // Restore the tree from the snapshot string
  SnapshotManager<QuadTree> restore_manager(tree.getMinLevel(), tree.getMaxLevel(), 0, 1);
  QuadTree restored_tree = restore_manager.readMetaAndRestoreFromString(snapshot_string);

  CHECK(number_leaf_cells == restored_tree.countOwnedLeaves());
  auto A_restored = restored_tree.getRootCells()[0];
  CHECK( A_restored->getChildCell(0)->isLeaf());
  CHECK(!A_restored->getChildCell(1)->isLeaf());
  CHECK(!A_restored->getChildCell(2)->isLeaf());
  CHECK( A_restored->getChildCell(3)->isLeaf());
}

// Restore a snapshot from a different tree iterator type
//                ┌─┬─┬───┬───────┐
//                ├─┼─┤   │       │
//                ├─┴─┼───┤       │
//                │   │   │       │
// structure  ->  ├───┴───┼───┬───┤
//                │       │   │   │
//                │       ├───┼─┬─┤
//                │       │   ├─┼─┤
//                └───────┴───┴─┴─┘
// Number of leaf cells should be 16
TEST_CASE("[core][manager][snapshot] Restore a snapshot from a different tree iterator type") {
  using Cell2D = Cell<2,2>;
  using ExportQuadTree = Tree<Cell2D, MortonIterator<Cell2D>>;
  using ImportQuadTree = Tree<Cell2D, HilbertIterator<Cell2D>>;

  // Create root cell
  auto A = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A};
  std::vector<RootCellEntry<Cell2D>> entries { eA };

  // Construction of the tree
  unsigned min_level{1}, max_level{3};
  ExportQuadTree tree(min_level, max_level);
  tree.createRootCells(entries);

  //Splitting some cells
  A->getChildCell(1)->split(max_level);
  A->getChildCell(1)->getChildCell(1)->split(max_level);
  A->getChildCell(2)->split(max_level);
  A->getChildCell(2)->getChildCell(2)->split(max_level);

  // Count number of leaf cells
  int number_leaf_cells = tree.countOwnedLeaves();
  CHECK(number_leaf_cells == 16);

  // Create a snapshot manager
  SnapshotManager<ExportQuadTree> snapshot_manager(tree.getMinLevel(), tree.getMaxLevel(), 0, 1);
  // Dump the tree to a string representation
  std::string snapshot_string = snapshot_manager.dumpMetaAndTreeToString(tree);
  // Restore the tree from the snapshot string
  SnapshotManager<ImportQuadTree> restore_manager(tree.getMinLevel(), tree.getMaxLevel(), 0, 1);
  ImportQuadTree restored_tree = restore_manager.readMetaAndRestoreFromString(snapshot_string);

  CHECK(number_leaf_cells == restored_tree.countOwnedLeaves());
  auto A_restored = restored_tree.getRootCells()[0];
  CHECK( A_restored->getChildCell(0)->isLeaf());
  CHECK(!A_restored->getChildCell(1)->isLeaf());
  CHECK(!A_restored->getChildCell(2)->isLeaf());
  CHECK( A_restored->getChildCell(3)->isLeaf());
  CHECK(!A_restored->getChildCell(1)->getChildCell(1)->isLeaf());
  CHECK( A_restored->getChildCell(1)->getChildCell(0)->isLeaf());
  CHECK(!A_restored->getChildCell(2)->getChildCell(2)->isLeaf());
  CHECK( A_restored->getChildCell(2)->getChildCell(0)->isLeaf());
}

// Snapshot of a quadtree with 4 root cells
//                ┌───┬───┬───┬───┐
//                │   │   │   │   │
//                ├── C ──┼── D ──┤
//                │   │   │   │   │
// structure  ->  ├───┼───┼───┼───┤
//                │   │   │   │   │
//                ├── A ──┼── B ──┤
//                │   │   │   │   │
//                └───┴───┴───┴───┘
TEST_CASE("[core][manager][snapshot] Snapshot of a quadtree with 4 root cells") {
  using Cell2D = Cell<2,2>;
  using QuadTree = Tree<Cell2D>;

  // Create 4 root cells
  auto A = std::make_shared<Cell2D>(nullptr);
  auto B = std::make_shared<Cell2D>(nullptr);
  auto C = std::make_shared<Cell2D>(nullptr);
  auto D = std::make_shared<Cell2D>(nullptr);

  // Create root cell entries
  RootCellEntry<Cell2D> eA{A}, eB{B}, eC{C}, eD{D};
  eA.setNeighbor(1, B);   // A +x -> B
  eA.setNeighbor(3, C);   // A +y -> C
  eB.setNeighbor(0, A);   // B -x -> A
  eB.setNeighbor(3, D);   // B +y -> D
  eC.setNeighbor(1, D);   // C +x -> D
  eC.setNeighbor(2, A);   // C -y -> A
  eD.setNeighbor(0, C);   // D -x -> C
  eD.setNeighbor(2, B);   // D -y -> B
  std::vector<RootCellEntry<Cell2D>> entries { eA, eB, eC, eD };

  QuadTree tree;
  tree.createRootCells(entries);

  // Create a snapshot manager
  SnapshotManager<QuadTree> snapshot_manager(tree.getMinLevel(), tree.getMaxLevel(), 0, 1);
  // Dump the tree to a string representation
  std::string snapshot_string = snapshot_manager.dumpMetaAndTreeToString(tree);
  // Restore the tree from the snapshot string
  SnapshotManager<QuadTree> restore_manager(tree.getMinLevel(), tree.getMaxLevel(), 0, 1);
  QuadTree restored_tree = restore_manager.readMetaAndRestoreFromString(snapshot_string);

  CHECK(16 == restored_tree.countOwnedLeaves());
  auto A_restored = restored_tree.getRootCells()[0],
       B_restored = restored_tree.getRootCells()[1],
       C_restored = restored_tree.getRootCells()[2],
       D_restored = restored_tree.getRootCells()[3];
  CHECK(A_restored->getNeighborCell(1) == B_restored);
  CHECK(A_restored->getNeighborCell(3) == C_restored);
  CHECK(B_restored->getNeighborCell(0) == A_restored);
  CHECK(B_restored->getNeighborCell(3) == D_restored);
  CHECK(C_restored->getNeighborCell(1) == D_restored);
  CHECK(C_restored->getNeighborCell(2) == A_restored);
  CHECK(D_restored->getNeighborCell(0) == C_restored);
  CHECK(D_restored->getNeighborCell(2) == B_restored);
}

// Snapshot of a quadtree with cell data
//                 root A   root B
//                ┌───┬─┬─┐┌─┬─┬───┐
//                │   ├─┼─┤├─┼─┤   │
// structure  ->  ├─┬─┼─┼─┤├─┴─┼─┬─┤
//                ├─┼─┼─┼─┤│   ├─┼─┤
//                └─┴─┴─┴─┘└───┴─┴─┘
// The cell data is composed of the positions i, j, k
TEST_CASE("[core][manager][snapshot] Snapshot of a quadtree with cell data") {
  static constexpr int Nx = 2, Ny = 2, Nz = 0;
  using Cell2D = Cell<Nx,Ny,Nz, core::manager::snapshot::TestCellData<Nx, Ny, Nz>>;
  using QuadTree = Tree<Cell2D>;
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
  QuadTree tree(1, max_level);
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
  core::manager::snapshot::initialize_tree_cells_limits(A);
  core::manager::snapshot::initialize_tree_cells_limits(B);

  // Create a snapshot manager
  SnapshotManager<QuadTree> snapshot_manager(tree.getMinLevel(), tree.getMaxLevel(), 0, 1, true); // Switch false <=> true here
  // Dump the tree to a string representation
  std::string snapshot_string = snapshot_manager.dumpMetaAndTreeToString(tree);
  std::cout << "Snapshot string:\n";
  core::manager::snapshot::printSnapshotDebug(snapshot_string);
  // Restore the tree from the snapshot string
  SnapshotManager<QuadTree> restore_manager(tree.getMinLevel(), tree.getMaxLevel(), 0, 1);
  QuadTree restored_tree = restore_manager.readMetaAndRestoreFromString(snapshot_string);

  CHECK(23 == restored_tree.countOwnedLeaves());
  auto A_restored = restored_tree.getRootCells()[0],
       B_restored = restored_tree.getRootCells()[1];

  core::manager::snapshot::check_cells_data(A, A_restored);
  core::manager::snapshot::check_cells_data(B, B_restored);
}
