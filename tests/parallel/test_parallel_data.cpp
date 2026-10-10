#include <doctest.h>

#include <memory>
#include <utility>
#include <vector>

#include <tamra/parallel/allgatherv.h>
#include <tamra/parallel/alltoallv.h>
#include <tamra/parallel/wrapper.h>

namespace {

class TestParallelData : public ParallelData {
 public:
  std::vector<double> values;

  explicit TestParallelData(std::vector<double> values = {}) : values(std::move(values)) {}
  unsigned getDataSize() const override { return static_cast<unsigned>(values.size()); }
  std::vector<double> toVectorOfData() const override { return values; }
  void fromVectorOfData(const std::vector<double> &buffer) override { values = buffer; }
};

void check_payloads(const std::vector<std::unique_ptr<ParallelData>> &received,
                    const std::vector<std::vector<double>> &expected) {
  CHECK(received.size() == expected.size());
  for (std::size_t i{0}; i < received.size() && i < expected.size(); ++i) {
    CAPTURE(i);
    const auto *data = dynamic_cast<const TestParallelData *>(received[i].get());
    CHECK(data != nullptr);
    if (data)
      CHECK(data->values == expected[i]);
  }
}

} // namespace

TEST_CASE("[parallel][data] Allgather reconstructs variable and empty payloads") {
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  std::vector<std::unique_ptr<ParallelData>> received;
  received.push_back(std::make_unique<TestParallelData>(std::vector<double>{99.}));
  for (unsigned mode{0}; mode < 3; ++mode) {
    CAPTURE(mode);
    std::vector<std::unique_ptr<ParallelData>> sent;
    std::vector<std::vector<double>> expected;
    for (unsigned p{0}; p < size; ++p) {
      const unsigned count = mode == 0 ? (p + 2) % 4 : mode == 1 ? 1 : 0;
      for (unsigned i{0}; i < count; ++i) {
        const std::vector<double> payload(mode == 0 ? i : 0, p + 0.25 * i);
        expected.push_back(payload);
        if (p == rank)
          sent.push_back(std::make_unique<TestParallelData>(payload));
      }
    }
    unsigned factory_calls = 0;
    vector_data_allgatherv(sent, received, size, [&factory_calls]() {
      ++factory_calls;
      return std::make_unique<TestParallelData>();
    });
    sent.clear(); // Received objects must own their data independently.
    CHECK(factory_calls == expected.size());
    check_payloads(received, expected);
  }
}

TEST_CASE("[parallel][data] Alltoall reconstructs sparse variable payloads") {
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  std::vector<std::unique_ptr<ParallelData>> received;
  received.push_back(std::make_unique<TestParallelData>(std::vector<double>{99.}));
  for (unsigned mode{0}; mode < 3; ++mode) {
    CAPTURE(mode);
    std::vector<std::vector<std::unique_ptr<ParallelData>>> sent(size);
    std::vector<std::vector<double>> expected;
    for (unsigned p{0}; p < size; ++p) {
      const unsigned send_count = mode == 0 ? (rank + 2 * p + 2) % 4 : mode == 1 ? 1 : 0;
      const unsigned recv_count = mode == 0 ? (p + 2 * rank + 2) % 4 : mode == 1 ? 1 : 0;
      for (unsigned i{0}; i < send_count; ++i)
        sent[p].push_back(std::make_unique<TestParallelData>(
          std::vector<double>(mode == 0 ? i : 0, 100. * rank + 10. * p + 0.25 * i)));
      for (unsigned i{0}; i < recv_count; ++i)
        expected.emplace_back(mode == 0 ? i : 0, 100. * p + 10. * rank + 0.25 * i);
    }
    unsigned factory_calls = 0;
    vector_data_alltoallv(sent, received, [&factory_calls]() {
      ++factory_calls;
      return std::make_unique<TestParallelData>();
    });
    sent.clear();
    CHECK(factory_calls == expected.size());
    check_payloads(received, expected);
  }
}
