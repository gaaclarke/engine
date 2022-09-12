// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/platform_view_embedder.h"

#include "flutter/fml/make_copyable.h"

namespace flutter {

class PlatformViewEmbedder::EmbedderPlatformMessageHandler
    : public PlatformMessageHandler {
 public:
  EmbedderPlatformMessageHandler(
      fml::WeakPtr<PlatformView> parent,
      fml::RefPtr<fml::TaskRunner> platform_task_runner)
      : parent_(parent), platform_task_runner_(platform_task_runner) {
    FML_LOG(INFO) << "aaclarke: EmbedderPlatformMessageHandler";
  }

  virtual void HandlePlatformMessage(std::unique_ptr<PlatformMessage> message) {
    FML_LOG(INFO) << "aaclarke: HandlePlatformMessage " << message->channel();
    platform_task_runner_->PostTask(fml::MakeCopyable(
        [parent = parent_, message = std::move(message)]() mutable {
          if (parent) {
            parent->HandlePlatformMessage(std::move(message));
          } else {
            FML_DLOG(WARNING)
                << "Dropping message on channel " << message->channel();
          }
        }));
  }
  virtual void InvokePlatformMessageResponseCallback(
      int response_id,
      std::unique_ptr<fml::Mapping> mapping) {}
  virtual void InvokePlatformMessageEmptyResponseCallback(int response_id) {}

 private:
  fml::WeakPtr<PlatformView> parent_;
  fml::RefPtr<fml::TaskRunner> platform_task_runner_;
};

PlatformViewEmbedder::PlatformViewEmbedder(
    PlatformView::Delegate& delegate,
    flutter::TaskRunners task_runners,
    EmbedderSurfaceSoftware::SoftwareDispatchTable software_dispatch_table,
    PlatformDispatchTable platform_dispatch_table,
    std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : PlatformView(delegate, std::move(task_runners)),
      external_view_embedder_(external_view_embedder),
      embedder_surface_(
          std::make_unique<EmbedderSurfaceSoftware>(software_dispatch_table,
                                                    external_view_embedder_)),
      platform_dispatch_table_(platform_dispatch_table) {
  FML_LOG(INFO) << "aaclarke: PlatformViewEmbedder 0";
  platform_message_handler_ = std::make_shared<EmbedderPlatformMessageHandler>(
      this->GetWeakPtr(), task_runners_.GetPlatformTaskRunner());
  FML_LOG(INFO) << "aaclarke: PlatformViewEmbedder 1";
}

#ifdef SHELL_ENABLE_GL
PlatformViewEmbedder::PlatformViewEmbedder(
    PlatformView::Delegate& delegate,
    flutter::TaskRunners task_runners,
    EmbedderSurfaceGL::GLDispatchTable gl_dispatch_table,
    bool fbo_reset_after_present,
    PlatformDispatchTable platform_dispatch_table,
    std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : PlatformView(delegate, std::move(task_runners)),
      external_view_embedder_(external_view_embedder),
      embedder_surface_(
          std::make_unique<EmbedderSurfaceGL>(gl_dispatch_table,
                                              fbo_reset_after_present,
                                              external_view_embedder_)),
      platform_dispatch_table_(platform_dispatch_table) {
  FML_LOG(INFO) << "aaclarke: PlatformViewEmbedder 2";
  platform_message_handler_ = std::make_shared<EmbedderPlatformMessageHandler>(
      this->GetWeakPtr(), task_runners_.GetPlatformTaskRunner());
  FML_LOG(INFO) << "aaclarke: PlatformViewEmbedder 3";
}
#endif

#ifdef SHELL_ENABLE_METAL
PlatformViewEmbedder::PlatformViewEmbedder(
    PlatformView::Delegate& delegate,
    flutter::TaskRunners task_runners,
    std::unique_ptr<EmbedderSurfaceMetal> embedder_surface,
    PlatformDispatchTable platform_dispatch_table,
    std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : PlatformView(delegate, std::move(task_runners)),
      external_view_embedder_(external_view_embedder),
      embedder_surface_(std::move(embedder_surface)),
      platform_dispatch_table_(platform_dispatch_table) {
  FML_LOG(INFO) << "aaclarke: PlatformViewEmbedder 4";
  platform_message_handler_ = std::make_shared<EmbedderPlatformMessageHandler>(
      this->GetWeakPtr(), task_runners_.GetPlatformTaskRunner());
  FML_LOG(INFO) << "aaclarke: PlatformViewEmbedder 5";
}
#endif

#ifdef SHELL_ENABLE_VULKAN
PlatformViewEmbedder::PlatformViewEmbedder(
    PlatformView::Delegate& delegate,
    flutter::TaskRunners task_runners,
    std::unique_ptr<EmbedderSurfaceVulkan> embedder_surface,
    PlatformDispatchTable platform_dispatch_table,
    std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : PlatformView(delegate, std::move(task_runners)),
      external_view_embedder_(external_view_embedder),
      embedder_surface_(std::move(embedder_surface)),
      platform_dispatch_table_(platform_dispatch_table) {
  FML_LOG(INFO) << "aaclarke: PlatformViewEmbedder 6";
  platform_message_handler_ = std::make_shared<EmbedderPlatformMessageHandler>(
      this->GetWeakPtr(), task_runners_.GetPlatformTaskRunner());
  FML_LOG(INFO) << "aaclarke: PlatformViewEmbedder 7";
}
#endif

void PlatformViewEmbedder::UpdateSemantics(
    flutter::SemanticsNodeUpdates update,
    flutter::CustomAccessibilityActionUpdates actions) {
  if (platform_dispatch_table_.update_semantics_callback != nullptr) {
    platform_dispatch_table_.update_semantics_callback(std::move(update),
                                                       std::move(actions));
  }
}

void PlatformViewEmbedder::HandlePlatformMessage(
    std::unique_ptr<flutter::PlatformMessage> message) {
  if (!message) {
    return;
  }

  if (platform_dispatch_table_.platform_message_response_callback == nullptr) {
    if (message->response()) {
      message->response()->CompleteEmpty();
    }
    return;
  }

  platform_dispatch_table_.platform_message_response_callback(
      std::move(message));
}

// |PlatformView|
std::unique_ptr<Surface> PlatformViewEmbedder::CreateRenderingSurface() {
  if (embedder_surface_ == nullptr) {
    FML_LOG(ERROR) << "Embedder surface was null.";
    return nullptr;
  }
  return embedder_surface_->CreateGPUSurface();
}

// |PlatformView|
std::shared_ptr<ExternalViewEmbedder>
PlatformViewEmbedder::CreateExternalViewEmbedder() {
  return external_view_embedder_;
}

// |PlatformView|
sk_sp<GrDirectContext> PlatformViewEmbedder::CreateResourceContext() const {
  if (embedder_surface_ == nullptr) {
    FML_LOG(ERROR) << "Embedder surface was null.";
    return nullptr;
  }
  return embedder_surface_->CreateResourceContext();
}

// |PlatformView|
std::unique_ptr<VsyncWaiter> PlatformViewEmbedder::CreateVSyncWaiter() {
  if (!platform_dispatch_table_.vsync_callback) {
    // Superclass implementation creates a timer based fallback.
    return PlatformView::CreateVSyncWaiter();
  }

  return std::make_unique<VsyncWaiterEmbedder>(
      platform_dispatch_table_.vsync_callback, task_runners_);
}

// |PlatformView|
std::unique_ptr<std::vector<std::string>>
PlatformViewEmbedder::ComputePlatformResolvedLocales(
    const std::vector<std::string>& supported_locale_data) {
  if (platform_dispatch_table_.compute_platform_resolved_locale_callback !=
      nullptr) {
    return platform_dispatch_table_.compute_platform_resolved_locale_callback(
        supported_locale_data);
  }
  std::unique_ptr<std::vector<std::string>> out =
      std::make_unique<std::vector<std::string>>();
  return out;
}

// |PlatformView|
void PlatformViewEmbedder::OnPreEngineRestart() const {
  if (platform_dispatch_table_.on_pre_engine_restart_callback != nullptr) {
    platform_dispatch_table_.on_pre_engine_restart_callback();
  }
}

std::shared_ptr<PlatformMessageHandler>
PlatformViewEmbedder::GetPlatformMessageHandler() const {
  return platform_message_handler_;
}

}  // namespace flutter
