//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Tools, Log
//-----------------------------------------------------------------------------
#pragma once
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctime>
#include <chrono>

namespace DLS::Tools{
    // ------------------------------------------------------------------
    enum class LogLevel {
        normal = 0
        ,error
        ,warning
        ,info
        ,debug
    };

    typedef void(*LogCallback)(LogLevel level, const char* message);
    // -------------------------------------------------------------------------
    static void DefaultLogger(LogLevel level, const char* message){
         switch (level) {
             case LogLevel::warning: ::printf("[warn] %s",  message); break;
             case LogLevel::error:   ::printf("[error] %s",  message); break;
             case LogLevel::info:    ::printf("[info] %s",  message); break;
             default:                ::printf("%s",  message); break;
         }
     }

   inline LogCallback LogHandler = DefaultLogger;

   inline  void raw_printf(LogLevel level, const char* fmt, va_list argptr)
     {
         if (!LogHandler) return;
         char buffer[8192] = {};
         uint32_t offset = 0;
         vsnprintf(buffer + offset, sizeof(buffer) - offset, fmt, argptr);

         LogHandler(level,buffer);

     }

    inline void printf(const char* format, ...)
     {
         va_list args;
         va_start(args, format);
         raw_printf(LogLevel::normal, format, args);
         va_end(args);
     }

   inline  void warnf(const char* format, ...)
     {
         va_list args;
         va_start(args, format);
         raw_printf(LogLevel::warning,format, args);
         va_end(args);
     }

   inline  void infof(const char* format, ...)
     {
         va_list args;
         va_start(args, format);
         raw_printf(LogLevel::info,format, args);
         va_end(args);
     }

   inline void errorf(const char* format, ...)
     {
         va_list args;
         va_start(args, format);
         raw_printf(LogLevel::error,format, args);
         va_end(args);
     }

   inline void printSeparator(int length) {
         const char* dashes = "----------------------------------------------------------------------------------------------------";
         if (length > 100) length = 100;
         if (length < 0) length = 0;

         printf("%.*s\n", length, dashes);
     }


    inline void sleep(unsigned int ms){
         #if defined(_WIN32)
         ::Sleep(ms);
         #else
         struct timespec ts;
         ts.tv_sec = ms / 1000;
         ts.tv_nsec = (ms % 1000) * 1000000;
         nanosleep(&ts, NULL);
         #endif
     }

    inline double time(){
         auto now = std::chrono::system_clock::now();
         auto duration = now.time_since_epoch();
         return std::chrono::duration<double>(duration).count();
     }

    inline uint64_t time_seconds() {
         auto now = std::chrono::system_clock::now();
         auto duration = now.time_since_epoch();
         return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    }

}
