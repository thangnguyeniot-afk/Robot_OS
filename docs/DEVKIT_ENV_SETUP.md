# RobotOS Devkit — Environment Setup Guide

**Target board:** STM32F411E-DISCO  
**Zephyr version:** v3.6.0  
**Host OS:** Windows 11  
**Status:** Validated end-to-end (blink + RTT LOG confirmed on hardware)

---

## Mục lục

1. [Tổng quan kiến trúc toolchain](#1-tổng-quan-kiến-trúc-toolchain)
2. [Prerequisites — cài trước khi bắt đầu](#2-prerequisites)
3. [Phase 0 — Kiểm tra cấu trúc repo](#3-phase-0--kiểm-tra-cấu-trúc-repo)
4. [Phase 1.1 — CMake + Ninja](#4-phase-11--cmake--ninja)
5. [Phase 1.2 — west (Zephyr meta-tool)](#5-phase-12--west-zephyr-meta-tool)
6. [Phase 1.3 — Zephyr SDK 0.17.0](#6-phase-13--zephyr-sdk-0170)
7. [Phase 1.4 — west init workspace](#7-phase-14--west-init-workspace)
8. [Phase 1.5 — west update (fetch modules)](#8-phase-15--west-update-fetch-modules)
9. [Phase 1.6 — Zephyr Python requirements](#9-phase-16--zephyr-python-requirements)
10. [Phase 2.1 — Build devkit firmware](#10-phase-21--build-devkit-firmware)
11. [Phase 2.2 — Flash to hardware](#11-phase-22--flash-to-hardware)
12. [RTT Log — đọc LOG qua STLink debug USB](#12-rtt-log--đọc-log-qua-stlink-debug-usb)
13. [VS Code integration — Cortex-Debug + RTT](#13-vs-code-integration--cortex-debug--rtt)
14. [Performance Tracing (tham khảo)](#14-performance-tracing-tham-khảo)
15. [PATH setup mỗi terminal mới](#15-path-setup-mỗi-terminal-mới)
16. [Checklist tổng kết](#16-checklist-tổng-kết)

---

## 1. Tổng quan kiến trúc toolchain

```
RobotOS repo (D:\Robot_OS\)
├── .west/config          ← west workspace root marker
├── RobotOS_v1.0/         ← main manifest project
│   ├── west.yml          ← pins Zephyr v3.6.0 + HAL modules
│   └── devkit/           ← devkit firmware (blink + LOG)
├── zephyr/               ← fetched by west (full Zephyr RTOS source)
├── modules/
│   ├── hal/stm32/        ← STM32 HAL (fetched by west)
│   ├── hal/cmsis/        ← ARM CMSIS (fetched by west)
│   └── debug/segger/     ← Segger RTT library (fetched by west)
└── build/                ← cmake build output (gitignored)
```

**Tool roles:**
| Tool | Role |
|------|------|
| `west` | Meta-tool: repo management, build, flash wrapper |
| `cmake` | Build system generator (generates ninja files) |
| `ninja` | Build executor |
| `python` | west + Zephyr scripts runtime |
| Zephyr SDK | Cross-compiler (arm-zephyr-eabi-gcc) + sysroots |
| openocd-xpack | Flash + debug server (RTT transport) |

---

## 2. Prerequisites

### 2.1 Python

Cần Python 3.10+. **Preferred cho team setup: Python 3.11 hoặc 3.12** với dedicated venv (xem Phase 1.6). Local machine đã test với Python 3.14.0.

Cài từ python.org, **tick "Add to PATH"** trong installer.

```powershell
python --version   # phải ra 3.x.x
pip --version
```

> **Issue gặp phải (Python 3.14):** `pip install` fail với "externally managed environment" (PEP 668). Local workaround: dùng `--break-system-packages`. Đây là workaround, không phải baseline khuyến nghị cho team — xem Phase 1.6 để setup venv đúng cách.

### 2.2 Git

Phải có git trong PATH (west dùng git để fetch).

```powershell
git --version
```

### 2.3 STM32 hardware check

Cắm board STM32F411E-DISCO qua USB (CN1 — USB Mini-B, chính là STLink debug port).  
Mở Device Manager kiểm tra:

- **Expected:** `STMicroelectronics STLink dongle` hoặc `STM32 STLink`
- **VID/PID:** `VID_0483&PID_3748` = STLink V1 descriptor (hardware limitation của board revision này)
- **Không có VCP:** PID_3748 KHÔNG có Virtual COM Port — đây là design của board, không phải lỗi driver. LOG không thể dùng UART → phải dùng RTT.

> **Issue gặp phải:** Device hiện "Unknown device". Fix: rút cắm lại USB vật lý → Windows tự apply đúng driver (oem62.inf). Không cần manual `pnputil /add-driver`.

---

## 3. Phase 0 — Kiểm tra cấu trúc repo

**Mục đích:** Xác nhận repo đúng cấu trúc trước khi bắt đầu bất kỳ bước nào.

```powershell
cd D:\Robot_OS
Get-ChildItem                         # kiểm tra các thư mục top-level
Get-Content RobotOS_v1.0\west.yml    # kiểm tra manifest content
```

**west.yml phải có:**

```yaml
manifest:
  self:
    path: RobotOS_v1.0    # ← CRITICAL: phải trỏ đúng thư mục chứa west.yml

  remotes:
    - name: zephyrproject-rtos
      url-base: https://github.com/zephyrproject-rtos

  projects:
    - name: zephyr
      remote: zephyrproject-rtos
      revision: v3.6.0
      import:
        name-allowlist:
            - cmsis
            - hal_stm32
            - hal_espressif
            - hal_nordic
            - segger        # ← cần cho RTT LOG
```

**Checks:**
- [ ] `self.path` = tên folder chứa west.yml (thường `RobotOS_v1.0`)
- [ ] `revision: v3.6.0` — không dùng `main` (unstable)
- [ ] `segger` trong allowlist — thiếu → `CONFIG_USE_SEGGER_RTT=y` sẽ fail build

> **Issue gặp phải:** Thiếu `segger` trong allowlist → build error `Segger module not found`. Thêm vào allowlist, rồi chạy lại `west update`.

> **Issue gặp phải (YAML):** Khi sửa west.yml, dễ bị merge 2 dòng thành 1 dòng nếu dùng replace tool. Ví dụ lỗi: `- hal_nordic            - segger` (2 item trên 1 dòng → YAML syntax error). Kiểm tra sau mỗi lần sửa bằng `Get-Content RobotOS_v1.0\west.yml`.

---

## 4. Phase 1.1 — CMake + Ninja

### CMake

Tải từ https://cmake.org/download/ → Windows installer.  
Tested: cmake 4.3.2 tại `C:\Program Files\CMake\bin\cmake.exe`.

```powershell
cmake --version   # phải ra cmake version 3.20+ (Zephyr requirement)
```

> **Issue:** CMake 4.x đôi khi không tự có trong PATH. Phải prepend manually:
> ```powershell
> $env:PATH = "C:\Program Files\CMake\bin;" + $env:PATH
> ```
> Mỗi terminal mới phải set lại (xem [Section 15](#15-path-setup-mỗi-terminal-mới)).

### Ninja

```powershell
winget install Ninja-build.Ninja
ninja --version   # sau khi refresh PATH
```

Path WinGet thường là:
```
C:\Users\<user>\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe\
```

> **Check:** `ninja --version` fail sau install WinGet là bình thường — WinGet updates User PATH nhưng terminal hiện tại chưa load. Fix: refresh PATH (xem Section 15) hoặc mở terminal mới.

---

## 5. Phase 1.2 — west (Zephyr meta-tool)

```powershell
pip install west
west --version   # phải ra 1.x.x (tested: 1.5.0)
```

> **Issue Python 3.14:** `pip install west` fail với "externally managed environment". Local workaround:
> ```powershell
> pip install west --break-system-packages
> ```
> Preferred: dùng venv (xem Phase 1.6). `--break-system-packages` là workaround, không phải baseline cho team.

---

## 6. Phase 1.3 — Zephyr SDK 0.17.0

**Tại sao phải đúng version:** Zephyr v3.6.0 yêu cầu SDK 0.16.x hoặc 0.17.x. SDK 0.17.0 đã test ổn.

### Download

Từ https://github.com/zephyrproject-rtos/sdk-ng/releases/tag/v0.17.0  
Chọn: `zephyr-sdk-0.17.0_windows-x86_64.7z`

### Cài đặt

Giải nén vào `C:\zephyr-sdk-0.17.0\`. Sau đó chạy setup:

```powershell
cd C:\zephyr-sdk-0.17.0
.\setup.cmd
```

Setup script làm 2 việc:
1. Đăng ký SDK vào Windows CMake registry
2. Install host tools (qemu, etc.)

### Kiểm tra

```powershell
# Kiểm tra GCC arm cross-compiler
C:\zephyr-sdk-0.17.0\arm-zephyr-eabi\bin\arm-zephyr-eabi-gcc --version
# Expected: arm-zephyr-eabi-gcc (Zephyr SDK 0.17.0) 12.2.0

# Kiểm tra CMake registry entry
Get-ItemProperty "HKCU:\SOFTWARE\Kitware\CMake\Packages\Zephyr-sdk*" -ErrorAction SilentlyContinue
```

**Environment variables cần set (mỗi terminal):**

```powershell
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = "C:\zephyr-sdk-0.17.0"
```

> **Issue:** Không set 2 biến này → cmake configure step báo lỗi `toolchain not found`. Phải set trước khi chạy `west build`.

---

## 7. Phase 1.4 — west init workspace

**Chỉ chạy 1 lần duy nhất** khi setup workspace từ đầu. Nếu `.west/` đã tồn tại thì skip.

> **⚠ Quan trọng về working directory:** Lệnh `west init -l RobotOS_v1.0` phải chạy từ `D:\Robot_OS` (workspace root), **không phải** từ `D:\Robot_OS\RobotOS_v1.0`. Chạy từ sai thư mục sẽ tạo `.west/` sai vị trí và tất cả `west` commands sau đó sẽ fail.

```powershell
cd D:\Robot_OS          # ← phải là workspace root, không phải RobotOS_v1.0
west init -l RobotOS_v1.0
```

Flag `-l` (local) = không clone gì thêm, chỉ đọc west.yml từ folder đã có sẵn.

**Kiểm tra:**

```powershell
Get-Content .west\config
```

Expected output:
```ini
[manifest]
path = RobotOS_v1.0
file = west.yml

[zephyr]
base = zephyr
```

> **Issue:** Nếu chạy `west init` từ sai thư mục hoặc thiếu flag `-l`, west sẽ cố clone Zephyr từ internet vào thư mục hiện tại. Fix: xóa `.west/` và chạy lại đúng lệnh.

---

## 8. Phase 1.5 — west update (fetch modules)

Fetch tất cả modules được định nghĩa trong west.yml. Cần internet. Lần đầu mất ~15–30 phút.

```powershell
$env:PATH = "C:\Program Files\CMake\bin;" + $env:PATH
west update
```

**Modules sẽ được fetch:**

| Module | Path | Revision |
|--------|------|----------|
| zephyr | `zephyr/` | v3.6.0 (468eb56cf24) |
| cmsis | `modules/hal/cmsis/` | 4b96cbb |
| hal_stm32 | `modules/hal/stm32/` | 60c9634f |
| hal_espressif | `modules/hal/espressif/` | 67fa60bd |
| hal_nordic | `modules/hal/nordic/` | dce8519 |
| segger | `modules/debug/segger/` | 9d01912 |

**Kiểm tra:**

```powershell
west list        # liệt kê tất cả modules và revision
Get-ChildItem zephyr\   # phải có hàng nghìn files
Get-ChildItem modules\debug\segger\   # phải tồn tại (cho RTT)
```

> **Issue:** `west update` lần đầu có thể fail giữa chừng do network timeout. Chạy lại `west update` — west sẽ resume từ chỗ thiếu, không fetch lại từ đầu.

> **Issue:** Thêm `segger` vào allowlist trong west.yml SAU KHI đã `west update` lần đầu → cần `west update` lại để fetch segger. Lần này chỉ fetch segger (nhanh).

---

## 9. Phase 1.6 — Zephyr Python requirements

Zephyr cần thêm các Python packages (pyelftools, west extensions, etc.).

**Preferred (team setup) — dùng virtual environment:**
```powershell
python -m venv D:\zephyr_venv
D:\zephyr_venv\Scripts\Activate.ps1
pip install west
pip install -r D:\Robot_OS\zephyr\scripts\requirements.txt
# Sau đó mỗi terminal mới: D:\zephyr_venv\Scripts\Activate.ps1
```

**Local workaround (Python 3.14, không dùng venv):**
```powershell
pip install -r zephyr\scripts\requirements.txt --break-system-packages
```

> **Lưu ý:** `--break-system-packages` là workaround cho Python 3.14 externally managed environment (PEP 668). Ổn cho local dev, nhưng không phải baseline cho team. Python 3.11/3.12 + venv cho kết quả ổn định hơn trên nhiều máy.

**Kiểm tra:**

```powershell
python -c "import elftools; print('pyelftools OK')"
python -c "import yaml; print('PyYAML OK')"
```

---

## 10. Phase 2.1 — Build devkit firmware

### Cấu trúc devkit

```
RobotOS_v1.0/devkit/
├── CMakeLists.txt     ← Zephyr app entry point
├── prj.conf           ← Kconfig (subsystem selection)
└── src/
    └── main.c         ← blink + LOG_INF
```

**CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(robotos_devkit)
target_sources(app PRIVATE src/main.c)
```

**prj.conf (RTT LOG, no UART):**
```ini
CONFIG_LOG=y
CONFIG_LOG_DEFAULT_LEVEL=4
CONFIG_LOG_BACKEND_UART=n
CONFIG_LOG_BACKEND_RTT=y
CONFIG_USE_SEGGER_RTT=y
CONFIG_GPIO=y
CONFIG_SHELL=n
CONFIG_SERIAL=n
CONFIG_UART_CONSOLE=n
CONFIG_TRACING=n
```

> **Tại sao không dùng UART:** Board STM32F411E-DISCO có STLink V1 (PID_3748) — không có VCP interface. Không thể đọc serial output qua USB. RTT (Real-Time Transfer) dùng chính kết nối SWD debug của STLink → không cần thêm cable.

> **⚠ Scope — devkit baseline only:** `prj.conf` này là minimal bring-up config cho devkit. `CONFIG_SERIAL=n` và `CONFIG_SHELL=n` phản ánh hardware limitation của STLink V1 trên board này. Nếu sau này thêm UART driver, shell, hoặc external USB-UART cable, các option này phải được re-evaluate — không copy trực tiếp vào production config.

### Build command

```powershell
cd D:\Robot_OS

# Set PATH + toolchain env
$env:PATH = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
$env:PATH = "C:\Program Files\CMake\bin;" + $env:PATH
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = "C:\zephyr-sdk-0.17.0"

# Build
west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
```

Flag `--pristine` = clean build (xóa cache). Dùng lần đầu hoặc khi đổi prj.conf.

**Expected output khi build thành công:**
```
[132/132] Linking C executable zephyr\zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:       27116 B       512 KB      5.17%
            SRAM:        7808 B       128 KB      5.96%
```

**Build artifacts tại:** `D:\Robot_OS\build\zephyr\`
- `zephyr.bin` — raw binary
- `zephyr.elf` — ELF với debug symbols (cần cho GDB + Cortex-Debug)
- `zephyr.hex` — Intel HEX (cần cho STM32_Programmer_CLI)

### Kiểm tra sau build

```powershell
# Xác nhận file tồn tại
Test-Path build\zephyr\zephyr.bin   # True
Test-Path build\zephyr\zephyr.elf   # True

# Xác nhận RTT control block tồn tại (address cần cho openocd)
C:\zephyr-sdk-0.17.0\arm-zephyr-eabi\bin\arm-zephyr-eabi-nm build\zephyr\zephyr.elf | Select-String "_SEGGER_RTT"
# Expected: 20000800 D _SEGGER_RTT
```

> **Issue:** Address của `_SEGGER_RTT` thay đổi nếu thêm/xóa global variables hoặc đổi RAM layout. Luôn re-check address sau mỗi clean build trước khi configure openocd.

---

## 11. Phase 2.2 — Flash to hardware

### Cài openocd-xpack

West flash với runner `openocd` cần openocd. Không dùng openocd chuẩn của MinGW/MSYS mà dùng bản xpack (hỗ trợ STLink V1 tốt hơn):

```powershell
winget install xpack-dev-tools.openocd-xpack
# Sau đó refresh PATH (xem Section 15)
openocd --version   # phải ra xpack-openocd 0.12.x
```

> **Tại sao xpack?** openocd 0.12.0-7 từ xpack-dev-tools đã test ổn với STLink V1 trên Windows. openocd package từ các nguồn khác (winget openocd.openocd, MinGW) có thể thiếu STLink driver hoặc missing libusb.

### Flash

```powershell
cd D:\Robot_OS
$env:PATH = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
west flash
```

**Expected output:**
```
-- west flash (openocd)
-- runners.openocd: Flashing file: D:/Robot_OS/build/zephyr/zephyr.hex
Open On-Chip Debugger ...
Info : STLINK V1J47S0 (API v1) VID:PID 0483:3748
Info : flash written and verified successfully
wrote 32768 bytes from file ... in 3.x seconds
```

> **Issue quan trọng:** `west flash` exit code = 1 dù flash thành công. Lý do: STLink V1 không hỗ trợ reset target qua openocd sau khi flash. openocd báo lỗi ở bước reset cuối nhưng firmware đã được ghi đúng.  
> **Fix:** Nhấn nút RESET trên board sau khi thấy `wrote 32768 bytes`. Firmware sẽ chạy.

> **Issue:** `libusb_open() LIBUSB_ERROR_ACCESS`. Xảy ra khi một process khác đang giữ USB handle của STLink (ví dụ: STLinkUpgrade.jar vẫn chạy ngầm). Fix: `Get-Process java | Stop-Process -Force`, rồi flash lại.

> **Issue:** STLink V1 không hỗ trợ STM32_Programmer_CLI qua SWD (`-c port=SWD` fail). Phải dùng openocd.

**Xác nhận firmware chạy:** LED cam (PD13) trên board phải nhấp nháy với chu kỳ 500ms + 500ms.

---

## 12. RTT Log — đọc LOG qua STLink debug USB

RTT (Real-Time Transfer) là cơ chế truyền data giữa MCU và host qua vùng nhớ RAM được share, đọc qua SWD debug connection. Không cần UART, không cần thêm cable.

### Cơ chế hoạt động

```
Zephyr LOG_INF() → RTT ring buffer (RAM 0x20000800) → SWD → STLink → USB → openocd RTT server → TCP port 9090 → client (terminal / VS Code)
```

### Tìm địa chỉ RTT control block

Sau mỗi clean build, confirm lại address:

```powershell
C:\zephyr-sdk-0.17.0\arm-zephyr-eabi\bin\arm-zephyr-eabi-nm build\zephyr\zephyr.elf | Select-String "_SEGGER_RTT"
# 20000800 D _SEGGER_RTT   ← lấy address này
```

### Chạy openocd RTT server

```powershell
$env:PATH = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
$openocd = (Get-Command openocd).Source

& $openocd `
  -s "D:/Robot_OS/zephyr/boards/arm/stm32f411e_disco/support" `
  -f openocd.cfg `
  -c "init" `
  -c "reset run" `
  -c "sleep 200" `
  -c "rtt setup 0x20000800 0x100 {SEGGER RTT}" `
  -c "rtt start" `
  -c "rtt server start 9090 0"
```

**Expected openocd output:**
```
Info : STLINK V1J47S0 (API v1)
Info : rtt: Control block found at 0x20000800
Info : Listening on port 9090 for rtt connections
```

> **Tại sao `reset run` thay vì `reset halt`?**  
> Với `reset halt`, CPU dừng ngay sau reset — Zephyr chưa chạy, `_SEGGER_RTT` control block chưa được khởi tạo trong RAM → `rtt setup` không tìm thấy control block.  
> `reset run` + `sleep 200` cho Zephyr chạy ~200ms để init xong rồi mới scan RTT.

### Đọc log qua TCP

Mở terminal mới:
```powershell
# Dùng PowerShell TCP client
$client = New-Object System.Net.Sockets.TcpClient("localhost", 9090)
$stream = $client.GetStream()
$reader = New-Object System.IO.StreamReader($stream)
while ($true) { $line = $reader.ReadLine(); if ($line) { Write-Host $line } }
```

Hoặc cài `ncat` / `nc`:
```
nc localhost 9090
```

**Expected output:**
```
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.000,000] <inf> devkit_main: RobotOS devkit starting — board: stm32f411e_disco
[00:00:00.000,000] <inf> devkit_main: LED blink loop starting
[00:00:00.000,000] <inf> devkit_main: tick
[00:00:00.500,000] <inf> devkit_main: tick
```

---

## 13. VS Code integration — Cortex-Debug + RTT

Cortex-Debug extension tích hợp openocd + GDB + RTT vào VS Code: có thể debug với breakpoint VÀ đọc LOG trong cùng một session.

### Cài extension

```powershell
code --install-extension marus25.cortex-debug
```

Hoặc trong VS Code: `Ctrl+Shift+X` → search `Cortex-Debug` (marus25).

### Settings

Trong `.vscode/settings.json`:

```json
"cortex-debug.openocdPath": "C:/Users/<user>/AppData/Local/Microsoft/WinGet/Packages/xpack-dev-tools.openocd-xpack_Microsoft.Winget.Source_8wekyb3d8bbwe/xpack-openocd-0.12.0-7/bin/openocd.exe",
"cortex-debug.gdbPath": "C:/zephyr-sdk-0.17.0/arm-zephyr-eabi/bin/arm-zephyr-eabi-gdb.exe"
```

> Thay `<user>` bằng Windows username thực tế.

Tìm đúng path openocd:
```powershell
(Get-Command openocd).Source
```

### Launch configuration

Trong `.vscode/launch.json` (đã có sẵn trong repo):

```json
{
  "name": "devkit: Debug + RTT (stm32f411e_disco)",
  "type": "cortex-debug",
  "request": "launch",
  "servertype": "openocd",
  "device": "STM32F411VE",
  "configFiles": [
    "D:/Robot_OS/zephyr/boards/arm/stm32f411e_disco/support/openocd.cfg"
  ],
  "openOCDLaunchCommands": ["init", "reset halt"],
  "executable": "D:/Robot_OS/build/zephyr/zephyr.elf",
  "cwd": "D:/Robot_OS",
  "runToEntryPoint": "main",
  "rttConfig": {
    "enabled": true,
    "address": "0x20000800",
    "decoders": [
      { "port": 0, "type": "console", "label": "Zephyr LOG" }
    ]
  }
}
```

> **⚠ RTT address — phải verify sau mỗi clean build trước khi launch Cortex-Debug:**  
> `0x20000800` là địa chỉ `_SEGGER_RTT` của firmware hiện tại. Address này thay đổi nếu có clean build với global variable mới hoặc thay đổi RAM layout. Quy trình bắt buộc:
> ```powershell
> C:\zephyr-sdk-0.17.0\arm-zephyr-eabi\bin\arm-zephyr-eabi-nm build\zephyr\zephyr.elf | Select-String "_SEGGER_RTT"
> ```
> Nếu address khác `0x20000800`, update field `"address"` trong `rttConfig` của `.vscode/launch.json` trước khi launch.

> **Lưu ý `svdFile`:** Nếu muốn xem peripheral registers trong Cortex-Debug, cần SVD file (`stm32f411.svd`). File này không có sẵn trong Zephyr source — download từ ST website hoặc bỏ dòng `svdFile` nếu không cần.

### Sử dụng

1. Đảm bảo không có openocd nào đang chạy: `Get-Process openocd | Stop-Process -Force`
2. `F5` hoặc `Run → Start Debugging` → chọn **"devkit: Debug + RTT"**
3. Tab **"Cortex-Debug: RTT: Zephyr LOG"** xuất hiện trong Terminal panel
4. Có thể đặt breakpoint trong `main.c`, step through code, xem variables

---

## 14. Performance Tracing (tham khảo)

Khi cần phân tích timing, thread scheduling, ISR latency:

### Segger SystemView

Hiển thị timeline của tasks, ISRs, kernel events — phù hợp với Zephyr RTOS.

**Thêm vào prj.conf:**
```ini
CONFIG_SEGGER_SYSTEMVIEW=y
CONFIG_TRACING=y
```

**Cần:** Segger SystemView desktop app (free: https://www.segger.com/products/development-tools/systemview/)  
Transport: RTT channel 1 (cùng kết nối STLink debug USB).

> **Lưu ý:** Zephyr v3.6.0 + Segger SystemView cần `modules/debug/segger` đã được fetch (Phase 1.5 có sẵn).

### Zephyr CTF Tracing (built-in, nhẹ hơn)

Không cần tool ngoài, decode bằng `babeltrace2` (open source).

**prj.conf:**
```ini
CONFIG_TRACING=y
CONFIG_TRACING_CTF=y
CONFIG_TRACING_BACKEND_CTF_RTT=y
```

Output qua RTT channel 1 → decode bằng:
```bash
babeltrace2 <ctf_trace_dir>
```

**Khuyến nghị:** Với RobotOS embedded target (STM32F411), Zephyr CTF đủ dùng cho phần lớn debug scenarios. SystemView có UI đẹp hơn nếu cần trace visual.

---

## 15. PATH setup mỗi terminal mới

Zephyr toolchain cần các biến môi trường. Windows không auto-load WinGet PATH vào terminal đang mở. Chạy block này ở đầu mỗi terminal session:

```powershell
# Reload PATH từ registry (bao gồm WinGet, SDK, ninja)
$env:PATH = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

# CMake (nếu chưa trong PATH)
$env:PATH = "C:\Program Files\CMake\bin;" + $env:PATH

# Zephyr toolchain
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = "C:\zephyr-sdk-0.17.0"

# Verify
cmake --version
ninja --version
west --version
openocd --version
```

> Có thể thêm block này vào PowerShell profile (`$PROFILE`) để tự động load, nhưng chú ý scope — chỉ set biến Zephyr khi làm việc với repo này.

---

## 16. Checklist tổng kết

> Items bên dưới đã được check trên **reference machine** (bring-up confirmed: LED blink ✅, RTT LOG ✅).  
> Khi setup máy mới, copy file này và **uncheck tất cả** — dùng như checklist trắng cho từng developer.

### Environment setup

- [x] Python 3.10+ trong PATH
- [x] Git trong PATH
- [x] CMake 3.20+ trong PATH
- [x] Ninja trong PATH
- [x] west 1.x installed
- [x] Zephyr SDK 0.17.0 tại `C:\zephyr-sdk-0.17.0\`, setup.cmd đã chạy
- [x] openocd-xpack 0.12.0+ installed (winget xpack-dev-tools.openocd-xpack)
- [x] VS Code + Cortex-Debug extension installed

### west workspace

- [x] `D:\Robot_OS\.west\config` tồn tại và đúng nội dung
- [x] `west list` hiện 6+ modules
- [x] `D:\Robot_OS\modules\debug\segger\` tồn tại
- [x] `D:\Robot_OS\zephyr\` tồn tại (hàng trăm MB)

### Build

- [x] `west build -b stm32f411e_disco RobotOS_v1.0/devkit/` thành công
- [x] `build\zephyr\zephyr.bin` + `zephyr.elf` tồn tại
- [x] `arm-zephyr-eabi-nm zephyr.elf | Select-String _SEGGER_RTT` trả về address

### Hardware

- [x] STM32F411E-DISCO nhận ra trong Device Manager (không "Unknown")
- [x] `west flash` ghi `32768 bytes` thành công
- [x] LED cam (PD13) nhấp nháy sau nhấn RESET
- [x] openocd RTT server tìm thấy control block
- [x] TCP port 9090: nhận được log messages

---

## Appendix — Quick reference

### Build + flash (full command)

```powershell
cd D:\Robot_OS
$env:PATH = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
$env:PATH = "C:\Program Files\CMake\bin;" + $env:PATH
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = "C:\zephyr-sdk-0.17.0"
west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
west flash
# → nhấn nút RESET trên board
```

### openocd RTT server (manual)

```powershell
$env:PATH = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
$openocd = (Get-Command openocd).Source
& $openocd -s "D:/Robot_OS/zephyr/boards/arm/stm32f411e_disco/support" -f openocd.cfg `
  -c "init" -c "reset run" -c "sleep 200" `
  -c "rtt setup 0x20000800 0x100 {SEGGER RTT}" `
  -c "rtt start" -c "rtt server start 9090 0"
```

### Tìm địa chỉ RTT block sau build mới

```powershell
C:\zephyr-sdk-0.17.0\arm-zephyr-eabi\bin\arm-zephyr-eabi-nm build\zephyr\zephyr.elf | Select-String "_SEGGER_RTT"
```
