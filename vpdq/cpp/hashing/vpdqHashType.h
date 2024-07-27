// ================================================================
// Copyright (c) Meta Platforms, Inc. and affiliates.
// ================================================================

#ifndef VPDQHASHTYPE_H
#define VPDQHASHTYPE_H

#include <pdq/cpp/common/pdqhashtypes.h>

namespace facebook {
namespace vpdq {
namespace hashing {

struct vpdqFeature {
  facebook::pdq::hashing::Hash256 pdqHash;
  int frameNumber;
  int quality;
  double timeStamp;

  std::string get_hash()
  {
      return pdqHash.format();
  }

  int get_frame_number()
  {
      return frameNumber;
  }

  int get_quality()
  {
      return quality;
  }

  float get_timestamp()
  {
      return timeStamp;
  }
};

} // namespace hashing
} // namespace vpdq
} // namespace facebook

#endif // VPDQHASHTYPE_H
