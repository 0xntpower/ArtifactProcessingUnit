# Code Conventions

This document outlines the coding standards and conventions used in this codebase to ensure consistency and maintainability.

## Naming Conventions

### Variables

- **Local variables**: `camelCase`
  ```cpp
  int eventCount = 0;
  std::string processName;
  ```

- **Member variables**: `camelCase_` with trailing underscore
  ```cpp
  class Example {
      std::mutex mutex_;
      EventRouter* eventRouter_;
  };
  ```

- **Constants**: `kPascalCase` with `k` prefix
  ```cpp
  constexpr size_t kMaxFileNameLength = 80;
  constexpr char kDefaultProcessName[] = "unknown";
  ```

- **Function parameters**: `camelCase`
  ```cpp
  void ProcessEvent(const Event& event, uint32_t processId);
  ```

### Functions and Methods

- **Functions/Methods**: `PascalCase`
  ```cpp
  void StartEventLoop();
  bool CanHandle(const GUID& providerId);
  ```

- **Private helper functions**: Use anonymous namespace
  ```cpp
  namespace {
      bool IsValidString(const std::string& str) { ... }
  }
  ```

### Classes and Types

- **Classes**: `PascalCase`
  ```cpp
  class EventRouter;
  class JsonPerProcessSink;
  ```

- **Interfaces**: `IPascalCase` with `I` prefix
  ```cpp
  struct IEventDecoder;
  struct ISink;
  struct IEventSource;
  ```

- **Structs (data-only)**: `PascalCase`
  ```cpp
  struct Event;
  struct ProcessBuffer;
  ```

- **Enums**: `PascalCase` for enum name, `PascalCase` for values
  ```cpp
  enum class ProcessOpcode : USHORT {
      Start = 1,
      Stop = 2,
      DCStart = 11
  };
  ```

### Namespaces

- **All lowercase** with `::` separator
  ```cpp
  namespace agent::core { }
  namespace agent::sinks { }
  namespace agent::decoders { }
  namespace agent::util { }
  ```

### Files

- **Header files**: `PascalCase.hpp`
  ```
  EventRouter.hpp
  IEventDecoder.hpp
  ```

- **Implementation files**: `PascalCase.cpp`
  ```
  EventRouter.cpp
  TdhUtil.cpp
  ```

- **One class per file** (with matching names)

## Code Organization

### Namespace Structure

```
agent/
├── core/           # Core interfaces and infrastructure
│   ├── IEventDecoder
│   ├── ISink
│   ├── IEventSource
│   ├── Event
│   ├── EventRouter
│   └── DecoderRegistry
├── sources/        # Event source implementations
│   └── EtwSource
├── sinks/          # Event sink implementations
│   ├── JsonPerProcessSink
│   └── ProcessTracker
├── decoders/       # Event decoder implementations
│   └── KernelProcessDecoder
└── util/           # Utility functions
    ├── TdhUtil
    └── GuidUtil
```

### File Structure

**Header files (`.hpp`)**:
```cpp
#pragma once

// System includes
#include <vector>
#include <memory>

// Windows includes (if needed)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

// Project includes
#include "Event.hpp"

namespace agent::core {

class ClassName {
public:
    // Public interface

private:
    // Private members
};

} // namespace agent::core
```

**Implementation files (`.cpp`)**:
```cpp
#include "ClassName.hpp"

// Additional includes
#include <algorithm>

namespace agent::core {

namespace {
    // Anonymous namespace for internal helpers
    constexpr int kInternalConstant = 42;
    
    bool HelperFunction() { ... }
}

// Class implementation

} // namespace agent::core
```

## Modern C++ Practices

### Use Modern C++ Features

- **`constexpr`** for compile-time constants
  ```cpp
  constexpr ULONG kBufferSize = 2048;
  ```

- **`nullptr`** instead of `NULL` or `0`
  ```cpp
  if (decoder != nullptr) { ... }
  ```

- **Range-based for loops**
  ```cpp
  for (const auto& event : events) { ... }
  ```

