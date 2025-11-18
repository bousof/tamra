#include "./BalanceManager.h"
//#include "../../utils/display_vector.h"

//***********************************************************//
//  CONSTRUCTORS, DESTRUCTOR AND INITIALIZATION              //
//***********************************************************//

// Constructor
template<typename CellType, typename TreeIteratorType>
BalanceManager<CellType, TreeIteratorType>::BalanceManager(const int min_level, const int max_level, const int rank, const int size)
: min_level(min_level),
  max_level(max_level),
  rank(rank),
  size(size) {}

// Destructor
template<typename CellType, typename TreeIteratorType>
BalanceManager<CellType, TreeIteratorType>::~BalanceManager() {};


//***********************************************************//
//  METHODS                                                  //
//***********************************************************//
// Determine if load balancing is needed
template<typename CellType, typename TreeIteratorType>
std::pair<bool, std::vector<double>> BalanceManager<CellType, TreeIteratorType>::isLoadBalancingNeeded(const std::vector<std::shared_ptr<CellType>> &root_cells, const double max_pct_unbalance) const {
  // Root process that handles decision making
  const int root = 0;

  // Sum up the loads in every root cell
  double load = 0;
  for (const auto &root_cell : root_cells)
    load += computeLoad(root_cell);

  // Share load between all processors
  std::vector<double> loads;
  doubleGather(load, loads, root, rank, size);

  // Compute the max and mean load and compare load unbalance with max_pct_unbalance
  bool balancing_needed;
  if (rank == 0) {
    double total_load = std::accumulate(loads.begin(), loads.end(), 0.0);
    double mean_load = total_load / static_cast<double>(loads.size());
    double max_load = *std::max_element(loads.begin(), loads.end());

    if ((max_load - mean_load) / mean_load > max_pct_unbalance)
      balancing_needed = true;
  }

  // Broadcast decision to all processes
  boolBcast(balancing_needed, root);

  return std::make_pair(balancing_needed, loads);
}

// Parallel meshing at min level
template<typename CellType, typename TreeIteratorType>
void BalanceManager<CellType, TreeIteratorType>::loadBalance(const std::vector<std::shared_ptr<CellType>> &root_cells, TreeIteratorType &iterator, const double max_pct_unbalance, ExtrapolationFunctionType extrapolation_function) const {
  // If only one process, nothing to do
  if (size == 1)
    return;

  // Root process that handles decision making
  const int root = 0;

  bool balancing_needed;
  std::vector<double> loads;
  std::tie(balancing_needed, loads) = isLoadBalancingNeeded(root_cells, max_pct_unbalance);
  if (!balancing_needed)
    return;

  // Compute the cumulative sums of loads and share between process
  std::vector<double> cumulative_loads, target_cumulative_loads;
  if (rank == root) {
    double sum_loads = cumulative_sum(loads, cumulative_loads, true);
    cumulative_loads.push_back(sum_loads);
    target_cumulative_loads = linspace(0., sum_loads, size+1);
  }

	// Share the cumulative loads and target ones in every processor
	{
    std::vector<double> cumulative_loads_vectors;
    if (rank == root)
      cumulative_loads_vectors = concatenate(cumulative_loads, target_cumulative_loads);
    vectorDoubleBcast(cumulative_loads_vectors, root, rank, 2*(size+1));
    if (rank != root) {
      cumulative_loads.insert(cumulative_loads.end(), cumulative_loads_vectors.begin(), cumulative_loads_vectors.begin()+size+1);
      target_cumulative_loads.insert(target_cumulative_loads.end(), cumulative_loads_vectors.begin()+size+1, cumulative_loads_vectors.end());
    }
  }

  //std::cout << "P_" << rank << ": cumulative loads";
  //displayVector(std::cout, cumulative_loads) << std::endl;
  //std::cout << "P_" << rank << ": target cumulative loads";
  //displayVector(std::cout, target_cumulative_loads) << std::endl;

  // Determine the cells to send to each process
  std::vector<std::vector<std::shared_ptr<CellType>>> cells_to_send = cellsToExchange(cumulative_loads, target_cumulative_loads, iterator);

  //std::cout << "P_" << rank << ": nb of cells to send ";
  //for (int p{0}; p<size; ++p) {
  //  std::cout << cells_to_send[p].size() << " ";
  //}
  //std::cout << std::endl;

  // Exchange and create load balancing cells
	exchangeAndCreateCells(cells_to_send, iterator, extrapolation_function);

  // Backpropagate flags from leaf to all cells
  for (const auto &root_cell : root_cells)
    backPropagateFlags(root_cell);
}

