// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_DISPLAY_LIST_DEBUGGER_H_
#define FLUTTER_SHELL_COMMON_DISPLAY_LIST_DEBUGGER_H_

#include "flutter/display_list/display_list.h"
#include "flutter/fml/status.h"
#include "flutter/lib/ui/window/platform_message.h"

namespace flutter {

/// Utility for saving display lists to disk in response to platform messages.
class DisplayListDebugger {
 public:
  static constexpr char kChannelName[] = "flutter/display_list_debugger";
  static void HandleMessage(std::unique_ptr<PlatformMessage> message);
  static fml::Status SaveDisplayList(const sk_sp<DisplayList>& display_list);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_DISPLAY_LIST_DEBUGGER_H_
