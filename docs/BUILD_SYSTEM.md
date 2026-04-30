# RobotOS Build System Architecture

> **Mục đích**: Giải thích chiến lược "2 tầng build" để contributor dễ đóng góp mà không cần hiểu sâu GN/Ninja

---

## 🎯 Tổng quan

RobotOS sử dụng **dual build system strategy**:

```
┌─────────────────────────────────────────────────────┐
│                  CONTRIBUTOR VIEW                    │
│                                                      │
│  ┌──────────────────────────────────────────────┐  │
│  │         CMake (Development)                   │  │
│  │  • Middleware/libs development                │  │
│  │  • Host unit tests (fast iteration)           │  │
│  │  • IDE integration (CLion/VS Code)            │  │
│  │  • CI: test trên PC (Linux/macOS/Windows)    │  │
│  └──────────────────────────────────────────────┘  │
│                                                      │
└─────────────────────────────────────────────────────┘
                         ↓
              (Auto-sync via script)
                         ↓
┌─────────────────────────────────────────────────────┐
│                  MAINTAINER VIEW                     │
│                                                      │
│  ┌──────────────────────────────────────────────┐  │
│  │      GN/Ninja (Firmware Integration)          │  │
│  │  • Kernel + profile assembly                  │  │
│  │  • Cross-compilation cho MCU                  │  │
│  │  • Linker scripts & memory layout             │  │
│  │  • CLI wrapper: ro build/flash                │  │
│  └──────────────────────────────────────────────┘  │
│                                                      │
└─────────────────────────────────────────────────────┘
```

**Nguyên tắc thiết kế**:
- Contributor **chỉ cần biết CMake**
- GN/Ninja là "implementation detail" được CLI wrapper che giấu
- Sync CMake ↔ GN được tự động hóa

---

## 📂 Cấu trúc File Build

```
robotos-inspire/
├─ CMakeLists.txt                    # Root CMake (host tests)
├─ BUILD.gn                          # Root GN (firmware)
├─ build.py                          # CLI wrapper (gọi gn/ninja)
├─ tools/
│  └─ sync_build.py                  # Auto-sync CMake → GN
│
├─ build/                            # Build configs & templates
│  ├─ ro_component.gni               # GN template cho modules
│  ├─ ro_config.gni                  # Common configs
│  ├─ toolchain/
│  │  ├─ arm_gcc.gni                 # ARM GCC toolchain
│  │  └─ host.gni                    # Host toolchain (test)
│  └─ cmake/
│     ├─ RoComponent.cmake           # CMake template
│     └─ RoConfig.cmake              # Common configs
│
├─ middleware/
│  └─ ro_core/
│     ├─ CMakeLists.txt              # ← Contributor chỉ edit file này
│     ├─ BUILD.gn                    # ← Auto-generated hoặc manual sync
│     ├─ src/
│     │  ├─ ro_node.c
│     │  └─ ro_pubsub.c
│     └─ include/
│        └─ robotos/
│           ├─ ro_node.h
│           └─ ro_pubsub.h
│
└─ platforms/
   └─ stm32f4/
      ├─ BUILD.gn                    # Platform-specific (maintainer only)
      └─ platform.cmake              # Platform stub for host tests
```

---

## 🔨 CMake Build (Development & CI)

### Mục đích
- **Fast iteration**: Compile + test trên PC trong < 10s
- **IDE support**: CLion, VS Code CMake Tools
- **CI friendly**: GitHub Actions chạy trên `ubuntu-latest`
- **Cross-platform**: Linux, macOS, Windows

### Workflow

```bash
# Developer workflow (typical)
cd robotos-inspire/

# Configure
cmake -B build_host -DROBOTOS_BUILD_TESTS=ON

# Build
cmake --build build_host -j8

# Test
cd build_host && ctest --output-on-failure

# Specific test
./build_host/middleware/ro_core/test_pubsub
```

### CMake Module Template

Contributor tạo module mới chỉ cần:

```cmake
# middleware/ro_control/CMakeLists.txt

ro_add_library(ro_control
  SOURCES
    src/ro_pid.c
    src/ro_filter.c
  PUBLIC_HEADERS
    include/robotos/ro_pid.h
    include/robotos/ro_filter.h
  DEPENDENCIES
    ro_core
  TESTS
    test/test_pid.c
    test/test_filter.c
)
```

Template `ro_add_library()` (trong `build/cmake/RoComponent.cmake`) sẽ tự động:
- Setup include paths
- Link dependencies
- Generate test executable nếu có
- Export targets cho downstream

