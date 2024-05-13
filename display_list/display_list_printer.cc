// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fstream>
#include <iostream>
#include <vector>

#include "flutter/display_list/display_list.h"
#include "flutter/testing/display_list_testing.h"

namespace flutter {
namespace testing {

class DisplayListPrinter {
 public:
  static void print(const char* path) {
    std::ifstream file(path, std::ios::binary);

    char magic[5] = {0};
    file.read(magic, sizeof(magic) - 1);
    assert(strcmp(magic, "dspl") == 0);

    int32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    assert(version == 1);

    int64_t byte_count;
    file.read(reinterpret_cast<char*>(&byte_count), sizeof(byte_count));

    DisplayListStorage storage;
    storage.realloc(byte_count);
    file.read(reinterpret_cast<char*>(storage.get()), byte_count);

    uint32_t op_count;
    file.read(reinterpret_cast<char*>(&op_count), sizeof(op_count));

    size_t nested_byte_count;
    file.read(reinterpret_cast<char*>(&nested_byte_count),
              sizeof(nested_byte_count));

    uint32_t nested_op_count;
    file.read(reinterpret_cast<char*>(&nested_op_count),
              sizeof(nested_op_count));

    uint32_t total_depth;
    file.read(reinterpret_cast<char*>(&total_depth), sizeof(total_depth));

    SkRect bounds;
    file.read(reinterpret_cast<char*>(&bounds.fLeft), sizeof(bounds.fLeft));
    file.read(reinterpret_cast<char*>(&bounds.fTop), sizeof(bounds.fTop));
    file.read(reinterpret_cast<char*>(&bounds.fRight), sizeof(bounds.fRight));
    file.read(reinterpret_cast<char*>(&bounds.fBottom), sizeof(bounds.fBottom));

    bool can_apply_group_opacity;
    file.read(reinterpret_cast<char*>(&can_apply_group_opacity),
              sizeof(can_apply_group_opacity));

    bool is_ui_thread_safe;
    file.read(reinterpret_cast<char*>(&is_ui_thread_safe),
              sizeof(is_ui_thread_safe));

    bool modifies_transparent_black;
    file.read(reinterpret_cast<char*>(&modifies_transparent_black),
              sizeof(modifies_transparent_black));

    DisplayList display_list(std::move(storage), byte_count, op_count,
                             nested_byte_count, nested_op_count, total_depth,
                             bounds, can_apply_group_opacity, is_ui_thread_safe,
                             modifies_transparent_black, nullptr);

    std::cout << display_list;
  }
};

}  // namespace testing
}  // namespace flutter

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    std::cout << "usage: display_list_printer <path to display list>"
              << std::endl;
  }
  flutter::testing::DisplayListPrinter::print(argv[1]);
  return 0;
}
