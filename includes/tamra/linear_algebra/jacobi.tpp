#include <tamra/linear_algebra/jacobi.h>

template <typename RealT, typename AccT>
std::vector<RealT> jacobi_iteration(const Eigen::SparseMatrix<RealT, Eigen::RowMajor> &A_local,
                                   const std::vector<RealT> &b_local,
                                   const std::vector<RealT> &x_local,
                                   const std::unordered_map<int, RealT> &x_lacking,
                                   const unsigned rank, // Default process rank
                                   const unsigned col_offset) { // Number of rows

  // Number of local rows owned by this rank
  const int local_row_count = A_local.rows();
  const int col_max = col_offset + local_row_count;
  std::vector<RealT> x_next(local_row_count);
  for (long i{0}; i<A_local.rows(); ++i) {
    AccT diag{0.}, sum{0.};

    for (typename Eigen::SparseMatrix<RealT, Eigen::RowMajor>::InnerIterator it(A_local, i); it; ++it) {
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
      throw std::runtime_error("Zero diagonal detected in Jacobi iteration.");
    }

    x_next[i] = (b_local[i] - sum) / diag; // Issue here
  }

  return x_next;
}

template <typename RealT, typename AccT>
std::vector<RealT> sparse_jacobi(const Eigen::SparseMatrix<RealT, Eigen::RowMajor> &A,
                                const std::vector<RealT> &b,
                                const std::vector<RealT> &x,
                                const unsigned max_iterations) {

  const int row_count = A.rows();
  std::vector<RealT> x_next = x;
  x_next.resize(row_count);

  for (unsigned iter{0}; iter<max_iterations; ++iter)
    x_next = jacobi_iteration<RealT, AccT>(A, b, x_next);

  return x_next;
}

#ifdef USE_MPI

inline unsigned owner_rank(unsigned col, const std::vector<int> &row_cumsum) {
  auto it = std::upper_bound(row_cumsum.begin(), row_cumsum.end(), col);
  return std::distance(row_cumsum.begin(), it);
}

template <typename RealT, typename AccT>
std::vector<RealT> parallel_sparse_jacobi(const Eigen::SparseMatrix<RealT, Eigen::RowMajor> &A_local,
                                         const std::vector<RealT> &b_local,
                                         const std::vector<RealT> &x_local,
                                         const unsigned max_iterations,
                                         const unsigned rank, const unsigned size) {

  // If only one process run the sequential version
  if (size == 1)
    return sparse_jacobi<RealT, AccT>(A_local, b_local, x_local, max_iterations);

  // Exchange nb of local rows on each process
  const int local_row_count = A_local.rows();
  std::vector<int> row_counts(size);
  MPI_Allgather(&local_row_count, 1, MPI_INT, row_counts.data(), 1, MPI_INT, MPI_COMM_WORLD);

  // Compute offsets to determine process partitions
  std::vector<int> row_cumsum(size);
  cumulative_sum(row_counts, row_cumsum);

  // Compute index of rows to receive from each process
  std::vector<std::set<int>> recv_indexes_sets(size);
  for (long i{0}; i<A_local.rows(); ++i)
    for (typename Eigen::SparseMatrix<RealT, Eigen::RowMajor>::InnerIterator it(A_local, i); it; ++it) {
      unsigned p = owner_rank(it.col(), row_cumsum);
      if (p != rank)
        recv_indexes_sets[p].insert(it.col());
    }

  std::vector<std::vector<int>> recv_indexes_buffers(size);
  std::vector<int> recv_indexes;
  for (unsigned p{0}; p<size; ++p) {
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
	vector_alltoallv<int>(recv_indexes_buffers, send_indexes);

  // Find the number of rows before
  int rows_offset = std::accumulate(row_counts.begin(), row_counts.begin()+rank, 0);

  // Vector for sending values to other procs
  std::vector<std::vector<RealT>> send_values(size);
  std::vector<RealT> recv_values;
  std::unordered_map<int, RealT> x_lacking;
  std::vector<RealT> x_next = x_local;
  x_next.resize(local_row_count);
  for (unsigned iter{0}; iter<max_iterations; ++iter) {
    for (unsigned p{0}; p<size; ++p) {
      send_values[p].resize(send_indexes[p].size());
      for (size_t i{0}; i<send_indexes[p].size(); ++i)
        send_values[p][i] = x_next[send_indexes[p][i] - rows_offset];
    }

    vector_alltoallv<RealT>(send_values, recv_values);

    // Create a vector for storing lacking x values
    for (size_t i{0}; i<recv_indexes.size(); ++i)
      x_lacking[recv_indexes[i]] = recv_values[i];

    x_next = jacobi_iteration<RealT, AccT>(A_local, b_local, x_next, x_lacking, rank, rows_offset);
  }

  return x_next;
}

#else

template <typename RealT, typename AccT>
std::vector<RealT> parallel_sparse_jacobi(const Eigen::SparseMatrix<RealT, Eigen::RowMajor> &A_local,
                                        const std::vector<RealT> &b_local,
                                        const std::vector<RealT> &x_local,
                                        const unsigned max_iterations,
                                        const unsigned, const unsigned) {

  // If MPI is not used, run the sequential version
  return sparse_jacobi<RealT, AccT>(A_local, b_local, x_local, max_iterations);;
}

#endif // USE_MPI
