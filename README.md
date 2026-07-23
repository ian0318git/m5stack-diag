<p align="right">
  <a href="#繁體中文版">🇹🇼 繁體中文</a>
</p>

# M5Stack CoreS3 Diagnostic System

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v6.0-blue)](https://github.com/espressif/esp-idf)
[![Target](https://img.shields.io/badge/target-ESP32--S3-orange)](https://www.espressif.com/en/products/socs/esp32-s3)
[![tag](https://img.shields.io/badge/tag-v1.0.0--tickets--done-green)](https://github.com/ian0318git/m5stack-diag)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

An embedded diagnostic system for the **M5Stack CoreS3**, built on **ESP-IDF v6.0** with **Clean Architecture** principles and operated through a **USB Serial/JTAG console** (`/dev/ttyACM0`).

---

## Overview

This project provides a structured hardware validation tool for the CoreS3 development board. It scans, tests, and reports the status of every on-board peripheral through an interactive command-line interface over USB. Designed for **board bring-up**, **engineering validation (EDVT)**, and **manufacturing** — with structured error reporting and debug guidance for field technicians.

**Key features:**
- **25 tickets implemented** across 4 phases: platform → peripherals → advanced I/O → system integration
- **Full peripheral test suite**: LCD (ILI9342C), touch (FT6336U), RTC (BM8563), IMU (BMI270), PMU (AXP2101), speaker (AW88298), microphone (ES7210), SD card, camera (GC0308), proximity/ALS (LTR-553), backlight, button
- **Abstract transport seam** (`diag_transport.h`): all chip drivers communicate through `diag_i2c_t`/`diag_spi_t` — zero direct ESP-IDF dependency, mockable for unit tests
- **Shared SPI2 bus manager** for LCD + SD card coexistence (ref-counted lifecycle)
- **Fugazi-style interactive menu** with batch execution (`run-all`) and burn-in mode
- **Component-level error reporting** with structured `cterr`-style debug hints
- **Clean Architecture separation** (Domain → Interface Adapter → HAL → Chip Drivers)
- **Reusable common chip drivers** under `common/chips/` — platform-agnostic
- **Code-reviewed**: 15 findings found and fixed across architecture, correctness, and conventions

---

## Architecture

```
.
├── common/
│   ├── chips/                          # Reusable chip drivers (platform-agnostic via diag_i2c_t/diag_spi_t)
│   │   ├── lcd_ILI9342C/               # ILI9342C 320×240 SPI display
│   │   ├── touch_FT6336/               # FT6336U capacitive touch (I²C)
│   │   ├── rtc_BM8563/                 # BM8563 RTC (I²C)
│   │   ├── imu_BMI270/                 # BMI270 6-axis IMU (I²C)
│   │   ├── power_AXP2101/              # AXP2101 PMU (I²C)
│   │   ├── aw9523b/                    # AW9523B GPIO expander (I²C)
│   │   ├── audio_AW88298/              # AW88298 I2S speaker amp (I²C)
│   │   ├── audio_ES7210/               # ES7210 audio ADC (I²C)
│   │   ├── camera_GC0308/              # GC0308 0.3MP camera (I²C)
│   │   └── proximity_LTR553/           # LTR-553ALS-WA proximity + ALS (I²C)
│   └── src/M5Stack_CoreS3/             # CoreS3 diagnostic project
│       ├── include/                    # Domain + Interface Adapter (pure interfaces)
│       │   ├── diag_core.h             # Core types, enums, test entities
│       │   ├── diag_transport.h        # Abstract I²C/SPI transport seam
│       │   ├── diag_config.h           # Board configuration & pin definitions
│       │   ├── diag_menu.h             # Console menu interface
│       │   ├── diag_menu_core.h        # Menu engine types (submenu_xtable_t, mitem_t)
│       │   ├── diag_runner.h           # Test scheduler interface
│       │   ├── diag_error.h            # Error reporting interface (cterr-style)
│       │   └── hal/                    # HAL abstraction interfaces
│       │       ├── hal_screen.h
│       │       ├── hal_touch.h
│       │       ├── hal_rtc.h
│       │       ├── hal_imu.h
│       │       ├── hal_power.h
│       │       └── hal_audio.h
│       ├── src/
│       │   ├── main.c                  # Composition root (registers tests, wires layers)
│       │   ├── diag_menu.c             # Console menu + CLI loop
│       │   ├── diag_menu_core.c        # Menu engine build/run/dispatch
│       │   ├── diag_runner.c           # Test runner implementation
│       │   ├── diag_error.c            # Error reporting (cterr-style)
│       │   └── hal/                    # CoreS3 board adapters
│       │       ├── hal_i2c_helpers.[ch] # Shared I²C bus (ref-counted)
│       │       ├── hal_i2c_adapter.[ch] # ESP-IDF adapter for diag_i2c_t
│       │       ├── hal_spi_adapter.[ch] # ESP-IDF adapter for diag_spi_t
│       │       ├── hal_spi2_bus.[ch]   # Shared SPI2 bus (LCD + SD card)
│       │       ├── hal_audio.[ch]      # I2S TX/RX + AW9523B audio GPIO
│       │       ├── hal_screen.c        # SPI init → delegates to lcd_ILI9342C
│       │       ├── hal_touch.c         # I²C init → delegates to touch_FT6336
│       │       ├── hal_rtc.c           # I²C init → delegates to rtc_BM8563
│       │       ├── hal_imu.c           # I²C init → delegates to imu_BMI270
│       │       └── hal_power.c         # I²C init → delegates to power_AXP2101
│       ├── tests/                      # Test functions (one per compilation unit)
│       │   ├── diag_tests.h            # Declarations + global error context
│       │   ├── fugazi_wrappers.c       # Menu wrappers (int→void* bridge)
│       │   ├── test_i2c_scan.c
│       │   ├── test_screen.c
│       │   ├── test_touch.c
│       │   ├── test_rtc.c
│       │   ├── test_imu.c
│       │   ├── test_power.c
│       │   ├── test_backlight.c
│       │   ├── test_speaker.c
│       │   ├── test_microphone.c
│       │   ├── test_sdcard.c
│       │   ├── test_camera.c
│       │   ├── test_proximity.c
│       │   └── test_button.c
│       └── CMakeLists.txt
├── doc/
│   ├── architecture.md                 # Architecture overview
│   ├── diag_function_spec.md           # Diagnostics Functional Specification
│   └── fugazi_design_analysis.md       # Design pattern analysis of reference codebase
├── example/
│   └── fugazi_ng_diag/                 # Reference production diagnostics codebase (Cisco)
└── CMakeLists.txt                      # Top-level ESP-IDF project
```

### Clean Architecture Layers

| Layer | Location | Responsibility |
|-------|----------|---------------|
| **Domain** | `include/diag_core.h`, `diag_transport.h`, `diag_error.h`, `diag_menu_core.h` | Test entities, transport abstractions, error types, menu data structures |
| **Interface Adapter** | `diag_menu.*`, `diag_runner.*` | Console presenter/controller, test orchestration |
| **HAL** | `src/hal/*` | Board-specific initialization (I²C bus, SPI2 bus, I2S), transport adapters |
| **Chip Drivers** | `common/chips/*` | Platform-agnostic chip drivers via abstract transport seam |

### Transport Seam

All chip drivers communicate through abstract `diag_i2c_t` / `diag_spi_t` interfaces defined in `diag_transport.h`. Board adapters (`hal_i2c_adapter.c`, `hal_spi_adapter.c`) provide the concrete ESP-IDF implementation. This allows:
- **Unit testing**: inject a mock transport without real hardware
- **Platform porting**: swap the adapter layer without touching chip driver code
- **Verification**: each adapter function was code-reviewed and verified

---

## Hardware Support

| Peripheral | Chip | Interface | I²C Addr | Test |
|-----------|------|-----------|----------|------|
| Display | ILI9342C (320×240 IPS) | SPI (CS=G3) | — | T7 |
| Backlight | AXP2101 DLDO1 | I²C @0x34 | — | T8 |
| Touch | FT6336U capacitive touch | I²C | **0x38** | T9, T10 |
| RTC | BM8563 real-time clock | I²C | **0x51** | T11 |
| IMU | BMI270 6-axis accel + gyro | I²C | **0x69** | T12 |
| PMU | AXP2101 power management | I²C | **0x34** | T5 |
| GPIO Expander | AW9523B (RST/INT control) | I²C | **0x58** | T6 |
| Speaker Amp | AW88298 (1W I2S) | I²C + I2S | **0x36** | T13 |
| Audio ADC | ES7210 (dual mic) | I²C + I2S | **0x40** | T14 |
| SD Card | microSD slot | SPI (CS=G4) | — | T15 |
| Button | PWR side button | GPIO G41 | — | T16 |
| Camera | GC0308 (0.3MP, optional) | DVP + I²C | **0x21** | T17 |
| Proximity | LTR-553ALS-WA (optional) | I²C | **0x23** | T18 |

---

## Quick Start

### Prerequisites

- [ESP-IDF v6.0+](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/) installed
- M5Stack CoreS3 connected via USB
- User added to `dialout` group: `sudo usermod -a -G dialout $USER && logout`

### Build & Flash

```bash
# Set up ESP-IDF environment
source ~/esp/esp-idf/export.sh

# Navigate to project root
cd m5stack_diag

# Build
idf.py build

# Flash & monitor
idf.py -p /dev/ttyACM0 flash monitor
```

---

## Usage

Once connected (USB Serial/JTAG, visible as `/dev/ttyACM0`):

```
============================================
   M5Stack CoreS3 Diagnostic System v1.0
============================================
Type 'help' for available commands.

diag>
```

### Commands

| Command | Description |
|---------|-------------|
| `help` | Show help message |
| `info` | List all tests and their status |
| `run <name\|#>` | Run a single test by name or 1-based index |
| `run-all` | Execute every test sequentially |
| `menu` | Interactive fugazi-style number menu |
| `burnin [iterations]` | Burn-in test (default 100 iterations, stops at first failure) |
| `errors` | Display structured error report |
| `status` | Show system status overview |
| `rtc-set YYYY MM DD HH MM SS` | Set the RTC time |
| `screen-on` / `screen-off` | Turn the display on/off |
| `reboot` | Software reset the system |
| `shutdown` | Power off the system |
| `reset` | Clear stored test results and error records |
| `exit` / `quit` | Exit the menu |

### Test List

| # | Name | Description |
|---|------|-------------|
| 1 | `i2c-scan` | Full I²C address scan (0x01–0x7F) with device identification |
| 2 | `screen` | ILI9342C colour bars, text overlay, crosshair test |
| 3 | `touch` | FT6336U firmware version, touch-point read |
| 4 | `rtc` | BM8563 interval tick test (read → wait 2s → read → verify) |
| 5 | `imu` | BMI270 chip ID + accel/gyro register test |
| 6 | `power` | AXP2101 battery voltage, charge status, temperature |
| 7 | `backlight` | AXP2101 DLDO1 backlight toggle (visual check) |
| 8 | `speaker` | AW88298 1 kHz sine wave tone (500 ms) |
| 9 | `mic` | ES7210 audio capture (100 ms, RMS level) |
| 10 | `sdcard` | microSD mount/write/read-back verify (4 KB) |
| 11 | `camera` | GC0308 probe + chip ID (SKIP if flex cable absent) |
| 12 | `proximity` | LTR-553 ALS + proximity read (SKIP if flex cable absent) |
| 13 | `button` | PWR button press test (5 s operator window) |

### Example Sessions

#### Full run-all

```
diag> run-all

Running ALL tests...

I2C Bus Scan:
  Found devices:
    [ OK ] 0x34 — AXP2101 (PMU)
    [ OK ] 0x38 — FT6336U (Touch)
    [ OK ] 0x51 — BM8563 (RTC)
    [ OK ] 0x58 — AW9523B (GPIO Exp)
    [ OK ] 0x69 — BMI270 (IMU)
  All P0 mandatory devices present.

Display (ILI9342C):
  Display: RED   (500 ms)...
  Display: GREEN (500 ms)...
  Display: BLUE  (500 ms)...
  Display: BLACK (500 ms)...
  Display test PASSED (visual check required)

Touch (FT6336):        fw=0x64 max_points=2
RTC (BM8563):          2026-07-23 14:30:02 (tick OK)
IMU (BMI270):          chip_id=0x24
Power (AXP2101):       Battery 4120 mV (98%), USB disconnected, Temp 35 C
...

========== Summary ==========
  [ PASSED  ] I2C Bus Scan
  [ PASSED  ] Display (ILI9342C)
  ...
0 passed, 0 failed, 0 skipped
```

#### Burn-in test

```
diag> burnin 10

========== Burn-In Test ==========
  Target: 10 iterations
--- Iteration 1/10 ---
** PASSED
--- Iteration 2/10 ---
** PASSED
...

========== Burn-In Summary ==========
  Completed: 10/10 iterations
  Total failures: 0
  Result: PASSED
```

#### Fugazi-style menu

```
diag> menu

========== CoreS3 Diagnostics ==========
  Errors: 0  |  Run: 0
  *[ 1] I2C Bus Scan                     ----
  *[ 2] Display (ILI9342C)               ----
  *[ 3] Touch (FT6336)                   ----
  *[ 4] RTC (BM8563)                     ----
  *[ 5] IMU (BMI270)                     ----
  *[ 6] Power (AXP2101)                  ----
  *[ 7] Backlight (DLDO1)                ----
  *[ 8] Speaker (AW88298)                ----
  *[ 9] Microphone (ES7210)              ----
  *[10] Camera (GC0308)                  ----
  *[11] Proximity (LTR-553)              ----
  *[12] SD Card (microSD)                ----
  *[13] Button (PWR)                     ----

  [a]ll  [r]eset  [e]rrors  [q]uit
  Select test #:
```

---

## Key Hardware Details

### Correct Pin Mapping (per M5Stack official docs)

| Interface | Pins |
|-----------|------|
| I²C bus | SDA=G12, SCL=G11 |
| LCD SPI | MOSI=G37, SCK=G36, CS=G3, DC=G35 |
| LCD RST | AW9523B P1_1 (NOT a direct GPIO) |
| Backlight | AXP2101 DLDO1 (NOT GPIO20 — shared with USB D+) |
| Touch RST | AW9523B P0_0 (NOT GPIO1) |
| Touch INT | AW9523B P1_2 (NOT GPIO3) |
| SD card SPI | MOSI=G37, MISO=G35, SCK=G36, CS=G4 (shares SPI2 with LCD) |
| Button (PWR) | G41 |
| Audio I2S | BCK=G34, WCK=G33, DATI=G13, DATO=G14, MCLK=G0 |
| Speaker RST | AW9523B P0_2 |
| Camera RST | AW9523B P1_0 |

---

## Project Structure

| Path | Purpose |
|------|---------|
| `common/chips/*/` | Standalone, reusable chip drivers — platform-agnostic, use `diag_i2c_t`/`diag_spi_t` |
| `include/diag_transport.h` | Abstract I²C/SPI transport seam (zero ESP-IDF dependency) |
| `include/hal/` | HAL interface contracts (board-level abstraction) |
| `src/hal/` | CoreS3 board adapters + ESP-IDF transport adapters |
| `tests/` | Test functions — one compilation unit per test |
| `doc/diag_function_spec.md` | Full DFS with coverage matrix, failure analysis, debug steps |
| `doc/fugazi_design_analysis.md` | Design pattern analysis of the reference codebase (fugazi_ng_diag) |
| `example/fugazi_ng_diag/` | Reference production diagnostics codebase (Cisco) |

---

## License

This project is licensed under the **MIT License**.

---

<a name="繁體中文版"></a>

<p align="right">
  <a href="#m5stack-cores3-diagnostic-system">🇬🇧 English</a>
</p>

# M5Stack CoreS3 診斷系統

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v6.0-blue)](https://github.com/espressif/esp-idf)
[![Target](https://img.shields.io/badge/target-ESP32--S3-orange)](https://www.espressif.com/en/products/socs/esp32-s3)
[![tag](https://img.shields.io/badge/tag-v1.0.0--tickets--done-green)](https://github.com/ian0318git/m5stack-diag)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

以 **ESP-IDF v6.0** 框架開發、採用**乾淨架構（Clean Architecture）**、透過 USB Serial/JTAG Console 操作的 M5Stack CoreS3 嵌入式硬體診斷系統。

---

## 概述

本專案為 CoreS3 開發板提供一套結構化的硬體驗證工具，涵蓋電路板除錯（board bring-up）、工程驗證（EDVT）與生產測試（manufacturing）三個階段。

**主要特色：**
- **25 個開發任務全部完成**，分為 4 個階段：平台基礎 → 主要週邊 → 進階 I/O → 系統整合
- **完整週邊測試**：LCD、觸控、RTC、IMU、電源管理、喇叭、麥克風、SD 卡、相機、光學感測、背光、按鈕
- **抽象傳輸層**（`diag_transport.h`）：所有晶片驅動透過 `diag_i2c_t`/`diag_spi_t` 介面通訊，與 ESP-IDF 完全解耦
- **SPI2 匯流排管理器**：LCD 與 SD 卡共用 SPI2 的參考計數生命週期管理
- **Fugazi 風格互動選單** + 批次執行 + Burn-In 測試
- **元件級錯誤回報**：結構化 `cterr` 風格的除錯提示
- **乾淨架構分層**（Domain → Interface Adapter → HAL → Chip Drivers）
- **可重複使用的晶片驅動**：`common/chips/` 下的驅動與平台無關
- **Code Review 完成**：15 個發現全部修復

## 使用方式

### 建置與燒錄

```bash
source ~/esp/esp-idf/export.sh
cd m5stack_diag
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

### 指令列表

| 指令 | 說明 |
|------|------|
| `help` | 顯示說明 |
| `info` | 列出所有測試與狀態 |
| `run <name\|#>` | 執行單一測試（依名稱或編號） |
| `run-all` | 依序執行全部測試 |
| `menu` | 互動數字選單 |
| `burnin [iterations]` | Burn-In 測試（預設 100 次） |
| `errors` | 顯示錯誤報告 |
| `status` | 顯示系統狀態總覽 |
| `rtc-set YYYY MM DD HH MM SS` | 設定 RTC |
| `screen-on` / `screen-off` | 開啟/關閉螢幕 |
| `reboot` | 軟體重置 |
| `shutdown` | 系統關機 |
| `reset` | 清除測試結果與錯誤記錄 |
| `exit` / `quit` | 離開選單 |

### 測試清單

| # | 名稱 | 說明 |
|---|------|------|
| 1 | `i2c-scan` | 完整 I²C 位址掃描 (0x01–0x7F) |
| 2 | `screen` | ILI9342C 色彩條/文字/十字線 |
| 3 | `touch` | FT6336U 觸控測試 |
| 4 | `rtc` | BM8563 間隔計時測試 |
| 5 | `imu` | BMI270 晶片 ID + 感測器讀取 |
| 6 | `power` | AXP2101 電壓/充電/溫度 |
| 7 | `backlight` | DLDO1 背光測試 |
| 8 | `speaker` | 1 kHz 正弦波輸出 |
| 9 | `mic` | 立體聲麥克風擷取 |
| 10 | `sdcard` | microSD 寫入/讀回驗證 |
| 11 | `camera` | GC0308 相機探測（選購件） |
| 12 | `proximity` | LTR-553 光學感測（選購件） |
| 13 | `button` | PWR 按鈕測試 |

---

## 授權條款

本專案採用 **MIT 授權**。
