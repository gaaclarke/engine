// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/display_list_debugger.h"

#include <atomic>
#include <fstream>

#include "flutter/fml/logging.h"
#include "flutter/testing/display_list_testing.h"
#include "third_party/skia/include/core/SkRect.h"

namespace {
std::atomic<char*> saveDisplayListPath;
template <typename T>
using FreePtr = std::unique_ptr<T, decltype(&free)>;
}  // namespace

namespace flutter {
void DisplayListDebugger::HandleMessage(
    std::unique_ptr<PlatformMessage> message) {
  const fml::MallocMapping& data = message->data();
  FreePtr<char>(saveDisplayListPath.exchange(
                    strdup(reinterpret_cast<const char*>(data.GetMapping()))),
                &free);
}

fml::Status DisplayListDebugger::SaveDisplayList(
    const sk_sp<DisplayList>& display_list) {
  if (FreePtr<char> path =
          FreePtr<char>(saveDisplayListPath.exchange(nullptr), &free)) {
    fml::UniqueFD dir = fml::OpenDirectory(
        path.get(), /*create_if_necessary=*/false, fml::FilePermission::kWrite);
    // DisplayList doesn't have an accessor for the byte size of the storage,
    // adding one may confuse the api so we derive it with bytes().
    std::string out_path = std::string(path.get()) + "/display_list.dat";
    std::ofstream file(out_path);
    FML_DCHECK(file.good());
    flutter::testing::operator<<(file, *display_list);
    // std::ofstream file(out_path, std::ios::binary | std::ios::trunc);

    // const char magic[] = "dspl";
    // file.write(magic, sizeof(magic) - 1);
    // int32_t version = 1;
    // file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    // file.write(reinterpret_cast<const char*>(&display_list->byte_count_),
    //            sizeof(display_list->byte_count_));
    // file.write(reinterpret_cast<const char*>(display_list->storage_.get()),
    //            display_list->byte_count_);
    // file.write(reinterpret_cast<const char*>(&display_list->op_count_),
    //            sizeof(display_list->op_count_));
    // file.write(reinterpret_cast<const
    // char*>(&display_list->nested_byte_count_),
    //            sizeof(display_list->nested_byte_count_));
    // file.write(reinterpret_cast<const
    // char*>(&display_list->nested_op_count_),
    //            sizeof(display_list->nested_op_count_));
    // file.write(reinterpret_cast<const char*>(&display_list->total_depth_),
    //            sizeof(display_list->total_depth_));
    // file.write(reinterpret_cast<const char*>(&display_list->bounds_.fLeft),
    //            sizeof(display_list->bounds_.fLeft));
    // file.write(reinterpret_cast<const char*>(&display_list->bounds_.fTop),
    //            sizeof(display_list->bounds_.fTop));
    // file.write(reinterpret_cast<const char*>(&display_list->bounds_.fRight),
    //            sizeof(display_list->bounds_.fRight));
    // file.write(reinterpret_cast<const char*>(&display_list->bounds_.fBottom),
    //            sizeof(display_list->bounds_.fBottom));
    // file.write(
    //     reinterpret_cast<const
    //     char*>(&display_list->can_apply_group_opacity_),
    //     sizeof(display_list->can_apply_group_opacity_));
    // file.write(reinterpret_cast<const
    // char*>(&display_list->is_ui_thread_safe_),
    //            sizeof(display_list->is_ui_thread_safe_));
    // file.write(reinterpret_cast<const char*>(
    //                &display_list->modifies_transparent_black_),
    //            sizeof(display_list->modifies_transparent_black_));

    bool success = file.good();
    file.close();
    FML_LOG(ERROR) << "store display_list (" << success << "):" << out_path;
    return success ? fml::Status()
                   : fml::Status(fml::StatusCode::kUnknown, "write failed");
  }
  return fml::Status(fml::StatusCode::kUnavailable, "unavailable");
}
}  // namespace flutter
