/*
 * Copyright (c) 2012-2016, chunquedong
 *
 * This file is part of cppfan project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
 *
 * History:
 *   2012-12-23  Jed Young  Creation
 */

#ifndef ThreadPool_hpp
#define ThreadPool_hpp

#include <mutex>
#include <thread>
#include <vector>

#include "Ref.h"
#include "Queue.h"
#include "Ptr.h"

namespace mgp {


class Task : public Refable {
  friend class ThreadPool;
protected:
  bool _done;
  bool canceled;
  
public:
  Task()
  : _done(false) , canceled(false) {
  }
  
  virtual ~Task() {}
  
  virtual void run() {}
  
  virtual void cancel() { canceled = true; }
  bool isCanceled() { return canceled; }
  bool isDone() { return _done; }
  
  virtual void done() {
    _done = true;
  }
};

/**
 * Thread Pool
 */
class ThreadPool {
public:
  typedef SPtr<Task> TaskPtr;
  
protected:
  BlockingQueue<TaskPtr> queue;
  bool onlyRunLatest;
  
private:
  std::vector<std::thread> threadList;
  int threadSize;
  
public:
  ThreadPool(int threadSize);
  virtual ~ThreadPool();
  
  bool start();
  
  void stop();
  
  void addTask(TaskPtr task);
  
  /**
   * cancel and remove the task
   */
  void remove(Task *task);
  
  /**
   * call visit for each one task
   */
  void each();
  
protected:
  virtual bool visit(Task *t, int index) { return true; }
  
private:
  void run();
  static int enterPoint(void *arg);
};

}

#endif /* ThreadPool_hpp */
