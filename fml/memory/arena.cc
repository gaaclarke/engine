// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/memory/arena.h"
#include <cstdlib>

namespace fml {
Arena::Arena(size_t size)
    : block_(static_cast<uint8_t*>(malloc(size))), size_(size) {
  position_ = block_;
}

Arena::~Arena() {
  free(block_);
}

void* Arena::RawAllocate(size_t size) {
  uint8_t* new_pos = position_ + size;
  if (new_pos > block_ + size_) {
    // TODO(gaaclarke): Overflow to a new block.
    return nullptr;
  }
  void* result = position_;
  position_ = new_pos;
  return result;
}

}  // namespace fml
