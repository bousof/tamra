#include "../../includes/utils/array_utils.h"

int cumulative_sum(const std::vector<int>& counts, std::vector<int>& displacements, bool startAtZero) {
  displacements.resize(counts.size());
  int sum = 0;
  for (unsigned i = 0; i < counts.size(); ++i) {
    if (startAtZero) 
      displacements[i] = sum;
    sum += counts[i];
    if (!startAtZero) 
      displacements[i] = sum;
  }
  return sum;
}
