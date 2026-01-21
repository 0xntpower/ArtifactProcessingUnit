#pragma once

// Windows
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// C standard
#include <cstdint>
#include <cstddef>
#include <cctype>

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