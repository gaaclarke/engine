// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/scoped_task_runner.h"

#include "flutter/fml/message_loop_impl.h"
#include "flutter/fml/synchronization/waitable_event.h"

namespace fml {
ScopedTaskRunner::ScopedTaskRunner(const fml::RefPtr<TaskRunner>& task_runner)
    : TaskRunner(nullptr), task_runner_(task_runner), blocker_(new Blocker()) {
  blocker_->Retain();
}

ScopedTaskRunner::~ScopedTaskRunner() {
  CancelFutureTasks();
  task_runner_->PostTask([blocker = this->blocker_] { blocker->Release(); });
}

void ScopedTaskRunner::CancelFutureTasks() {
  // if (task_runner_->RunsTasksOnCurrentThread()) {
  //   blocker_->SetIsRunning(false);
  // } else {
  //   AutoResetWaitableEvent latch;
  //   task_runner_->PostTask([this, &latch] {
  //     blocker_->SetIsRunning(false);
  //     latch.Signal();
  //   });
  //   latch.Wait();
  // }
}

void ScopedTaskRunner::PostTask(const fml::closure& task) {
  task_runner_->PostTask(task);
  // blocker_->Retain();
  // task_runner_->PostTask([blocker = this->blocker_, task] {
  //   FML_LOG(ERROR) << "is running:" << blocker->IsRunning();
  //   if (blocker->IsRunning()) {
  //     task();
  //   }
  //   blocker->Release();
  // });
}

void ScopedTaskRunner::PostTaskForTime(const fml::closure& task,
                                       fml::TimePoint target_time) {
  task_runner_->PostTaskForTime(task, target_time);
  // blocker_->Retain();
  // task_runner_->PostTaskForTime(
  //     [blocker = this->blocker_, task] {
  //       FML_LOG(ERROR) << "is running:" << blocker->IsRunning();
  //       if (blocker->IsRunning()) {
  //         task();
  //       }
  //       blocker->Release();
  //     },
  //     target_time);
}

void ScopedTaskRunner::PostDelayedTask(const fml::closure& task,
                                       fml::TimeDelta delay) {
  task_runner_->PostDelayedTask(task, delay);
  // blocker_->Retain();
  // task_runner_->PostDelayedTask(
  //     [blocker = this->blocker_, task] {
  //       FML_LOG(ERROR) << "is running:" << blocker->IsRunning();
  //       if (blocker->IsRunning()) {
  //         task();
  //       }
  //       blocker->Release();
  //     },
  //     delay);
}

bool ScopedTaskRunner::RunsTasksOnCurrentThread() {
  return task_runner_->RunsTasksOnCurrentThread();
}

TaskQueueId ScopedTaskRunner::GetTaskQueueId() {
  return task_runner_->GetTaskQueueId();
}

}  // namespace fml
