/*
 * Copyright (c) 2012-2016, chunquedong
 *
 * This file is part of cppfan project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
 *
 * History:
 *   2012-12-23  Jed Young  Creation
 */

#ifndef _CPPF_CACHE_H
#define _CPPF_CACHE_H


#include "LinkedList.h"
#include <unordered_map>

namespace mgp {

/*========================================================================
 * Hash Map
 */
template<typename K, typename V>
class HashMap : public std::unordered_map<K, V> {
public:
    HashMap(size_t tableSize)
        : std::unordered_map<K, V>(tableSize) {}

    virtual ~HashMap() {
        std::unordered_map<K, V>::clear();
    }

    V &get(const K &key, V &defVal) {
        auto itr = this->find(key);
        if (itr == this->end()) {
            return defVal;
        }
        return itr->second;
    }

    /*V &getOrAdd(const K &key, std::function<V(const K&)> func) {
        auto itr = this->find(key);
        if (itr == this->end()) {
            V v = func(key);
            set(key, v);
            return (*this)[key];
        }
        return itr->second;
    }*/

    V &set(const K &key, V &val) {
        (*this)[key] = val;
        return val;
    }

    bool contains(const K &key) const {
        auto itr = this->find(key);
        return itr != this->end();
    }

    bool remove(const K &key) {
        auto itr = this->find(key);
        if (itr == this->end()) {
            return false;
        }
        this->erase(itr);
        return true;
    }
};

/**
 * LRU Cache
 */
template<typename K, typename V>
class Cache {
    struct CacheItem {
        K key;
        V val;
        CacheItem* previous;
        CacheItem* next;
    };
    HashMap<K, CacheItem*> map;
    LinkedList<CacheItem> list;
    long _maxSize;
public:
    Cache(long maxSize) : map(maxSize), _maxSize(maxSize) {
    }

    ~Cache() {
        clear();
    }

    long maxSize() { return _maxSize; }
    void maxSize(long m) { _maxSize = m; }

   V &get(K &key) {
    CacheItem *item = map[key];
    list.remove(item);
    list.insertFirst(item);
    return item->val;
  }

  V &_get(K &key, V &defVal) {
    CacheItem* defNull = nullptr;
    CacheItem *item = map.get(key, defNull);
    if (!item) return defVal;
    return item->val;
  }

  void set(K &key, V &val) {
    CacheItem *item = new CacheItem();
    item->key = key;
    item->val = val;
    map.set(key, item);
    list.insertFirst(item);

    clearUp(_maxSize);

    assert(map.size() == list.size());
  }

  bool contains(K &key) const {
    return map.contains(key);
  }

  long size() const { return map.size(); }

  void clear() {
    clearUp(0);
  }

protected:
  virtual void onRemove(K &key, V &val) {
    //delete val;
  }

private:
  void clearUp(long max) {
    while (map.size() > max) {
      CacheItem *item = list.last();
      map.remove(item->key);
      list.remove(item);
      onRemove(item->key, item->val);
      delete item;
    }
  }
};

}
#endif // CACHE_H
