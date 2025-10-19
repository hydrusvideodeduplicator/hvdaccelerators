// ================================================================
// Copyright (c) Meta Platforms, Inc. and affiliates.
// ================================================================

#ifndef VPDQHASHTYPE_H
#define VPDQHASHTYPE_H

#include <pdq/cpp/common/pdqhashtypes.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace facebook {
namespace vpdq {
namespace hashing {

struct vpdqFeature {
  facebook::pdq::hashing::Hash256 pdqHash;
  int frameNumber;
  int quality;

  std::string get_hash() { return pdqHash.format(); }

  /// @brief Convert a serialized vpdqFeature string into a vpdqFeature.
  ///
  /// @param serialized The serialized vpdqFeature string.
  ///
  /// @throws std::invalid_argument if the string is not correctly serialized.
  static vpdqFeature from_str(std::string const& serialized) {
    vpdqFeature feature{
        facebook::pdq::hashing::Hash256::fromHexString(serialized),
        0,
        100,
    };

    return feature;
  }

  std::string to_string() const { return pdqHash.toHexString(); }
};

/// @brief The VPDQ hash of a video.
///
/// A VPDQ hash is a collection of PDQ Hashes. Each PDQ hash is 32 bytes. For
/// this implementation, we are storing them all in one big buffer for
/// performance reasons.
///
/// Unlike Meta's implementation of the VPDQ hash, we aren't storing the hash
/// quality or frame number because it isn't used for similarity search.
///
/// @note A VPDQ hash of a video can be empty if all frames were low quality
/// and filtered out while hashing. This is uncommon, but must be supported.
struct VpdqHash {
  /// @brief The collection of PDQ hashes stored contiguously as raw bytes.
  ///
  /// @note This is NOT a hex string. These are raw bytes.
  std::string pdqHashes;

  /// @brief The number of bytes per raw PDQ hash.
  static constexpr std::uint32_t bytesPerPdqHash = 32;

  /// @brief Create a VpdqHash from the raw pdq hashes.
  static VpdqHash from_bytes(std::string pdqHashesBytes) {
    VpdqHash res{};
    res.pdqHashes = std::move(pdqHashesBytes);
    return res;
  }

  /// @brief Create a VpdqHash from a hex string of pdq hashes.
  static VpdqHash from_string(const std::string& pdqHashesHex) {
    if ((pdqHashesHex.size() % (bytesPerPdqHash * 2U)) != 0) {
      throw std::invalid_argument("Hash string is incorrect length.");
    }
    VpdqHash res{};

    // Empty hashes are allowed. This can happen in the case of all hashes
    // being filtered out due to low quality.
    if (pdqHashesHex.size() == 0U) {
      return res;
    }

    // Hex hashes require 2 bytes per digit, so they're 2x larger than the
    // normal raw bytes hash.
    res.pdqHashes.reserve(pdqHashesHex.size() / 2U);

    for (std::size_t i = 0; i < pdqHashesHex.size(); i += 2) {
      unsigned int byte;
      if (std::sscanf(pdqHashesHex.c_str() + i, "%2x", &byte) != 1) {
        throw std::invalid_argument("Invalid hex string.");
      }
      res.pdqHashes.push_back(static_cast<char>(byte));
    }

    return res;
  }

  /// @brief Serialize the VpdqHash to a hex string of pdq hashes.
  ///
  /// @note The resulting string may be empty if the vpdq hash contains no pdq
  /// hashes.
  std::string to_string() const {
    std::string hex{};
    hex.reserve(pdqHashes.size() * 2);
    char buf[3];
    for (unsigned char c : pdqHashes) {
      std::sprintf(buf, "%02x", c);
      hex.append(buf);
    }
    return hex;
  }

  /// @brief Get the number of PDQ hashes.
  std::uint32_t getHashCount() const {
    return pdqHashes.size() / bytesPerPdqHash;
  }

  /// @brief Get the number of PDQ hashes.
  std::size_t size() const { return getHashCount(); }

  /// @brief Check if there are any PDQ hashes.
  bool empty() const { return pdqHashes.empty(); }

  bool operator==(const VpdqHash& other) const {
    if (this != &other) {
      return pdqHashes == other.pdqHashes;
    }
    return true;
  }

  bool operator!=(const VpdqHash& other) const { return !(*this == other); }
};

} // namespace hashing
} // namespace vpdq
} // namespace facebook

#endif // VPDQHASHTYPE_H
