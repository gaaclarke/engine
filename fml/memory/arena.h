// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstddef>
#include <cstdint>
#include <type_traits>

#ifndef FLUTTER_FML_MEMORY_ARENA_H_
#define FLUTTER_FML_MEMORY_ARENA_H_

namespace fml {

class Arena {
 public:
  Arena(size_t size);

  ~Arena();

  template <typename T>
  T* Allocate(size_t count) {
    static_assert(std::is_trivially_destructible<T>::value,
                  "T must be trivially destructible");
    return static_cast<T*>(RawAllocate(sizeof(T) * count));
  }

 private:
  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;

  void* RawAllocate(size_t size);

  uint8_t* block_ = nullptr;
  const size_t size_;
  uint8_t* position_;
};

template <typename T>
class ArenaAllocator {
 public:
  typedef T value_type;
  static_assert(std::is_trivially_destructible<T>::value,
                "T must be trivially destructible");

  ArenaAllocator(Arena& arena) : arena_(arena) {}

  T* allocate(size_t n) { return static_cast<T*>(arena_.Allocate<T>(n)); }

  void deallocate(T* ptr, size_t n) {
    // noop, the ~Arena will delete it.
  }

 private:
  Arena& arena_;
};

}  // namespace fml

#endif  // FLUTTER_FML_MEMORY_ARENA_H_