### Testing

```cmake
# Host tests configuration
if(ROBOTOS_BUILD_TESTS)
  enable_testing()
  
  # Optional: Unity framework cho embedded-style tests
  add_subdirectory(third_party/unity)
  
  ro_add_test(test_pubsub
    SOURCES test/test_pubsub.c
    LINK_LIBRARIES ro_core unity
  )
endif()
```

Test chạy trên host với:
- Mock hardware (GPIO, UART, Timer)
- Real middleware logic
- Fast feedback (< 1s per test)

---

## ⚙️ GN Build (Firmware Integration)

### Mục đích
- **Kernel integration**: Link với LiteOS-M kernel
- **Profile assembly**: Combine kernel + middleware + platform
- **Memory layout**: Linker scripts cho từng MCU
- **Optimization**: Firmware size, cross-compile flags

### Workflow

Maintainer (hoặc CLI):

```bash
# Via CLI wrapper (hides GN complexity)
python build.py --platform stm32f4 --profile robotos_mcu_minimal

# Under the hood:
# 1. gn gen out/stm32f4 --args='target_cpu="arm" ...'
# 2. ninja -C out/stm32f4
```

Contributor **không cần chạy** GN manually.

### GN Module Template

File `BUILD.gn` cho module:

```gn
# middleware/ro_core/BUILD.gn

import("//build/ro_component.gni")

ro_static_lib("ro_core") {
  sources = [
    "src/ro_node.c",
    "src/ro_pubsub.c",
    "src/ro_timer.c",
    "src/ro_executor.c",
  ]
  
  public = [
    "include/robotos/ro_node.h",
    "include/robotos/ro_pubsub.h",
    "include/robotos/ro_timer.h",
    "include/robotos/ro_executor.h",
  ]
  
  deps = [
    "//third_party/kernel_liteos_m:kernel",
  ]
  
  # Optional: specify realtime constraints
  cflags = [ "-DROBOTOS_REALTIME_MODULE" ]
}
```

### GN Template Wrapper

File `build/ro_component.gni`:

```gn
# Simplified template to hide GN complexity
template("ro_static_lib") {
  assert(defined(invoker.sources), "sources is required")
  assert(defined(invoker.public), "public headers required")
  
  static_library(target_name) {
    forward_variables_from(invoker, "*", ["public"])
    
    # Auto-setup include dirs
    include_dirs = [ "include" ]
    
    # Auto-apply common configs
    configs += [ "//build:ro_common_config" ]
    
    # Metadata for tooling
    metadata = {
      ro_module_type = [ "middleware" ]
      ro_headers = invoker.public
    }
  }
}
```

90% modules chỉ cần điền vào `ro_static_lib()`, không cần học full GN syntax.

---

## 🔄 Sync Strategy (CMake ↔ GN)

### Vấn đề
- Contributor edit `CMakeLists.txt` → thêm file mới
- `BUILD.gn` cũng cần update → dễ quên, gây build break

### Giải pháp

**Option 1: Auto-generation** (Recommended v0.1)

```bash
# tools/sync_build.py
python tools/sync_build.py --check  # CI runs this

# Parse all CMakeLists.txt
# Generate BUILD.gn for each module
# Fail if manual BUILD.gn differs
```

**Option 2: Manual với CI check** (Fallback)

```yaml
# .github/workflows/ci.yml
- name: Check build files sync
  run: |
    python tools/sync_build.py --verify
    # Fails if BUILD.gn missing sources from CMakeLists.txt
```

**Option 3: Single source of truth** (Future)

```python
# ro_module.yaml (declarative)
name: ro_core
sources:
  - src/ro_node.c
  - src/ro_pubsub.c
headers:
  - include/robotos/ro_node.h
deps:
  - ro_memory

# Generate both CMakeLists.txt AND BUILD.gn
```

### Sync Script Behavior

```bash
$ python tools/sync_build.py --module middleware/ro_core

✓ Parsing CMakeLists.txt...
  Found sources: ro_node.c, ro_pubsub.c, ro_timer.c
  Found headers: ro_node.h, ro_pubsub.h, ro_timer.h
  Found deps: kernel_liteos_m

✓ Generating BUILD.gn...
  Written to middleware/ro_core/BUILD.gn

✓ Verification...
  CMake sources == GN sources ✓
  CMake deps ⊆ GN deps ✓
```

---

## 🏗️ Build Configurations

### CMake Configurations

