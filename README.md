<p align="right">
  <a href="#繁體中文版">🇹🇼 繁體中文</a>
</p>

# M5Stack CoreS3 Diagnostic System

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v6.0-blue)](https://github.com/espressif/esp-idf)
[![Target](https://img.shields.io/badge/target-ESP32--S3-orange)](https://www.espressif.com/en/products/socs/esp32-s3)
[![tag](https://img.shields.io/badge/tag-v0.4.0--allpass-green)](https://github.com/ian0318git/m5stack-diag)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

An embedded diagnostic system for the **M5Stack CoreS3**, built on **ESP-IDF v6.0** with **Clean Architecture** principles and operated through a **USB Serial/JTAG console** (`/dev/ttyACM0`).

---

## Overview

This project provides a structured hardware validation tool for the CoreS3 development board. It scans, tests, and reports the status of every on-board peripheral through an interactive command-line interface over USB.

**Key features:**
- Test all on-board peripherals: LCD (ILI9342C), touch (FT6336U), RTC (BM8563), IMU (BMI270), PMU (AXP2101)
- Full I²C address range scan (0x01–0x7F) with automatic device identification
- Fugazi-style interactive menu with batch execution
- Component-level error reporting with debug hints
- Clean Architecture separation (Domain → Interface Adapter → Drivers)
- Reusable common chip drivers under `common/chips/`
- Automated RTC time recovery after power loss

---

## Architecture

```
.
├── common/
│   ├── chips/                          # Reusable chip drivers (board-agnostic)
│   │   ├── lcd_ILI9342C/               # ILI9342C register defs + SPI driver
│   │   ├── touch_FT6336/               # FT6336 register defs + I²C driver
│   │   ├── rtc_BM8563/                 # BM8563 register defs + I²C driver
│   │   ├── imu_BMI270/                 # BMI270 register defs + I²C driver
│   │   ├── power_AXP2101/              # AXP2101 register defs + I²C driver
│   │   └── aw9523b/                    # AW9523B GPIO expander driver
│   └── src/M5Stack_CoreS3/             # CoreS3 diagnostic project
│       ├── include/                    # Domain + Interface Adapter (pure interfaces)
│       │   ├── diag_core.h             # Core types, enums, test entities
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
│       │       └── hal_power.h
│       ├── src/
│       │   ├── main.c                  # ESP-IDF app_main entry
│       │   ├── diag_menu.c             # Console menu + CLI loop
│       │   ├── diag_menu_core.c        # Menu engine build/run/dispatch
│       │   ├── diag_runner.c           # Test runner implementation
│       │   ├── diag_error.c            # Error reporting (cterr-style)
│       │   └── hal/                    # CoreS3 board adapters
│       │       ├── hal_i2c_helpers.[ch] # Shared I²C bus (ref-counted)
│       │       ├── hal_screen.c        # SPI init → delegates to lcd_ILI9342C
│       │       ├── hal_touch.c         # I²C init → delegates to touch_FT6336
│       │       ├── hal_rtc.c           # I²C init → delegates to rtc_BM8563
│       │       ├── hal_imu.c           # I²C init → delegates to imu_BMI270
│       │       └── hal_power.c         # I²C init → delegates to power_AXP2101
│       └── CMakeLists.txt
├── doc/
│   ├── architecture.md                 # Architecture overview
│   └── diag_function_spec.md           # Diagnostics Functional Specification
├── example/
│   └── fugazi_ng_diag/                 # Reference codebase (Cisco NG-Diag)
└── CMakeLists.txt                      # Top-level ESP-IDF project
```

### Clean Architecture Layers

| Layer | Location | Responsibility |
|-------|----------|---------------|
| **Domain** | `include/diag_core.h`, `diag_error.h`, `diag_menu_core.h` | Test entities, error types, menu data structures |
| **Interface Adapter** | `diag_menu.*`, `diag_runner.*` | Console presenter/controller, test orchestration |
| **Frameworks & Drivers** | `include/hal/*`, `src/hal/*`, `common/chips/*` | HAL abstractions, board adapters, common chip drivers |

---

## Hardware Support

| Peripheral | Chip | Interface | I²C Addr |
|-----------|------|-----------|----------|
| Display | ILI9342C (320×240 IPS) | SPI (MOSI=G37, SCK=G36, CS=G3, DC=G35) | — |
| Touch | FT6336U capacitive touch | I²C | **0x38** |
| RTC | BM8563 real-time clock | I²C | **0x51** |
| IMU | BMI270 6-axis accel + gyro | I²C | **0x69** |
| PMU | AXP2101 power management | I²C | **0x34** |
| GPIO Expander | AW9523B (RST/INT control) | I²C | **0x58** |
| Audio ADC | ES7210 (dual mic) | I²C + I2S | **0x40** |
| Speaker Amp | AW88298 (1W I2S) | I²C + I2S | **0x36** |
| Camera | GC0308 (0.3MP, optional) | DVP + I²C | **0x21** |
| Proximity | LTR-553ALS-WA (optional) | I²C | **0x23** |

---

## Quick Start

### Prerequisites

- [ESP-IDF v6.0+](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/) installed
- M5Stack CoreS3 connected via USB

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

Once connected (115200 baud, `/dev/ttyACM0`):

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
| 2 | `display` | ILI9342C colour bars, text overlay, crosshair test |
| 3 | `touch` | FT6336U firmware version, touch-point read |
| 4 | `rtc` | BM8563 interval tick test (read → wait 2s → read → verify) |
| 5 | `imu` | BMI270 chip ID + accel/gyro register test |
| 6 | `power` | AXP2101 battery voltage, charge status, temperature |

### Example Sessions

#### Batch run-all

```
diag> run-all

Scanning full I2C address range 0x01-0x7F...
  Found devices:
    [ OK ] 0x34 — AXP2101 (PMU)
    [ OK ] 0x38 — FT6336U (Touch)
    [ OK ] 0x40 — ES7210 (Audio ADC)
    [ OK ] 0x51 — BM8563 (RTC)
    [ OK ] 0x58 — AW9523B (GPIO Exp)
    [ OK ] 0x69 — BMI270 (IMU)
  -- 0x36 AW88298 (Speaker) — optional, skip
  All P0 mandatory devices present.

I (4563) ILI9342C: ILI9342C initialised (320x240)
Screen test complete.
...
I (8321) FT6336: FT6336 initialised (fw=0x64)
Touch: FT6336 fw=0x64 max_points=2
RTC T1: 2026-07-19 20:27:02
RTC T2: 2026-07-19 20:27:04
RTC tick: OK (2 s elapsed)
I (10757) BMI270: BMI270 initialised (chip_id=0x24)
IMU: chip_id=0x24
  Accel (mg):   x=   +0  y=   +0  z=+981
  Gyro  (mdps): x=    +0  y=    +0  z=    +0
Power: Battery 4120 mV (98%), USB disconnected, Temp 35 C

Done: 0 failures.
```

#### Interactive fugazi-style menu

```
diag> menu

========== CoreS3 Diagnostics ==========
  Errors: 0  |  Run: 0
  *[ 1] I2C Bus Scan                     PASSED
  *[ 2] Display (ILI9342C)               PASSED
  *[ 3] Touch (FT6336)                   PASSED
  *[ 4] RTC (BM8563)                     PASSED
  *[ 5] IMU (BMI270)                     PASSED
  *[ 6] Power (AXP2101)                  PASSED

  [a]ll  [r]eset  [e]rrors  [q]uit
  Select test #: 3
Running [3] Touch (FT6336)...
Touch: FT6336 fw=0x64 max_points=2
Touch points: 1
  Point 0: (120, 180) event=2 id=0
Result: PASSED
```

#### Error report

```
diag> errors

========== Error Report ==========
  Total errors: 2
  [1] TOUCH / MB/TOUCH (x1)
      : I2C@0x38 FT6336: firmware version read returned 0
      > Check AXP2101 LDOIO0 touch power (reg 0x90)
      > Check I2C bus 0x38 pull-ups and INT/RST pins
  [2] IMU / MB/IMU (x1)
      : I2C@0x69 BMI270: config rejected (INT_STAT=0x02), no data
      > Chip ID 0x24 confirmed — I2C register comms verified
==================================
```

#### RTC auto-recovery

```
RTC T1: 2000-00-00 09:22:09
RTC T2: 2000-00-00 09:22:11
  ** VL flag was set — RTC time invalid, setting default...
  Setting RTC to build time: 2025-07-19 14:30:00
RTC now: 2025-07-19 14:30:01
RTC tick: OK (2 s elapsed)
Result: PASSED
```

#### System status

```
diag> status

========== System Status ==========
Battery: 28 mV (0%)
USB: disconnected
RTC: 2026-07-19 20:34:06
IMU: online
====================================
```

#### I²C bus scan detail

```
diag> run 1

Scanning full I2C address range 0x01-0x7F...

  Found devices:
    [ OK ] 0x34 — AXP2101 (PMU)
    [ OK ] 0x38 — FT6336U (Touch)
    [ OK ] 0x40 — ES7210 (Audio ADC)
    [ OK ] 0x51 — BM8563 (RTC)
    [ OK ] 0x58 — AW9523B (GPIO Exp)
    [ OK ] 0x69 — BMI270 (IMU)

  6 device(s) found on I2C bus
  -- 0x36 AW88298 (Speaker) — optional, skip
  All P0 mandatory devices present.
Result: PASSED
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
| SD card SPI | MOSI=G37, MISO=G35, SCK=G36, CS=G4 |
| Button (PWR) | G41 |
| Audio I2S | BCK=G34, WCK=G33, DATI=G13, DATO=G14, MCLK=G0 |

---

## Project Structure

| Path | Purpose |
|------|---------|
| `common/chips/*/` | Standalone, reusable chip drivers — no board dependency |
| `common/src/M5Stack_CoreS3/include/hal/` | HAL interface contracts (board-level abstraction) |
| `common/src/M5Stack_CoreS3/src/hal/` | CoreS3 board adapters — pin mux, bus init, delegate to chip drivers |
| `common/src/M5Stack_CoreS3/src/diag_*` | Application logic — menu, runner, error reporting |
| `doc/diag_function_spec.md` | Full DFS with coverage matrix, failure analysis, debug steps |
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
[![tag](https://img.shields.io/badge/tag-v0.4.0--allpass-green)](https://github.com/ian0318git/m5stack-diag)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

以 **ESP-IDF v6.0** 框架開發、採用**乾淨架構（Clean Architecture）**、透過 USB Serial/JTAG Console 操作的 M5Stack CoreS3 嵌入式硬體診斷系統。

---

## 概述

本專案為 CoreS3 開發板提供一套結構化的硬體驗證工具。透過互動式命令列介面，掃描、測試並回報所有板載週邊的狀態。

**主要特色：**
- 完整測試所有板載週邊：LCD (ILI9342C)、觸控 (FT6336U)、RTC (BM8563)、IMU (BMI270)、電源 (AXP2101)
- I²C 完整位址掃描 (0x01–0x7F)，自動辨識裝置
- Fugazi 風格互動數字選單 + 批次執行
- 元件級錯誤回報與除錯提示
- 乾淨架構分層（Domain → Interface Adapter → Drivers）
- `common/chips/` 目錄下提供可重複使用的通用晶片驅動
- RTC 時間遺失時自動回復

---

## 使用方式

連線後（115200 baud, `/dev/ttyACM0`）：

### 指令列表

| 指令 | 說明 |
|------|------|
| `help` | 顯示說明 |
| `info` | 列出所有測試與狀態 |
| `run <name\|#>` | 執行單一測試（依名稱或 1-based 編號） |
| `run-all` | 依序執行全部測試 |
| `menu` | 互動數字選單 |
| `errors` | 顯示錯誤報告 |
| `status` | 顯示系統狀態總覽 |
| `rtc-set YYYY MM DD HH MM SS` | 設定 RTC 時間 |
| `screen-on` / `screen-off` | 開啟/關閉螢幕 |
| `reboot` | 軟體重置 |
| `shutdown` | 系統關機 |
| `reset` | 清除測試結果與錯誤記錄 |
| `exit` / `quit` | 離開選單 |

### 測試清單

| # | 名稱 | 說明 |
|---|------|------|
| 1 | `i2c-scan` | 完整 I²C 位址掃描 + 裝置辨識 |
| 2 | `display` | ILI9342C 色彩條、文字、十字線測試 |
| 3 | `touch` | FT6336U 韌體版本、觸控點讀取 |
| 4 | `rtc` | BM8563 間隔計時測試（讀取→等待→讀取→驗證 tick） |
| 5 | `imu` | BMI270 晶片 ID + 加速規/陀螺儀暫存器測試 |
| 6 | `power` | AXP2101 電池電壓、充電狀態、溫度 |

### 操作範例

```
diag> run-all

Scanning full I2C address range 0x01-0x7F...
  Found devices:
    [ OK ] 0x34 — AXP2101 (PMU)
    [ OK ] 0x38 — FT6336U (Touch)
    [ OK ] 0x51 — BM8563 (RTC)
    [ OK ] 0x58 — AW9523B (GPIO Exp)
    [ OK ] 0x69 — BMI270 (IMU)
  All P0 mandatory devices present.

I (4563) ILI9342C: ILI9342C initialised (320x240)
...
RTC T1: 2026-07-19 20:27:02
RTC T2: 2026-07-19 20:27:04
RTC tick: OK (2 s elapsed)
IMU: chip_id=0x24
Power: Battery 4120 mV (98%)
Done: 0 failures.
```

```
diag> menu

========== CoreS3 Diagnostics ==========
  Errors: 0  |  Run: 6
  *[1] I2C Bus Scan                     PASSED
  *[2] Display (ILI9342C)               PASSED
  *[3] Touch (FT6336)                   PASSED
  *[4] RTC (BM8563)                     PASSED
  *[5] IMU (BMI270)                     PASSED
  *[6] Power (AXP2101)                  PASSED

  [a]ll  [r]eset  [e]rrors  [q]uit
  Select test #: 3
```

```
diag> errors

========== Error Report ==========
  Total errors: 1
  [1] IMU / MB/IMU (x1)
      : I2C@0x69 BMI270: config rejected (INT_STAT=0x02), no data
      > Chip ID 0x24 confirmed — I2C register comms verified
==================================
```

### 接腳配置（依 M5Stack 官方文件）

| 介面 | 接腳 |
|------|------|
| I²C 匯流排 | SDA=G12, SCL=G11 |
| LCD SPI | MOSI=G37, SCK=G36, CS=G3, DC=G35 |
| LCD RST | AW9523B P1_1（非直接 GPIO） |
| 背光 | AXP2101 DLDO1（非 GPIO20，與 USB D+ 共用） |
| Touch RST | AW9523B P0_0（非 GPIO1） |
| Touch INT | AW9523B P1_2（非 GPIO3） |
| SD 卡 | CS=G4, MOSI=G37, MISO=G35, SCK=G36 |
| 按鍵 (PWR) | G41 |

---

## 授權條款

本專案採用 **MIT 授權**。
