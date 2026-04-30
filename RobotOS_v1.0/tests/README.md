# tests/

Unit tests chạy trên **host (PC)** — không cần phần cứng, không cần Zephyr.

Dùng host adapter stubs (`src/adapter/host/`) và CMake + CTest.

---

## Danh sách test (4 test files)

| File | Test gì | Số test cases |
|------|---------|--------------|
| `test_gcode_parser.c` | `gcode_parse_line()` — valid/invalid lines, edge cases | ~15 cases |
| `test_motion_planner.c` | `motion_planner_tick()` — seg generation, queue interaction | ~10 cases |
| `test_kinematics_cartesian.c` | mm→steps conversion, multi-axis, profile parameters | ~8 cases |
| `test_app_sm.c` | State transitions, invalid events, ESTOP behaviour | ~12 cases |
| `CMakeLists.txt` | Build config cho tất cả tests | — |

---

## Build & Run

```bash
# Từ thư mục gốc RobotOS_v1.0/
mkdir build && cd build
cmake -DHOST_BUILD=ON ..
cmake --build .

# Chạy tất cả tests
ctest --output-on-failure -V

# Chạy một test cụ thể
./test_gcode_parser
./test_app_sm
```

### Output mẫu

```
[==========] Running 15 tests from 1 test suite.
[----------] 15 tests from GcodeParser
[ RUN      ] GcodeParser.G0_Basic
[       OK ] GcodeParser.G0_Basic (0 ms)
[ RUN      ] GcodeParser.G1_WithFeedrate
[       OK ] GcodeParser.G1_WithFeedrate (0 ms)
...
[  PASSED  ] 15 tests.
```

---

## Thêm test mới

1. Tạo file `test_<module>.c` trong `tests/`.
2. Thêm vào `tests/CMakeLists.txt`:
   ```cmake
   add_executable(test_mymodule test_mymodule.c)
   target_link_libraries(test_mymodule robotos_host)
   add_test(NAME test_mymodule COMMAND test_mymodule)
   ```
3. Chạy `ctest` để verify.

---

## Quy ước viết test

```c
// test_gcode_parser.c
#include <assert.h>
#include <stdio.h>
#include <app/gcode_parser.h>

static void test_g0_basic(void) {
    gcode_cmd_t cmd;
    int r = gcode_parse_line("G0 X10 Y20", &cmd);
    assert(r == 0);
    assert(cmd.code == 0);
    assert(cmd.x == 10.0f);
    printf("[PASS] test_g0_basic\n");
}

int main(void) {
    test_g0_basic();
    // ...
    return 0;
}
```
