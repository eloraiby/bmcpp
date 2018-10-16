#ifndef HASHMAP_HPP
#define HASHMAP_HPP

/*
 * Skia source code
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "array.hpp"

namespace BmCpp {


// Before trying to use HashTable, look below to see if HashMap or HashSet works for you.
// They're easier to use, usually perform the same, and have fewer sharp edges.

// T and K are treated as ordinary copyable C++ types.
// Traits must have:
//   - static K GetKey(T)
//   - static uint32_t Hash(K)
// If the key is large and stored inside T, you may want to make K a const&.
// Similarly, if T is large you might want it to be a pointer.
template <typename T, typename K, typename Traits = T>
class HashTable {
public:
    HashTable() : fCount(0), fCapacity(0) {}
    HashTable(HashTable&& other)
        : fCount(other.fCount)
        , fCapacity(other.fCapacity)
        , fSlots(move(other.fSlots)) { other.fCount = other.fCapacity = 0; }

    HashTable& operator=(HashTable&& other) {
        if (this != &other) {
            this->~HashTable();
            new (this) HashTable(move(other));
        }
        return *this;
    }

    // Clear the table.
    void reset() { *this = HashTable(); }

    // How many entries are in the table?
    int count() const { return fCount; }

    // Approximately how many bytes of memory do we use beyond sizeof(*this)?
    size_t approxBytesUsed() const { return fCapacity * sizeof(Slot); }

    // !!!!!!!!!!!!!!!!!                 CAUTION                   !!!!!!!!!!!!!!!!!
    // set(), find() and foreach() all allow mutable access to table entries.
    // If you change an entry so that it no longer has the same key, all hell
    // will break loose.  Do not do that!
    //
    // Please prefer to use SkTHashMap or SkTHashSet, which do not have this danger.

    // The pointers returned by set() and find() are valid only until the next call to set().
    // The pointers you receive in foreach() are only valid for its duration.

    // Copy val into the hash table, returning a pointer to the copy now in the table.
    // If there already is an entry in the table with the same key, we overwrite it.
    T* set(T val) {
        if (4 * fCount >= 3 * fCapacity) {
            this->resize(fCapacity > 0 ? fCapacity * 2 : 4);
        }
        return this->uncheckedSet(move(val));
    }

    // If there is an entry in the table with this key, return a pointer to it.  If not, null.
    T* find(const K& key) const {
        uint32_t hash = Hash(key);
        int index = hash & (fCapacity-1);
        for (int n = 0; n < fCapacity; n++) {
            Slot& s = fSlots[index];
            if (s.empty()) {
                return nullptr;
            }
            if (hash == s.hash && key == Traits::GetKey(s.val)) {
                return &s.val;
            }
            index = this->next(index);
        }
        //SkASSERT(fCapacity == 0);
        return nullptr;
    }

    // Remove the value with this key from the hash table.
    void remove(const K& key) {
       // SkASSERT(this->find(key));

        uint32_t hash = Hash(key);
        int index = hash & (fCapacity-1);
        for (int n = 0; n < fCapacity; n++) {
            Slot& s = fSlots[index];
            //SkASSERT(!s.empty());
            if (hash == s.hash && key == Traits::GetKey(s.val)) {
                fCount--;
                break;
            }
            index = this->next(index);
        }

        // Rearrange elements to restore the invariants for linear probing.
        for (;;) {
            Slot& emptySlot = fSlots[index];
            int emptyIndex = index;
            int originalIndex;
            // Look for an element that can be moved into the empty slot.
            // If the empty slot is in between where an element landed, and its native slot, then
            // move it to the empty slot. Don't move it if its native slot is in between where
            // the element landed and the empty slot.
            // [native] <= [empty] < [candidate] == GOOD, can move candidate to empty slot
            // [empty] < [native] < [candidate] == BAD, need to leave candidate where it is
            do {
                index = this->next(index);
                Slot& s = fSlots[index];
                if (s.empty()) {
                    // We're done shuffling elements around.  Clear the last empty slot.
                    emptySlot = Slot();
                    return;
                }
                originalIndex = s.hash & (fCapacity - 1);
            } while ((index <= originalIndex && originalIndex < emptyIndex)
                     || (originalIndex < emptyIndex && emptyIndex < index)
                     || (emptyIndex < index && index <= originalIndex));
            // Move the element to the empty slot.
            Slot& moveFrom = fSlots[index];
            emptySlot = move(moveFrom);
        }
    }

    // Call fn on every entry in the table.  You may mutate the entries, but be very careful.
    template <typename Fn>  // f(T*)
    void foreach(Fn&& fn) {
        for (int i = 0; i < fCapacity; i++) {
            if (!fSlots[i].empty()) {
                fn(&fSlots[i].val);
            }
        }
    }

    // Call fn on every entry in the table.  You may not mutate anything.
    template <typename Fn>  // f(T) or f(const T&)
    void foreach(Fn&& fn) const {
        for (int i = 0; i < fCapacity; i++) {
            if (!fSlots[i].empty()) {
                fn(fSlots[i].val);
            }
        }
    }

private:
    T* uncheckedSet(T&& val) {
        const K& key = Traits::GetKey(val);
        uint32_t hash = Hash(key);
        int index = hash & (fCapacity-1);
        for (int n = 0; n < fCapacity; n++) {
            Slot& s = fSlots[index];
            if (s.empty()) {
                // New entry.
                s.val  = move(val);
                s.hash = hash;
                fCount++;
                return &s.val;
            }
            if (hash == s.hash && key == Traits::GetKey(s.val)) {
                // Overwrite previous entry.
                // Note: this triggers extra copies when adding the same value repeatedly.
                s.val = move(val);
                return &s.val;
            }

            index = this->next(index);
        }
        //SkASSERT(false);
        return nullptr;
    }

    void resize(int capacity) {
        int oldCapacity = fCapacity;
        //SkDEBUGCODE(int oldCount = fCount);

        fCount = 0;
        fCapacity = capacity;
        Array<Slot> oldSlots = move(fSlots);
        fSlots = Array<Slot>(capacity);

        for (int i = 0; i < oldCapacity; i++) {
            Slot& s = oldSlots[i];
            if (!s.empty()) {
                this->uncheckedSet(move(s.val));
            }
        }
        //SkASSERT(fCount == oldCount);
    }

    int next(int index) const {
        index--;
        if (index < 0) { index += fCapacity; }
        return index;
    }

    static uint32_t Hash(const K& key) {
        uint32_t hash = Traits::Hash(key);
        return hash ? hash : 1;  // We reserve hash 0 to mark empty.
    }

    struct Slot {
        Slot() : hash(0) {}
        Slot(T&& v, uint32_t h) : val(move(v)), hash(h) {}
        Slot(Slot&& o) { *this = move(o); }
        Slot& operator=(Slot&& o) {
            val  = move(o.val);
            hash = o.hash;
            return *this;
        }

        bool empty() const { return this->hash == 0; }

        T        val;
        uint32_t hash;
    };

    int fCount, fCapacity;
    Array<Slot> fSlots;

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;
};

// Maps K->V.  A more user-friendly wrapper around SkTHashTable, suitable for most use cases.
// K and V are treated as ordinary copyable C++ types, with no assumed relationship between the two.
template <typename K, typename V>
class HashMap {
public:
    HashMap() {}
    HashMap(HashMap&&) = default;
    HashMap& operator=(HashMap&&) = default;

    // Clear the map.
    void reset() { fTable.reset(); }

    // How many key/value pairs are in the table?
    int count() const { return fTable.count(); }

    // Approximately how many bytes of memory do we use beyond sizeof(*this)?
    size_t approxBytesUsed() const { return fTable.approxBytesUsed(); }

    // N.B. The pointers returned by set() and find() are valid only until the next call to set().

    // Set key to val in the table, replacing any previous value with the same key.
    // We copy both key and val, and return a pointer to the value copy now in the table.
    V* set(K key, V val) {
        Pair* out = fTable.set({move(key), move(val)});
        return &out->val;
    }

    // If there is key/value entry in the table with this key, return a pointer to the value.
    // If not, return null.
    V* find(const K& key) const {
        if (Pair* p = fTable.find(key)) {
            return &p->val;
        }
        return nullptr;
    }

    // Remove the key/value entry in the table with this key.
    void remove(const K& key) {
        //SkASSERT(this->find(key));
        fTable.remove(key);
    }

    // Call fn on every key/value pair in the table.  You may mutate the value but not the key.
    template <typename Fn>  // f(K, V*) or f(const K&, V*)
    void foreach(Fn&& fn) {
        fTable.foreach([&fn](Pair* p){ fn(p->key, &p->val); });
    }

    // Call fn on every key/value pair in the table.  You may not mutate anything.
    template <typename Fn>  // f(K, V), f(const K&, V), f(K, const V&) or f(const K&, const V&).
    void foreach(Fn&& fn) const {
        fTable.foreach([&fn](const Pair& p){ fn(p.key, p.val); });
    }

private:
    struct Pair {
        K key;
        V val;
        static const K& GetKey(const Pair& p) { return p.key; }
        static uint32_t Hash(const K& key) { return hashFn<K>(key); }
    };

    HashTable<Pair, K> fTable;

    HashMap(const HashMap&) = delete;
    HashMap& operator=(const HashMap&) = delete;
};

// A set of T.  T is treated as an ordinary copyable C++ type.
template <typename T>
class HashSet {
public:
    HashSet() {}
    HashSet(HashSet&&) = default;
    HashSet& operator=(HashSet&&) = default;

    // Clear the set.
    void reset() { fTable.reset(); }

    // How many items are in the set?
    int count() const { return fTable.count(); }

    // Approximately how many bytes of memory do we use beyond sizeof(*this)?
    size_t approxBytesUsed() const { return fTable.approxBytesUsed(); }

    // Copy an item into the set.
    void add(T item) { fTable.set(move(item)); }

    // Is this item in the set?
    bool contains(const T& item) const { return SkToBool(this->find(item)); }

    // If an item equal to this is in the set, return a pointer to it, otherwise null.
    // This pointer remains valid until the next call to add().
    const T* find(const T& item) const { return fTable.find(item); }

    // Remove the item in the set equal to this.
    void remove(const T& item) {
        //SkASSERT(this->contains(item));
        fTable.remove(item);
    }

    // Call fn on every item in the set.  You may not mutate anything.
    template <typename Fn>  // f(T), f(const T&)
    void foreach (Fn&& fn) const {
        fTable.foreach(fn);
    }

private:
    struct Traits {
        static const T& GetKey(const T& item) { return item; }
        static uint32_t Hash(const T& item) { return hashFn<T>(item); }
    };
    HashTable<T, T, Traits> fTable;

    HashSet(const HashSet&) = delete;
    HashSet& operator=(const HashSet&) = delete;
};


}

#endif // HASHMAP_HPP
