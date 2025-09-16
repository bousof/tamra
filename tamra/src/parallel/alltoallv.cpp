#include "../../includes/parallel/alltoallv.h"

std::vector<int> vectorUnsignedAlltoallv(const std::vector<std::vector<unsigned>> &send_buffers, std::vector<unsigned> &recv_buffer, const int size) {
	return vectorAlltoallv(send_buffers, recv_buffer, size, MPI_UNSIGNED);
}

std::vector<int> vectorIntAlltoallv(const std::vector<std::vector<int>> &send_buffers, std::vector<int> &recv_buffer, const int size) {
	return vectorAlltoallv(send_buffers, recv_buffer, size, MPI_INT);
}

std::vector<int> vectorDoubleAlltoallv(const std::vector<std::vector<double>> &send_buffers, std::vector<double> &recv_buffer, const int size) {
	return vectorAlltoallv(send_buffers, recv_buffer, size, MPI_DOUBLE);
}

void vectorUnsignedAlltoallv(const std::vector<std::vector<unsigned>> &send_buffers, std::vector<std::vector<unsigned>> &recv_buffers, const int size) {
	vectorAlltoallv(send_buffers, recv_buffers, size, MPI_UNSIGNED);
}

void vectorIntAlltoallv(const std::vector<std::vector<int>> &send_buffers, std::vector<std::vector<int>> &recv_buffers, const int size) {
	vectorAlltoallv(send_buffers, recv_buffers, size, MPI_INT);
}

void vectorDoubleAlltoallv(const std::vector<std::vector<double>> &send_buffers, std::vector<std::vector<double>> &recv_buffers, const int size) {
	vectorAlltoallv(send_buffers, recv_buffers, size, MPI_DOUBLE);
}

void matrixUnsignedAlltoallv(const std::vector<std::vector<std::vector<unsigned>>> &send_buffers, std::vector<std::vector<unsigned>> &recv_buffers, const int size, const unsigned colCount) {
  matrixAlltoallv(send_buffers, recv_buffers, size, MPI_UNSIGNED, colCount);
}

void matrixUnsignedAlltoallv(const std::vector<std::vector<std::vector<unsigned>>> &send_buffers, std::vector<std::vector<std::vector<unsigned>>> &recv_buffers, const int size, const unsigned colCount) {
  matrixAlltoallv(send_buffers, recv_buffers, size, MPI_UNSIGNED, colCount);
}

void vectorDataAlltoallv(const std::vector<std::vector<std::unique_ptr<ParallelData>>> &send_buffers, std::vector<std::unique_ptr<ParallelData>> &recv_buffer, const int size, const ParallelDataFactory createData) {
  // Prepare data sizes for sharing between processors
	std::vector<std::vector<unsigned>> send_counts(size);
	for (int p{0}; p<size; ++p) {
		send_counts[p].resize(send_buffers[p].size());
		for (unsigned i{0}; i<send_buffers[p].size(); ++i)
			send_counts[p][i] = send_buffers[p][i]->getDataSize();
	}

	// Communication of data sizes between all processors
	std::vector<unsigned> recv_counts;
	vectorUnsignedAlltoallv(send_counts, recv_counts, size);

	// Sending buffer, data must be put to a vector of vector of doubles
	std::vector<std::vector<double>> send_data_buffers(size);
	for (int p{0}; p<size; ++p) {
    int tot_size_data_p = std::accumulate(send_counts[p].begin(), send_counts[p].end(), 0);
		send_data_buffers[p].reserve(tot_size_data_p);

    for (const auto &data_ptr : send_buffers[p]) {
      const auto data = data_ptr->toVectorOfData();
      send_data_buffers[p].insert(send_data_buffers[p].end(), data.begin(), data.end());
    }
	}

	// Communication of cells data between all processors
	std::vector<double> recv_data_buffer;
	vectorDoubleAlltoallv(send_data_buffers, recv_data_buffer, size);

	// Sending buffer, data come as a vector of doubles and must be transformed to a vector of ParallelData
	recv_buffer.resize(recv_counts.size());
	int data_counter = 0;
	for (int i{0}; i<recv_counts.size(); ++i) {
		std::vector<double> subdata(&recv_data_buffer[data_counter], &recv_data_buffer[data_counter+recv_counts[i]]);
		data_counter += recv_counts[i];

    recv_buffer[i] = createData();
		recv_buffer[i]->fromVectorOfData(subdata);
	}
}
