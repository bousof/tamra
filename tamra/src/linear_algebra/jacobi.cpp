#include "../../includes/linear_algebra/jacobi.h"

std::vector<double> jacobiIteration(const Eigen::SparseMatrix<double, Eigen::RowMajor> &A_local,
                                    const std::vector<double> &b_local,
                                    const std::vector<double> &x_local,
                                    const std::unordered_map<int, double> &x_lacking,
                                    const int rank, // Default process rank
                                    const int col_offset) { // Number of rows

  // Number of local rows owned by this rank
  const int nb_local_rows = A_local.rows();
  const int col_max = col_offset + nb_local_rows;
  std::vector<double> x_next(nb_local_rows);
  for (int i{0}; i<A_local.rows(); ++i) {
    double diag = 0.0, sum = 0.0;

    for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator it(A_local, i); it; ++it) {
      if (it.col() == i+col_offset) // Modified
        diag = it.value();
      else if ((col_offset<=it.col()) && (it.col()<col_max))
        //sum += it.value() * x_local.at(it.col()); // OLD VERSION
        sum += it.value() * x_local[it.col()-col_offset];
      else
        sum += it.value() * x_lacking.at(it.col());
    }

    if (diag == 0.0) {
      std::cerr << "Zero diagonal detected at row " << i << " on rank " << rank << std::endl;
      std::terminate();  // or throw
    }

    x_next[i] = (b_local[i] - sum) / diag; // Issue here
  }

  return x_next;
}

std::vector<double> sparseJacobi(const Eigen::SparseMatrix<double, Eigen::RowMajor> &A,
                                 const std::vector<double> &b,
                                 const std::vector<double> &x,
                                 const int max_iterations) {

  const int nb_rows = A.rows();
  std::vector<double> x_next = x;
  x_next.resize(nb_rows);

  for (int iter{0}; iter<max_iterations; ++iter)
    x_next = jacobiIteration(A, b, x_next);

  return x_next;
}

#ifdef USE_MPI

inline int owner_rank(int col, const std::vector<int> &row_cumsum) {
  auto it = std::upper_bound(row_cumsum.begin(), row_cumsum.end(), col);
  return std::distance(row_cumsum.begin(), it);
}

std::vector<double> parallelSparseJacobi(const Eigen::SparseMatrix<double, Eigen::RowMajor> &A_local,
                                         const std::vector<double> &b_local,
                                         const std::vector<double> &x_local,
                                         const int max_iterations,
                                         const int rank, const int size) {

  // If only one process run the sequential version
  if (size == 1)
    return sparseJacobi(A_local, b_local, x_local, max_iterations);

  // Exchange nb of local rows on each process
  const int nb_local_rows = A_local.rows();
  std::vector<int> nb_rows(size);
  MPI_Allgather(&nb_local_rows, 1, MPI_INT, nb_rows.data(), 1, MPI_INT, MPI_COMM_WORLD);

  // Compute offsets to determine process partitions
  std::vector<int> row_cumsum(size);
  cumulative_sum(nb_rows, row_cumsum);

  // Compute index of rows to receive from each process
  std::vector<std::set<int>> recv_indexes_sets(size);
  for (int i{0}; i<A_local.rows(); ++i)
    for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator it(A_local, i); it; ++it) {
      int p = owner_rank(it.col(), row_cumsum);
      if (p != rank)
        recv_indexes_sets[p].insert(it.col());
    }

  std::vector<std::vector<int>> recv_indexes_buffers(size);
  std::vector<int> recv_indexes;
  for (int p{0}; p<size; ++p) {
    recv_indexes_buffers[p].assign(
      recv_indexes_sets[p].begin(),
      recv_indexes_sets[p].end()
    );
    recv_indexes.insert(
      recv_indexes.end(),
      recv_indexes_sets[p].begin(),
      recv_indexes_sets[p].end()
    );
  }

  // Share index of rows to receive from each process
  // and get index of rows to send to each process
  std::vector<std::vector<int>> send_indexes;
	vectorIntAlltoallv(recv_indexes_buffers, send_indexes, size);

  // Find the number of rows before
  int rows_offset = std::accumulate(nb_rows.begin(), nb_rows.begin()+rank, 0);

  // Vector for sending values to other procs
  std::vector<std::vector<double>> send_values(size);
  std::vector<double> recv_values;
  std::unordered_map<int, double> x_lacking;
  std::vector<double> x_next = x_local;
  x_next.resize(nb_local_rows);
  for (int iter{0}; iter<max_iterations; ++iter) {
    for (int p{0}; p<size; ++p) {
      send_values[p].resize(send_indexes[p].size());
      for (int i{0}; i<send_indexes[p].size(); ++i)
        send_values[p][i] = x_next[send_indexes[p][i] - rows_offset];
    }

    vectorDoubleAlltoallv(send_values, recv_values, size);

    // Create a vector for storing lacking x values
    for (int i{0}; i<recv_indexes.size(); ++i)
      x_lacking[recv_indexes[i]] = recv_values[i];

      x_next = jacobiIteration(A_local, b_local, x_next, x_lacking, rank, rows_offset);
  }

  return x_next;
}

#endif
 