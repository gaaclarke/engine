// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/macros.h"
#include "gtest/gtest.h"
#include "flutter/fml/memory/arena.h"


TEST(Arena, StdVector) {  
  fml::Arena arena(1024);
  fml::ArenaAllocator<int> allocator(arena);
  std::vector<int, fml::ArenaAllocator<int>> ints(allocator);
  ints.push_back(1);
  ints.push_back(2);
  ints.push_back(3);
  EXPECT_EQ(ints.size(), 3u);
  EXPECT_EQ(ints[0], 1);
  EXPECT_EQ(ints[1], 2);
  EXPECT_EQ(ints[2], 3);
}

TEST(Arena, Overflow) {
  fml::Arena arena(sizeof(int) * 2);
  EXPECT_TRUE(arena.Allocate<int>(1));
  EXPECT_TRUE(arena.Allocate<int>(1));
  EXPECT_FALSE(arena.Allocate<int>(1));
}