```bash
# Debug build (default)
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release

# With sanitizers (host only)
cmake -B build -DROBOTOS_SANITIZERS=address,undefined

# Cross-compile stub (future)
cmake -B build -DROBOTOS_PLATFORM=stm32f4 -DCMAKE_TOOLCHAIN_FILE=...
```

### GN Configurations (via profiles)

```bash
# Minimal profile (v0.1)
python build.py --profile robotos_mcu_minimal

# With trace enabled
python build.py --profile robotos_mcu_minimal --trace

# Debug build
python build.py --profile robotos_mcu_minimal --debug

# Size-optimized
python build.py --profile robotos_mcu_minimal --optimize=size
```

Profile file example:

```gn
# profiles/robotos_mcu_minimal/args.gn
target_cpu = "arm"
target_os = "liteos_m"

# Features
robotos_enable_trace = true
robotos_enable_params = true
robotos_enable_uart_transport = false  # v0.1

# Kernel config
liteos_m_config_task_max = 16
liteos_m_config_queue_max = 8
```

---

## 🚀 CLI Wrapper (`ro` command)

### Purpose
Hide GN/Ninja complexity behind friendly CLI.

### Usage

```bash
# Build
ro build --platform stm32f4 --profile minimal

# Flash (wrapper for OpenOCD/esptool)
ro flash --platform stm32f4 --port /dev/ttyUSB0

# Monitor
ro monitor --port /dev/ttyUSB0 --baudrate 115200

# Clean
ro clean

# Advanced: show underlying commands
ro build --platform stm32f4 --verbose
# → gn gen out/stm32f4 --args='...'
# → ninja -C out/stm32f4
```

### Implementation Sketch

```python
# tools/ro_cli/commands/build.py

def cmd_build(args):
    """Build firmware image"""
    
    # 1. Validate platform
    if args.platform not in SUPPORTED_PLATFORMS:
        error(f"Unknown platform: {args.platform}")
    
    # 2. Sync build files
    run("python tools/sync_build.py --check")
    
    # 3. Generate GN args
    gn_args = generate_gn_args(args.platform, args.profile)
    
    # 4. Run GN
    out_dir = f"out/{args.platform}"
    run(f"gn gen {out_dir} --args='{gn_args}'")
    
    # 5. Run Ninja
    run(f"ninja -C {out_dir} -j{args.jobs}")
    
    # 6. Show binary info
    show_binary_info(f"{out_dir}/robotos.elf")
```

---

## 📊 CI/CD Integration

### GitHub Actions Workflow

```yaml
# .github/workflows/ci.yml

name: CI

on: [push, pull_request]

jobs:
  # ===== CMake build (fast, every PR) =====
  host-tests:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]
    steps:
      - uses: actions/checkout@v3
        with:
          submodules: recursive
      
      - name: Configure CMake
        run: cmake -B build -DROBOTOS_BUILD_TESTS=ON
      
      - name: Build
        run: cmake --build build -j4
      
      - name: Test
        run: cd build && ctest --output-on-failure
  
  # ===== Lint & format =====
  lint:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Check clang-format
        run: |
          find . -name "*.c" -o -name "*.h" | xargs clang-format --dry-run --Werror
      
      - name: Check build sync
        run: python tools/sync_build.py --verify
  
  # ===== GN build (slow, nightly or on-demand) =====
  firmware-build:
    runs-on: ubuntu-latest
    if: github.event_name == 'schedule' || contains(github.event.head_commit.message, '[firmware]')
    steps:
      - uses: actions/checkout@v3
        with:
          submodules: recursive
      
      - name: Install ARM GCC
        run: |
          sudo apt-get update
          sudo apt-get install -y gcc-arm-none-eabi
      
      - name: Install GN/Ninja
        run: |
          # Download prebuilt GN
          wget https://chrome-infra-packages.appspot.com/.../gn
          wget https://github.com/ninja-build/ninja/releases/.../ninja
      
      - name: Build firmware
        run: python build.py --platform stm32f4 --profile robotos_mcu_minimal
      
      - name: Upload artifact
        uses: actions/upload-artifact@v3
        with:
          name: robotos-stm32f4.elf
          path: out/stm32f4/robotos.elf
```

### Benchmark Workflow

```yaml
# .github/workflows/benchmark.yml
name: Benchmark

on:
  pull_request:
    paths:
      - 'middleware/**'
      - 'libs/**'

jobs:
  benchmark:
    runs-on: ubuntu-latest
    steps:
      - name: Run benchmarks
        run: |
          cmake --build build --target bench_all
          ./build/bench/bench_sched --format=json > bench_results.json
      
      - name: Compare with baseline
        run: python tools/compare_bench.py baseline.json bench_results.json
      
      - name: Comment on PR
        uses: actions/github-script@v6
        with:
          script: |
            // Post benchmark results as PR comment
```

