//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// StructArena
//-----------------------------------------------------------------------------
#pragma once
#include <vector>
#include <cinttypes>
#include "Marshall.h"

namespace DLS {

    struct StructArena {
        struct Entry {
            void* mPtr;
            void(*deleteFn)(void*);
            Marshall::IStructTranslator* mTranslator;
        };
        std::vector<Entry> objects;
        // ----------------------------------------------------
        template <typename T>
        size_t Add(T* obj, Marshall::IStructTranslator* translator) {
            objects.push_back({
                static_cast<void*>(obj), [](void* p) {
                    delete static_cast<T*>(p);
                }
                , translator
            });
            return objects.size() - 1;
        }
        // ----------------------------------------------------
        // Handle with care!!
        void Clear() {
            for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
                if (it->mPtr) {
                    it->deleteFn(it->mPtr);
                }
            }
            objects.clear();
        }
        // ----------------------------------------------------
        ~StructArena() {
            Clear();
        }
    };


} //namespace
