//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// ArenaAllocator GarbageList
// auto detect / cast delete or free
//-----------------------------------------------------------------------------
#pragma once

namespace DLS {

struct ArenaAllocator {
    struct Entry {
        void* ptr;
        void(*deleteFn)(void*);
    };
    std::vector<Entry> objects;
    // ----------------------------------------------------
    template <typename T>
    void Add(T* obj) {
        objects.push_back({
            static_cast<void*>(obj), [](void* p) {
                delete static_cast<T*>(p);
            }
        });
    }
    // ----------------------------------------------------
    // Handle with care!!
    void Clear() {
        for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
            if (it->ptr) {
                it->deleteFn(it->ptr);
            }
        }
        objects.clear();
    }
    // ----------------------------------------------------
    ~ArenaAllocator() {
        Clear();
    }
};
inline ArenaAllocator GarbageList;

} //namespace