---

## 🎓 Contributor Guide

### Thêm Module Mới

**Step 1**: Tạo structure

```bash
mkdir -p middleware/ro_new_module/{src,include/robotos,test}
```

**Step 2**: Viết `CMakeLists.txt`

```cmake
# middleware/ro_new_module/CMakeLists.txt

ro_add_library(ro_new_module
  SOURCES
    src/ro_feature.c
  PUBLIC_HEADERS
    include/robotos/ro_feature.h
  DEPENDENCIES
    ro_core
  TESTS
    test/test_feature.c
)
```

**Step 3**: Implement & test

```bash
# Build
cmake --build build

# Test
./build/middleware/ro_new_module/test_feature
```

**Step 4**: Sync to GN (optional, CI sẽ check)

```bash
python tools/sync_build.py --module middleware/ro_new_module
```

**Step 5**: PR

```bash
git add middleware/ro_new_module
git commit -m "Add ro_new_module: <description>"
# CI sẽ tự động:
# - Build CMake
# - Run tests
# - Verify GN sync
```

### Thêm Source File vào Module Có Sẵn

**Step 1**: Edit `CMakeLists.txt`

```cmake
ro_add_library(ro_core
  SOURCES
    src/ro_node.c
    src/ro_pubsub.c
+   src/ro_new_feature.c  # ← Add this
  ...
)
```

**Step 2**: Implement

```c
// middleware/ro_core/src/ro_new_feature.c
#include "robotos/ro_new_feature.h"
// ...
```

**Step 3**: Test

```bash
cmake --build build
./build/middleware/ro_core/test_core
```

**Step 4**: Commit (CI verify sync)

---

## 🐛 Troubleshooting

### CMake build fails

```bash
# Clean rebuild
rm -rf build/
cmake -B build -DROBOTOS_BUILD_TESTS=ON
cmake --build build -j8
```

### GN sync error

```
ERROR: BUILD.gn missing sources: src/ro_new_feature.c
```

**Fix**:

```bash
python tools/sync_build.py --module middleware/ro_core
# Or manually edit BUILD.gn
```

### Linker error on firmware build

```
undefined reference to `ro_feature_init`
```

**Probable cause**: Function declared but not implemented, or missing in `BUILD.gn`

**Fix**:
1. Check function implementation exists
2. Check `BUILD.gn` includes the `.c` file
3. Check dependency in `deps = [...]`

### CI fails but local build OK

**Common causes**:
- Forgot to commit `BUILD.gn` changes
- Platform-specific code without guards
- Test depends on timing (flaky test)

**Fix**:
```bash
# Run CI locally with Act
act -j host-tests

# Or Docker
docker run --rm -v $(pwd):/work -w /work ubuntu:latest bash -c "
  apt-get update && apt-get install -y cmake build-essential
  cmake -B build && cmake --build build && cd build && ctest
"
```

---

## 📚 References

### GN Documentation
- [GN Quick Start](https://gn.googlesource.com/gn/+/main/docs/quick_start.md)
- [GN Language Reference](https://gn.googlesource.com/gn/+/main/docs/language.md)
- [OpenHarmony Build System](https://gitee.com/openharmony/docs/blob/master/en/device-dev/subsystems/subsys-build-all.md)

### CMake Documentation
- [Modern CMake](https://cliutils.gitlab.io/modern-cmake/)
- [CMake Examples](https://github.com/ttroy50/cmake-examples)

### Embedded Build Systems
- [Zephyr Build System](https://docs.zephyrproject.org/latest/build/index.html) (CMake-based)
- [micro-ROS Build](https://micro.ros.org/docs/tutorials/core/first_application_linux/) (CMake wrapper)

---

## 🔮 Future Improvements

### V0.2+
- [ ] Full cross-compile support via CMake (không cần GN cho simple cases)
- [ ] Bazel support (optional, nếu community cần)
- [ ] PlatformIO integration (`platform = robotos`)

### V1.0
- [ ] Web-based build configurator (generate profile GN args)
- [ ] Docker images với toolchains pre-installed
- [ ] GitHub Codespaces config

---

**Tóm lại**:
- **Developer** dùng CMake, test nhanh trên host
- **Maintainer** dùng GN, build firmware cuối cùng
- **CI** chạy cả 2, đảm bảo sync
- **Contributor** không cần lo GN, chỉ viết code + test
