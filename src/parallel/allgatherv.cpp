#include <tamra/parallel/allgatherv.h>

//-----------------------------------------------------------//
//  IMPLEMENTATIONS                                          //
//-----------------------------------------------------------//

void vector_data_allgatherv(const std::vector<std::unique_ptr<ParallelData>> &send_buffer, std::vector<std::unique_ptr<ParallelData>> &recv_buffer, const unsigned size, const ParallelDataFactory create_data) {
  // Prepare data sizes for sharing between processors
	std::vector<unsigned> send_counts(send_buffer.size());
	for (size_t i{0}; i<send_buffer.size(); ++i)
		send_counts[i] = send_buffer[i]->getDataSize();

	// Communication of data sizes between all processors
	std::vector<unsigned> recv_counts;
	vector_allgatherv<unsigned>(send_counts, recv_counts, size);

	// Sending buffer, data must be put to a vector of doubles
  int tot_size_data = std::accumulate(send_counts.begin(), send_counts.end(), 0);
	std::vector<double> send_data_buffer;
  send_data_buffer.reserve(tot_size_data);
	for (const auto &data_ptr : send_buffer) {
    const auto data = data_ptr->toVectorOfData();
    send_data_buffer.insert(send_data_buffer.end(), data.begin(), data.end());
  }

	// Communication of cells data between all processors
	std::vector<double> recv_data_buffer;
	vector_allgatherv<double>(send_data_buffer, recv_data_buffer, size);

	// Sending buffer, data come as a vector of doubles and must be transformed to a vector of ParallelData
	recv_buffer.resize(recv_counts.size());
	std::size_t data_counter = 0;
	for (size_t i{0}; i<recv_counts.size(); ++i) {
		std::vector<double> subdata(recv_data_buffer.begin() + data_counter,
                                recv_data_buffer.begin() + data_counter + recv_counts[i]);
		data_counter += recv_counts[i];

    recv_buffer[i] = create_data();
		recv_buffer[i]->fromVectorOfData(subdata);
	}
}
