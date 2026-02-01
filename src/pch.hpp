#pragma once

// Windows
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

// C standard
#include <cstdint>
#include <cstddef>
#include <cctype>
#include <cerrno>

// STL containers
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <unordered_map>

// STL utilities
#include <memory>
#include <optional>
#include <functional>
#include <algorithm>
#include <utility>

// STL I/O
#include <filesystem>
#include <fstream>

// STL concurrency
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>

// STL formatting
#include <format>
#include <stdexcept>

// Third-party
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <SQLiteCpp/SQLiteCpp.h>
#include <zip.h>
#include <blake3.h>
#include <pugixml.hpp>

// Library linking
#pragma comment(lib, "SQLiteCpp.lib")
#pragma comment(lib, "sqlite3.lib")
#pragma comment(lib, "blake3.lib")
#pragma comment(lib, "zip.lib")
#pragma comment(lib, "pugixml.lib")

// spdlog has different names for debug/release builds
#ifdef _DEBUG
#pragma comment(lib, "spdlogd.lib")
#else
#pragma comment(lib, "spdlog.lib")
#endif

// Note: zlib is a transitive dependency of libzip and will be linked automatically