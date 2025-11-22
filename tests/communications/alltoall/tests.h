#include <iostream>
#include <parallel/allreduce.h>
#include <parallel/alltoall.h>
#include <parallel/alltoallv.h>
#include <parallel/wrapper.h>
#include <UnitTestRegistry.h>
#include <vector>

bool testIntAlltoall(const unsigned rank, const unsigned size);
bool testVectorIntAlltoall(const unsigned rank, const unsigned size);
bool testVectorDoubleAllToAllFlat(const unsigned rank, const unsigned size);
bool testVectorDoubleAllToAllSplit(const unsigned rank, const unsigned size);
bool testVectorDataAllToAllFixed(const unsigned rank, const unsigned size);
bool testVectorDataAllToAllDynamic(const unsigned rank, const unsigned size);

void registerCommunicationsAllToAllTests() {
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  UnitTestRegistry::registerParallelTest("Int AllToAll", [=]() { return testIntAlltoall(rank, size); }, "communications/alltoall");
  UnitTestRegistry::registerParallelTest("Vector Int AllToAll", [=]() { return testVectorIntAlltoall(rank, size); }, "communications/alltoall");
  UnitTestRegistry::registerParallelTest("Vector Double AllToAll (flat)", [=]() { return testVectorDoubleAllToAllFlat(rank, size); }, "communications/alltoall");
  UnitTestRegistry::registerParallelTest("Vector Double AllToAll (split)", [=]() { return testVectorDoubleAllToAllSplit(rank, size); }, "communications/alltoall");
  UnitTestRegistry::registerParallelTest("Vector Data AllToAll (fixed size)", [=]() { return testVectorDataAllToAllFixed(rank, size); }, "communications/alltoall");
  UnitTestRegistry::registerParallelTest("Vector Data AllToAll (dynamic size)", [=]() { return testVectorDataAllToAllDynamic(rank, size); }, "communications/alltoall");
}

// Int AllToAll
bool testIntAlltoall(const unsigned rank, const unsigned size) {
  // Send rank to all others
  std::vector<int> send_buffer(size, rank),
                   recv_buffer(size, 0);
  intAlltoall(send_buffer, recv_buffer);

  // Should receive ranks from all others
  bool passed = true;
  for (int i{0}; i<(int)size; ++i)
    passed &= recv_buffer.at(i) == i;

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}

// Vector Int AllToAll
bool testVectorIntAlltoall(const unsigned rank, const unsigned size) {
  std::vector<std::vector<unsigned>> send_buffers(size);
  for (unsigned p{0}; p<size; ++p)
    send_buffers.at(p).push_back(rank * 10 + p);

  std::vector<unsigned> recv_buffer;
  vectorUnsignedAlltoallv(send_buffers, recv_buffer);

  bool passed = recv_buffer.size() == size;
  for (unsigned i{0}; i<size && passed; ++i)
    passed &= (recv_buffer.at(i) % 10 == rank);

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}

// Vector Double AllToAll (flat)
bool testVectorDoubleAllToAllFlat(const unsigned rank, const unsigned size) {
  std::vector<std::vector<double>> send_buffers(size);
  for (unsigned p{0}; p<size; ++p)
    send_buffers.at(p).push_back(static_cast<double>(rank + p) + 0.5);

  std::vector<double> recv_buffer;
  vectorDoubleAlltoallv(send_buffers, recv_buffer);

  bool passed = recv_buffer.size() == size;
  for (unsigned i{0}; i<size && passed; ++i)
    passed &= std::fabs(recv_buffer.at(i) - (i + rank + 0.5)) < 1e-10;

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}

// Vector Double AllToAll (split)
bool testVectorDoubleAllToAllSplit(const unsigned rank, const unsigned size) {
  std::vector<std::vector<double>> send_buffers(size);
  for (unsigned p{0}; p<size; ++p)
    send_buffers.at(p).assign(size-1-p, static_cast<double>(rank + p) + 0.5);

  std::vector<std::vector<double>> recv_buffers;
  vectorDoubleAlltoallv(send_buffers, recv_buffers);

  bool passed = recv_buffers.size() == size;
  for (unsigned p{0}; p<size && passed; ++p) {
    passed &= recv_buffers.at(p).size() == (size-1-rank);
    if (rank != size-1)
      passed &= std::fabs(recv_buffers.at(p).at(0) - (p + rank + 0.5)) < 1e-10;
  }

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}

// Mock ParallelData class for testing vectorDataAllToAll
struct MockData : public ParallelData {
  unsigned id = 0;
  double x = 0.0;
  std::vector<double> data;

  void fromVectorOfData(const std::vector<double> &buffer) override {
    id = static_cast<unsigned>(buffer.at(0));
    x = buffer.at(1);
    data.assign(buffer.begin() + 2, buffer.end());
  }

  std::vector<double> toVectorOfData() const override {
    std::vector<double> buffer;
    buffer.reserve(getDataSize());
    buffer.push_back(static_cast<double>(id));
    buffer.push_back(x);
    buffer.insert(buffer.end(), data.begin(), data.end());
    return buffer;
  }

  unsigned getDataSize() const override {
    return 2 + data.size();
  }
};

// Vector Data AllToAll (fixed size)
bool testVectorDataAllToAllFixed(const unsigned rank, const unsigned size) {
  std::vector<std::vector<std::unique_ptr<ParallelData>>> send_buffers(size);
  for (unsigned p{0}; p<size; ++p) {
    auto data = std::make_unique<MockData>();
    data->id = rank;
    data->x = static_cast<double>(p);
    send_buffers.at(p).push_back(std::move(data));
  }

  std::vector<std::unique_ptr<ParallelData>> recv_buffer;
  vectorDataAlltoallv(send_buffers, recv_buffer, []() {
    return std::make_unique<MockData>();
  });

  bool passed = recv_buffer.size() == size;
  for (unsigned i{0}; i<size && passed; ++i) {
    auto* data = dynamic_cast<MockData*>(recv_buffer.at(i).get());
    passed &= data && (data->id == i);
    passed &= std::fabs(data->x - rank) < 1e-10;
  }

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}

// Vector Data AllToAll (dynamic size)
bool testVectorDataAllToAllDynamic(const unsigned rank, const unsigned size) {
  std::vector<std::vector<std::unique_ptr<ParallelData>>> send_buffers(size);
  for (unsigned p{0}; p<size; ++p) {
    auto data = std::make_unique<MockData>();
    data->id = rank;
    data->x = static_cast<double>(p);
    data->data.assign(p+1, 3.14);
    send_buffers.at(p).push_back(std::move(data));
  }

  std::vector<std::unique_ptr<ParallelData>> recv_buffer;
  vectorDataAlltoallv(send_buffers, recv_buffer, []() {
    return std::make_unique<MockData>();
  });

  bool passed = recv_buffer.size() == size;
  for (unsigned i{0}; i<size && passed; ++i) {
    auto* data = dynamic_cast<MockData*>(recv_buffer.at(i).get());
    passed &= data && (data->id == i);
    passed &= std::fabs(data->x - rank) < 1e-10;
    passed &= data->data.size() == rank+1;
    passed &= std::fabs(data->data.at(rank) - 3.14) < 1e-10;
  }

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}
