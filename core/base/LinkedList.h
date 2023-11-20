/*
 * Copyright (c) 2012-2016, chunquedong
 *
 * This file is part of cppfan project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
 *
 * History:
 *   2012-12-23  Jed Young  Creation
 */

#ifndef _CPPF_LINKEDLIST_H_
#define _CPPF_LINKEDLIST_H_

#include "stddef.h"

namespace mgp {

/**
 * Intrusive linked list
 * the element must provider 'previous' and 'next' field
 */
template<typename T>
class LinkedList {
  T head;
  T &tail;
  int length;
public:
  LinkedList() : tail(head), length(0) {
    head.previous = &head;
    head.next = &head;
  }
  
  void _clear() {
    tail = head;
    head.previous = &head;
    head.next = &head;
    length = 0;
  }
  
  int size() { return length; }
  
  LinkedList &add(T *elem) {
    T *left = tail.previous;
    T *right = left->next;
    elem->next = right;
    right->previous = elem;
    elem->previous = left;
    left->next = elem;
    ++length;
    return *this;
  }
  
  void insertFirst(T *elem) {
    T *left = &this->head;
    T *right = left->next;
    elem->next = right;
    right->previous = elem;
    elem->previous = left;
    left->next = elem;
    ++length;
  }
  
  T *getAt(int index) {
    T *elem;
    int i = 0;
    elem = this->head.next;
    while (elem != &this->tail) {
      if (i == index) {
        return elem;
      }
      elem = elem->next;
      ++i;
    }
    return NULL;
  }
  
  void insertBefore(T *elem, T *pos) {
    cf_assert(pos);
    cf_assert(elem);
    
    T *left = pos->previous;
    T *right = pos;
    elem->next = right;
    right->previous = elem;
    elem->previous = left;
    left->next = elem;
    ++length;
  }
  
  bool isEmpty() {
    return head.next == &tail;
  }
  
  bool remove(T *elem) {
    if (elem == NULL) return false;
    elem->previous->next = elem->next;
    elem->next->previous = elem->previous;
    --length;
    return true;
  }
  
  T *first() { return head.next; }
  T *last() { return tail.previous; }
  T *end() { return &tail; }
};

}
#endif // LINKEDLIST_H
