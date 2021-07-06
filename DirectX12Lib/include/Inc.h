#pragma once

#define NOMINMAX

#include <stdio.h>
#include <tchar.h>
#include <stdint.h>
#include <assert.h>

#include <cctype>
#include <cwctype>
#include <locale>
#include <string>

#include <set>
#include <bitset>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <memory>
#include <map>
#include <queue>
#include <stack>

#include <sstream>
#include <fstream>
#include <iostream>

#include <numeric>
#include <functional>
#include <algorithm> 
#include <regex>
#include <random>

#include <mutex>
#include <atomic>
#include <future>
#include <thread>
#include <chrono>

#include <filesystem>

#if defined(_HAS_CXX17) && _HAS_CXX17
#include <variant>
#include <optional>
#include <any>
#endif _HAS_CXX17

using namespace std::chrono_literals;

#define USE_UTF8

typedef unsigned char byte_t;

#ifndef _WIN64
typedef int32_t intx_t;
typedef uint32_t uintx_t;
typedef int16_t intx_h;
typedef uint16_t uintx_h;
#else
#define BIT64
typedef int64_t intx_t;
typedef uint64_t uintx_t;
typedef int32_t intx_h;
typedef uint32_t uintx_h;
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>