// Determine the local load for this process
template<typename CellType, typename TreeIteratorType>
double BalanceManager<CellType, TreeIteratorType>::computeLoad(const std::shared_ptr<CellType> &cell) const {
  if (!cell->belongToThisProc())
    return 0.;
  if (cell->isLeaf())
    return cell->getLoad();

  // Add up the computing loads of the child cells
  double load = 0.;
  for (const auto &child : cell->getChildCells())
    load += computeLoad(child);

  return load;
}

// Determine the cells to send to each process
template<typename CellType, typename TreeIteratorType>
std::vector<std::vector<std::shared_ptr<CellType>>> BalanceManager<CellType, TreeIteratorType>::cellsToExchange(const std::vector<double> &cumulative_loads, const std::vector<double> &target_cumulative_loads, TreeIteratorType &iterator) const {
  // If process has no cells we return empty arrays
  std::vector<std::vector<std::shared_ptr<CellType>>> cells_to_send(size);
  if (!iterator.toOwnedBegin())
    return cells_to_send;

  // Determining the cell ids located at transitions
  if (rank > 0
   && target_cumulative_loads[rank] > cumulative_loads[rank]) { // Prevent zero-load exchanges at balanced boundaries
    // Main loop for determination of transition cell ids
    bool loop = true;
    double previous_load = cumulative_loads[rank], current_cell_load;
    int target_proc = 0;
    do {
      // Computation load of the current cell
      current_cell_load = iterator.getCell()->getLoad();

      // Either increment target proc if cells is not in its partition or add cell to send to it
      if (target_cumulative_loads[target_proc+1] < (previous_load+current_cell_load)) {
        ++target_proc;
        continue;
      } else {
        cells_to_send[target_proc].push_back(iterator.getCell());
      }

      // Add cell load to counter
      previous_load += current_cell_load;
      loop = iterator.ownedNext();
    } while ((target_proc<rank) && loop);
  }
  if (rank < size-1
   && target_cumulative_loads[rank+1] < cumulative_loads[rank+1]) { // Prevent zero-load exchanges at balanced boundaries
    // Main loop for determination of cell ids
    iterator.toOwnedEnd();
    bool loop = true;
    double next_load = cumulative_loads[rank+1], current_cell_load;
    int target_proc = size-1;
    do {
      // Computation load of the current cell
      current_cell_load = iterator.getCell()->getLoad();

      // Either decrement target proc if cells is not in its partition or add cell to send to it
      if (target_cumulative_loads[target_proc] > (next_load-current_cell_load)) {
        --target_proc;
        continue;
      } else
        cells_to_send[target_proc].push_back(iterator.getCell());

      // Remove cell load to counter
      next_load -= current_cell_load;
      loop = iterator.ownedPrev();
    } while ((target_proc>rank) && loop);
  }

  // Reverse array for ranks > rank because they are starting from end
  for (int p{rank+1}; p<size; ++p)
    std::reverse(cells_to_send[p].begin(), cells_to_send[p].end());

  { // Unset sent leaf flags to belong to other proc
    for (int p{0}; p<size; ++p)
      for (auto &cell : cells_to_send[p])
        cell->setToOtherProcRecurs();
  }

  return cells_to_send;
}

