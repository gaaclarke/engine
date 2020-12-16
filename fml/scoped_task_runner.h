// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_SCOPED_TASK_RUNNER_H_
#define FLUTTER_FML_SCOPED_TASK_RUNNER_H_

#include "flutter/fml/task_runner.h"

namespace fml {
class ScopedTaskRunner : public TaskRunner {
 public:
  ScopedTaskRunner(const fml::RefPtr<TaskRunner>& task_runner);

  ~ScopedTaskRunner();

  void CancelFutureTasks();

  const fml::RefPtr<TaskRunner>& GetTaskRunner() const { return task_runner_; }

  virtual void PostTask(const fml::closure& task) override;

  virtual void PostTaskForTime(const fml::closure& task,
                               fml::TimePoint target_time) override;

  virtual void PostDelayedTask(const fml::closure& task,
                               fml::TimeDelta delay) override;

  virtual bool RunsTasksOnCurrentThread() override;

  virtual TaskQueueId GetTaskQueueId() override;

 private:
  class Blocker {
   public:
    Blocker() = default;

    void Retain() { count_.fetch_add(1); }
    void Release() {
      if (count_.fetch_add(-1) <= 0) {
        delete this;
      }
    }

    bool IsRunning() const { return is_running_; }
    void SetIsRunning(bool is_running) { is_running_ = is_running; }

   private:
    bool is_running_ = true;
    std::atomic<int> count_ = 0;
    FML_DISALLOW_COPY_AND_ASSIGN(Blocker);
  };
  fml::RefPtr<TaskRunner> task_runner_;
  Blocker* blocker_;

  FML_DISALLOW_COPY_AND_ASSIGN(ScopedTaskRunner);
};
}  // namespace fml

#endif  // FLUTTER_FML_SCOPED_TASK_RUNNER_H_
