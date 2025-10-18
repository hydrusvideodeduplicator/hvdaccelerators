// ================================================================
// Copyright (c) Meta Platforms, Inc. and affiliates.
// ================================================================

#ifndef VPDQHASHTYPE_H
#define VPDQHASHTYPE_H

#include <pdq/cpp/common/pdqhashtypes.h>

#include <sstream>
#include <string>
#include <vector>

namespace facebook {
namespace vpdq {
namespace hashing {

inline std::vector<std::string> split(const std::string& s, char delimiter) {
  std::vector<std::string> tokens;
  std::stringstream ss(s);
  std::string item;

  while (std::getline(ss, item, delimiter)) {
    tokens.push_back(item);
  }

  return tokens;
}

struct vpdqFeature {
  facebook::pdq::hashing::Hash256 pdqHash;
  int frameNumber;
  int quality;

  std::string get_hash() { return pdqHash.format(); }

  int get_frame_number() { return frameNumber; }

  int get_quality() { return quality; }

  /// @brief Convert a serialized vpdqFeature string into a vpdqFeature.
  ///
  /// @param serialized The serialized vpdqFeature string.
  ///
  /// @throws std::invalid_argument if the string is not correctly serialized.
  static vpdqFeature from_str(std::string const& serialized) {
    auto const parts = split(serialized, ',');
    if (parts.size() != 3U) {
      throw std::invalid_argument(
          "Failed to create a vpdqFeature from string. Wrong number of comma delimited parts.");
    }

    auto const& pdq_hex_str = parts[0];
    auto const& qual_str = parts[1];
    auto const& frame_num_str = parts[2];

    int quality;
    try {
      quality = std::stoi(qual_str);
    } catch (const std::exception& e) {
      throw std::invalid_argument(
          "Failed to create a vpdqFeature from string. Quality was not an integer.");
    }

    int frameNumber;
    try {
      frameNumber = std::stoi(frame_num_str);
    } catch (const std::exception& e) {
      throw std::invalid_argument(
          "Failed to create a vpdqFeature from string. Frame number was not an integer.");
    }

    vpdqFeature feature{
        facebook::pdq::hashing::Hash256::fromHexString(pdq_hex_str),
        frameNumber,
        quality,
    };

    if (!feature.is_valid()) {
      throw std::invalid_argument(
          "Failed to create a vpdqFeature from string. String has invalid parts.");
    }
    return feature;
  }

  bool is_valid() const {
    // No need to check pdqHash, because pdqHash objects are always in a valid
    // state.
    if ((quality < 0) || (quality > 100)) {
      return false;
    }

    if (frameNumber < 0) {
      return false;
    }

    return true;
  }

  std::string to_string() const {
    return pdqHash.toHexString() + "," + std::to_string(quality) + "," +
        std::to_string(frameNumber);
  }
};

} // namespace hashing
} // namespace vpdq
} // namespace facebook

#endif // VPDQHASHTYPE_H