- **Structured bindings** (C++17)
  ```cpp
  for (const auto& [pid, buffer] : processBuffers_) { ... }
  ```

- **`auto`** for type inference (when type is obvious)
  ```cpp
  auto& buffer = processBuffers_[processId];
  ```

- **Smart pointers** over raw pointers
  ```cpp
  std::unique_ptr<IEventDecoder> decoder;
  std::shared_ptr<ISink> sink;
  ```

### RAII and Resource Management

- Always use RAII for resource management
- Prefer smart pointers over manual `new`/`delete`
- Use `std::lock_guard` for mutex locking
- Implement proper destructors for cleanup

```cpp
class Resource {
public:
    Resource() { Initialize(); }
    ~Resource() { Cleanup(); }
    
    // Delete copy operations
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
};
```

### Error Handling

- Use `std::optional` for operations that may fail
  ```cpp
  std::optional<Event> Decode(const EVENT_RECORD& eventRecord);
  ```

- Return `bool` for simple success/failure
  ```cpp
  bool TryGetString(const EVENT_RECORD& rec, std::string& out);
  ```

- Use exceptions for exceptional circumstances only

### Const Correctness

- Mark methods `const` if they don't modify state
  ```cpp
  bool CanHandle(const GUID& providerId) const;
  ```

- Use `const` references for parameters
  ```cpp
  void OnEvent(const Event& event);
  ```

- Use `const` for variables that shouldn't change
  ```cpp
  const std::string fileName = GetFileName();
  ```

## Code Style

### Braces and Indentation

- **4 spaces** for indentation (no tabs)
- **Opening brace** on same line for functions/classes
- **Closing brace** on its own line

```cpp
void Function() {
    if (condition) {
        DoSomething();
    } else {
        DoSomethingElse();
    }
}
```

### Comments

- **Document public APIs** with clear comments
- **Explain "why"**, not "what"
- Use `//` for single-line comments
- Use `/* */` for multi-line comments sparingly

```cpp
// Extract process name from image path for human-readable JSON filenames
std::string ExtractProcessNameFromImagePath(const std::string& imagePath) const;
```

### Line Length

- Prefer **100-120 characters** maximum
- Break long parameter lists across multiple lines

```cpp
EtwSource::EtwSource(core::EventRouter* eventRouter, 
                     core::DecoderRegistry* decoderRegistry)
    : eventRouter_(eventRouter), 
      decoderRegistry_(decoderRegistry) {}
```

### Include Order

1. Corresponding header (for `.cpp` files)
2. C system headers
3. C++ standard library headers
4. Windows headers
5. Third-party library headers
6. Project headers

```cpp
#include "MyClass.hpp"

#include <cstdint>
#include <vector>
#include <memory>

#include <Windows.h>

#include "third-party/json.hpp"

#include "Event.hpp"
#include "EventRouter.hpp"
```

## Windows-Specific Conventions

### Macros

Always define before Windows includes:
```cpp
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
```

### Pragma Comments

Use for linking libraries:
```cpp
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "tdh.lib")
```

### GUIDs

Define as constants with descriptive names:
```cpp
constexpr GUID kClassicProcessProviderGuid = {
    0x3d6fa8d0, 0xfe05, 0x11d0, 
    { 0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c }
};
```

## Testing and Debugging

### Debug Output

Use `std::fprintf(stderr, ...)` for debug output:
```cpp
std::fprintf(stderr, "[OnEventRecord] Provider=%s Id=%u\n", 
            guidString.c_str(), eventId);
```

### Assertions

Use static_assert for compile-time checks:
```cpp
static_assert(sizeof(wchar_t) == 2, "Wide char must be 2 bytes");
```

## Contributing Guidelines

1. **Follow all conventions** outlined in this document
2. **One logical change per commit** with clear commit messages
3. **Test your changes** thoroughly before submitting
4. **Document new public APIs** with comments
5. **Update this document** if introducing new conventions
6. **Keep interfaces stable** - changes to interfaces affect all implementations
7. **Use meaningful names** - code should be self-documenting