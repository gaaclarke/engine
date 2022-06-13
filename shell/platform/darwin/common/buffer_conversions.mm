// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/common/buffer_conversions.h"

#include "flutter/fml/platform/darwin/scoped_nsobject.h"

namespace flutter {
namespace {
class NSDataMapping : public fml::Mapping {
 public:
  explicit NSDataMapping(NSData* data) : data_([data retain]) {}

  size_t GetSize() const override { return [data_.get() length]; }

  const uint8_t* GetMapping() const override {
    return static_cast<const uint8_t*>([data_.get() bytes]);
  }

  uint8_t* GetMutableMapping() const override {
    // This is safe because the NSData instances we are wrapping all live in RAM
    // and are short lived objects that live for the purpose of responding to
    // messages.  We could convert the API to use NSMutableData but that is a
    // breaking change and allocating a NSMutableData is much slower than NSData
    // unfortunately.
    return const_cast<uint8_t*>(static_cast<const uint8_t*>([data_.get() bytes]));
  }

  bool IsDontNeedSafe() const override { return false; }

 private:
  fml::scoped_nsobject<NSData> data_;
  FML_DISALLOW_COPY_AND_ASSIGN(NSDataMapping);
};
}  // namespace

fml::MallocMapping CopyNSDataToMapping(NSData* data) {
  const uint8_t* bytes = static_cast<const uint8_t*>(data.bytes);
  return fml::MallocMapping::Copy(bytes, data.length);
}

NSData* ConvertMappingToNSData(fml::MallocMapping buffer) {
  size_t size = buffer.GetSize();
  return [NSData dataWithBytesNoCopy:buffer.Release() length:size];
}

std::unique_ptr<fml::Mapping> ConvertNSDataToMappingPtr(NSData* data) {
  return std::make_unique<NSDataMapping>(data);
}

NSData* CopyMappingPtrToNSData(std::unique_ptr<fml::Mapping> mapping) {
  return [NSData dataWithBytes:mapping->GetMapping() length:mapping->GetSize()];
}

}  // namespace flutter