// Exchange cells structure and data
template<typename CellType, typename TreeIteratorType>
void BalanceManager<CellType, TreeIteratorType>::exchangeAndCreateCells(const std::vector<std::vector<std::shared_ptr<CellType>>> &cells_to_send, TreeIteratorType &iterator, ExtrapolationFunctionType extrapolation_function) const {
  // For the first cell we sent the cell ID to be able to locate it.
  // For the lacking ines only the level is set to avoid redundant information.
  std::vector<std::vector<unsigned>> cells_structure_to_send(size);
  {
    std::vector<unsigned> first_cell_id, cell_levels;
    for (unsigned p{0}; p<size; ++p)
      if (cells_to_send[p].size()) {
        // First cell ID
        first_cell_id = iterator.getCellId(cells_to_send[p][0]);
        // Other cells levels
        cell_levels.resize(cells_to_send[p].size() - 1);
        for (unsigned i{1}; i<cells_to_send[p].size(); ++i)
          cell_levels[i-1] = cells_to_send[p][i]->getLevel();
        // Insert the other cells levels
        compressCellStructure(first_cell_id, cell_levels, cells_structure_to_send[p]);
      }
  }

  // Gather cell data in a vector for sharing between process
  std::vector<std::vector<std::unique_ptr<ParallelData>>> all_cell_data(size);
  {
    for (unsigned p{0}; p<size; ++p) {
      all_cell_data[p].reserve(cells_to_send[p].size());
      for (unsigned i{0}; i<cells_to_send[p].size(); ++i)
        all_cell_data[p].push_back(std::make_unique<typename CellType::CellDataType>(cells_to_send[p][i]->getCellData()));
    }
  }

  //std::cout << "P_" << rank << ": send structure ";
  //displayVector(std::cout, cells_structure_to_send) << std::endl;

  // Exchange tree structure
  std::vector<std::vector<unsigned>> cells_structure_recv(size);
  vectorUnsignedAlltoallv(cells_structure_to_send, cells_structure_recv);

  // Exchange cell data
  std::vector<std::unique_ptr<ParallelData>> all_cell_data_recv;
  vectorDataAlltoallv(all_cell_data, all_cell_data_recv, []() {
    return std::make_unique<typename CellType::CellDataType>();
  });

  //std::cout << "P_" << rank << ": recv structure";
  //displayVector(std::cout, cells_structure_recv) << std::endl;

  { // Create received cells and set leaf flags to this proc
    std::vector<unsigned> first_cell_id, cell_levels;
    const unsigned cell_id_size = iterator.getCellIdManager().getCellIdSize();
    unsigned cell_counter = 0;
    for (unsigned p{0}; p<size; ++p)
      if (cells_structure_recv[p].size()) {
        // Insert the first cell ID
        uncompressCellStructure(cells_structure_recv[p], first_cell_id, cell_levels, cell_id_size);
        // Create the first cell and assign it to this proc
        iterator.toCellId(first_cell_id, true, extrapolation_function);
        iterator.getCell()->setToThisProcRecurs();
        // Set first cell data
        iterator.getCell()->setCellData(std::unique_ptr<typename CellType::CellDataType>(
          static_cast<typename CellType::CellDataType*>(all_cell_data_recv[cell_counter++].release())
        ));
        if (!iterator.getCell()->isLeaf())
          // Call extrapolation function on non-leaf cells
          iterator.getCell()->extrapolateRecursively(extrapolation_function);
        // Insert the other cells levels
        for (const unsigned &cell_level : cell_levels) {
          iterator.next(cell_level);
          while (iterator.getCell()->getLevel()<cell_level) {
            if (iterator.getCell()->isLeaf())
              iterator.getCell()->split(max_level, extrapolation_function);
            iterator.toLeaf(cell_level);
          }
          iterator.getCell()->setToThisProcRecurs();

          // Set cell data
          iterator.getCell()->setCellData(std::unique_ptr<typename CellType::CellDataType>(
            static_cast<typename CellType::CellDataType*>(all_cell_data_recv[cell_counter++].release())
          ));

          if (!iterator.getCell()->isLeaf())
            // Call extrapolation function on non-leaf cells
            iterator.getCell()->extrapolateRecursively(extrapolation_function);
        }
      }
  }
}

// Compress the structure of cells (1 cell ID + other cell levels)
template<typename CellType, typename TreeIteratorType>
void BalanceManager<CellType, TreeIteratorType>::compressCellStructure(const std::vector<unsigned> &first_cell_id, const std::vector<unsigned> &cell_levels, std::vector<unsigned> &cell_structure) const {
  cell_structure.resize(first_cell_id.size() + cell_levels.size());
  // Insert the first cell ID
  std::copy(first_cell_id.begin(), first_cell_id.end(), cell_structure.begin());
  // Insert the other cells levels
  std::copy(cell_levels.begin(), cell_levels.end(), cell_structure.begin()+first_cell_id.size());
}

// Uncompress the structure of cells (1 cell ID + other cell levels)
template<typename CellType, typename TreeIteratorType>
void BalanceManager<CellType, TreeIteratorType>::uncompressCellStructure(const std::vector<unsigned> &cell_structure, std::vector<unsigned> &first_cell_id, std::vector<unsigned> &cell_levels, const unsigned cell_id_size) const {
  // Extract the first cell ID
  first_cell_id.resize(cell_id_size);
  std::copy(cell_structure.begin(), cell_structure.begin() + cell_id_size, first_cell_id.begin());
  // Extract the other cells levels
  cell_levels.resize(cell_structure.size()-cell_id_size);
  std::copy(cell_structure.begin() + cell_id_size, cell_structure.end(), cell_levels.begin());
}

// Set a parent to belong to this proc if any of its child do else set to other proc
template<typename CellType, typename TreeIteratorType>
bool BalanceManager<CellType, TreeIteratorType>::backPropagateFlags(const std::shared_ptr<CellType> &cell) const {
  if (cell->isLeaf())
    return cell->belongToThisProc();

  bool to_this_proc = false;
  for (const auto &child : cell->getChildCells())
    if (backPropagateFlags(child))
      to_this_proc = true;

  if (to_this_proc)
    cell->setToThisProc();
  else
    cell->setToOtherProc();

  return to_this_proc;
}
