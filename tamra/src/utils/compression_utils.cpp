#include "../../includes/utils/compression_utils.h"

// Compress an array of unsigned by stacking bits.
// For instance if max_value = 3, value can be 0,1,2, or 3 so it require 3 bits.
// First value is the size of the array then one unsigned (32 bits) can store 16 values.
void compress_unsigned_vector(const std::vector<unsigned> &v, std::vector<unsigned> &result, const unsigned max_value) {
  unsigned bits_for_value = std::ceil(std::log2(max_value+1));
  unsigned bits_per_unsigned = std::numeric_limits<unsigned>::digits;
  unsigned nb_values_per_unsigned = bits_per_unsigned / bits_for_value;

  result.resize((v.size() + nb_values_per_unsigned - 1) / nb_values_per_unsigned + 1); // +1 for storing size
  std::fill(result.begin(), result.end(), 0);
  result[0] = v.size();
  for (unsigned i{0}, j, offset; i<v.size(); ++i) {
    // Position in the compressed array
    j = i / nb_values_per_unsigned;
    // Offset in the compressed array value
    offset = (i % nb_values_per_unsigned) * bits_for_value;
    // Insert value at correct position
    result[j+1] |= v[i] << offset;
  }
}

// Unompress an array of unsigned by stacking bits.
// For instance if max_value = 3, value can be 0,1,2, or 3 so it require 3 bits.
// First value is the size of the array then one unsigned (32 bits) can store 16 values.
void uncompress_unsigned_vector(const std::vector<unsigned> &v, std::vector<unsigned> &result, const unsigned max_value) {
  unsigned bits_for_value = std::ceil(std::log2(max_value+1));
  unsigned bits_per_unsigned = std::numeric_limits<unsigned>::digits;
  unsigned nb_values_per_unsigned = bits_per_unsigned / bits_for_value;
  unsigned mask_value = (1U << bits_for_value) - 1;

  result.resize(v[0]);
  std::fill(result.begin(), result.end(), 0);
  for (unsigned i{0}, j, offset; i<v[0]; ++i) {
    // Position in the compressed array
    j = i / nb_values_per_unsigned;
    // Offset in the compressed array value
    offset = (i % nb_values_per_unsigned) * bits_for_value;
    // Extract value from correct position
    result[i] = static_cast<unsigned>((v[j+1] >> offset) & mask_value);
  }
}
