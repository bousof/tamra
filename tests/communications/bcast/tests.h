#ifndef USE_MPI
#  define USE_MPI
#endif
#include <mpi.h>

#include <iostream>
#include <parallel/allreduce.h>
#include <parallel/bcast.h>
#include <testing/UnitTestRegistry.h>
#include <vector>

bool testVectorUnsignedBCastCount(int rank, int size);
bool testVectorUnsignedBCastNoCount(int rank, int size);
bool testMatrixUnsignedBCastCount(int rank, int size);
bool testMatrixUnsignedBCastNoCount(int rank, int size);

void registerCommunicationsBCastTests() {
  int rank; int size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  UnitTestRegistry::label = "P_" + std::to_string(rank) + ": ";
  UnitTestRegistry::registerParallelTest("Broadcast vector of unsigned (count known)", [=]() { return testVectorUnsignedBCastCount(rank, size); }, "communications/bcast");
  UnitTestRegistry::registerParallelTest("Broadcast vector of unsigned (count unknown)", [=]() { return testVectorUnsignedBCastNoCount(rank, size); }, "communications/bcast");
  UnitTestRegistry::registerParallelTest("Broadcast matrix of unsigned (count known)", [=]() { return testMatrixUnsignedBCastCount(rank, size); }, "communications/bcast");
  UnitTestRegistry::registerParallelTest("Broadcast matrix of unsigned (count unknown)", [=]() { return testMatrixUnsignedBCastNoCount(rank, size); }, "communications/bcast");
}

// Broadcast vector of unsigned (count known)
bool testVectorUnsignedBCastCount(int rank, int size) {
  int root = 0, count = 3;

  std::vector<unsigned> buffer;
  if (rank == root)
    buffer = {2, 7, 8};
  vectorUnsignedBCast(buffer, root, rank, count);

  bool passed = buffer == std::vector<unsigned>({2, 7, 8});

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}

// Broadcast vector of unsigned (count unknown)
bool testVectorUnsignedBCastNoCount(int rank, int size) {
  int root = 0;

  std::vector<unsigned> buffer;
  if (rank == root)
    buffer = {2, 7, 8};
  vectorUnsignedBCast(buffer, root, rank);

  bool passed = buffer == std::vector<unsigned>({2, 7, 8});

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}

// Broadcast matrix of unsigned (count known)
bool testMatrixUnsignedBCastCount(int rank, int size) {
  int root = 0, rowCount = 4, colCount = 3;

  std::vector< std::vector<unsigned> > buffer;
  if (rank == root)
    buffer = { {1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12} };
  matrixUnsignedBCast(buffer, root, rank, rowCount, colCount);

  bool passed = buffer[0] == std::vector<unsigned>({1, 2, 3});
  passed &= buffer[1] == std::vector<unsigned>({4, 5, 6});
  passed &= buffer[2] == std::vector<unsigned>({7, 8, 9});
  passed &= buffer[3] == std::vector<unsigned>({10, 11, 12});

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}

// Broadcast matrix of unsigned (count unknown)
bool testMatrixUnsignedBCastNoCount(int rank, int size) {
  int root = 0;

  std::vector< std::vector<unsigned> > buffer;
  if (rank == root)
    buffer = { {1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12} };
  matrixUnsignedBCast(buffer, root, rank);

  bool passed = buffer[0] == std::vector<unsigned>({1, 2, 3});
  passed &= buffer[1] == std::vector<unsigned>({4, 5, 6});
  passed &= buffer[2] == std::vector<unsigned>({7, 8, 9});
  passed &= buffer[3] == std::vector<unsigned>({10, 11, 12});

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}
