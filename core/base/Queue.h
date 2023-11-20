/*
 * Copyright (c) 2012-2016, chunquedong
 *
 * This file is part of cppfan project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
 *
 * History:
 *   2012-12-23  Jed Young  Creation
 */
#ifndef Queue_hpp
#define Queue_hpp

#include <list>
#include <mutex>

namespace mgp {

template<typename T>
class Queue {
protected:
  std::list<T> queue;
public:
  Queue() {
  }
  
  virtual ~Queue() {
  }
  
  virtual size_t size() {
    return queue.size();
  }
  
  /**
   * add to back
   */
  virtual void enqueue(T &t) {
    queue.push_back(t);
  }
  
  /**
   * return false if empty
   */
  virtual bool dequeue(T &t) {
    if (queue.empty()) {
      return false;
    }
    
    t = queue.front();
    queue.pop_front();
    return true;
  }
  
  virtual void clear() {
    queue.clear();
  }
  
  std::list<T> &getRawQueue() { return queue; }
};

template<typename T>
class PriorityQueue : public Queue<T> {
public:
  void enqueue(T &t) {
    
    typename std::list<T>::reverse_iterator rb = Queue<T>::queue.rbegin();
    typename std::list<T>::reverse_iterator re = Queue<T>::queue.rend();
    
    bool added = false;
    while (rb != re) {
      if (t.priority <= (*rb).priority) {
        //base pointer to next element
        Queue<T>::queue.insert(rb.base(), t);
        added = true;
        break;
      }
      ++rb;
    }
    if (!added) {
      Queue<T>::queue.push_front(t);
    }
  }
};

/**
 * Concurrent Linked Queue
 */
template<typename T>
class ConcurrentQueue : public Queue<T> {
protected:
  std::mutex mutex;
  typedef std::lock_guard<std::mutex> lock_guard;
public:
  ConcurrentQueue() {
  }
  
  virtual ~ConcurrentQueue() {
  }
  
  size_t size() {
    lock_guard guard(mutex);
    return Queue<T>::queue.size();
  }
  
  /**
   * add to back
   */
  virtual void enqueue(T &t) {
    lock_guard guard(mutex);
    Queue<T>::queue.push_back(t);
  }
  
  /**
   * return false if empty
   */
  virtual bool dequeue(T &t) {
    lock_guard guard(mutex);
    
    if (Queue<T>::queue.empty()) {
      return false;
    }
    
    t = Queue<T>::queue.front();
    Queue<T>::queue.pop_front();
    
    return true;
  }
  
  void clear() {
    lock_guard guard(mutex);
    Queue<T>::queue.clear();
  }
  
  void lock() { mutex.lock(); }
  void unlock() { mutex.unlock(); }
};


/**
 * BlockingQueue
 * Blocking when delete from empty queue.
 */
template<typename T>
class BlockingQueue : public ConcurrentQueue<T> {
protected:
  std::condition_variable deleteCond;
  bool cancelDelete;
  
public:
  BlockingQueue() {
    cancelDelete = false;
  }
  
  ~BlockingQueue() {
  }
  
  void enqueue(T &t) {
    ConcurrentQueue<T>::lock();
    
    Queue<T>::queue.push_back(t);
    deleteCond.notify_one();
    
    ConcurrentQueue<T>::unlock();
  }
  
  bool dequeue(T &t) {
    //int rc;
    std::unique_lock<std::mutex> lk(ConcurrentQueue<T>::mutex);
    
    while (!cancelDelete) {
      if (Queue<T>::queue.empty()) {
        deleteCond.wait(lk);
      } else {
        t = Queue<T>::queue.front();
        Queue<T>::queue.pop_front();
        
        //ConcurrentQueue<T>::unlock();
        return true;
      }
    }
    
    return false;
  }
  
  void cancel() {
    ConcurrentQueue<T>::lock();
    
    cancelDelete = true;
    deleteCond.notify_all();
    
    ConcurrentQueue<T>::unlock();
  }
  
  bool isCanceled() {
    return cancelDelete;
  }
};

}
#endif /* Queue_hpp */
