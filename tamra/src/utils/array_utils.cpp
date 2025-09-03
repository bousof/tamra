#include "../../includes/utils/array_utils.h"

std::vector<double> linspace(const double min, const double max, const unsigned count) {
  std::vector<double> result(count);
  result[0] = min;
  for (unsigned i = 1; i < count-1; ++i)
    result[i] = min + (i*(max-min))/(count-1);

  if (count > 1)
    result[count-1] = max;
  return result;
}

bool all(const std::vector<bool> &vector) {
  return std::all_of(vector.begin(), vector.end(), [](bool value) { return value; });
}

bool any(const std::vector<bool> &vector) {
  return std::any_of(vector.begin(), vector.end(), [](bool value) { return value; });
}
