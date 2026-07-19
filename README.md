<p align="right">
  <a href="#繁體中文版">🇹🇼 繁體中文</a>
</p>

# M5Stack CoreS3 Diagnostic System

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.x-blue)](https://github.com/espressif/esp-idf)
[![Target](https://img.shields.io/badge/target-ESP32--S3-orange)](https://www.espressif.com/en/products/socs/esp32-s3)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

An embedded diagnostic system for the **M5Stack CoreS3**, built on **ESP-IDF v5.x** with **Clean Architecture** principles and operated through a **UART console**.

---

## Overview

This project provides a structured hardware validation tool for the CoreS3 development board. It scans, tests, and reports the status of every on-board peripheral through an interactive command-line interface over UART.

**Key features:**
- Test all on-board peripherals: LCD, touch, RTC, IMU, PMU
- I²C bus scan for device presence verification
- Clean Architecture separation (Domain → Interface Adapter → Drivers)
- Reusable common chip drivers under `common/chips/`
- Interactive UART menu with tab-completion friendly commands
- Batch mode (`run-all`) for production testing

---

## Architecture

```
.
├── common/
│   ├── chips/                          # Reusable chip drivers (board-agnostic)
│   │   ├── screen_GC9A01/              # GC9A01 register defs + SPI driver
│   │   ├── touch_FT6336/               # FT6336 register defs + I²C driver
│   │   ├── rtc_BM8563/                 # BM8563 register defs + I²C driver
│   │   ├── imu_BMI270/                 # BMI270 register defs + I²C driver
│   │   └── power_AXP2101/              # AXP2101 register defs + I²C driver
│   └── src/M5Stack_CoreS3/             # CoreS3 diagnostic project
│       ├── include/                    # Domain + Interface Adapter (pure interfaces)
│       │   ├── diag_core.h             # Core types, enums, test entities
│       │   ├── diag_config.h           # Board configuration & pin definitions
│       │   ├── diag_menu.h             # UART console menu interface
│       │   ├── diag_runner.h           # Test scheduler interface
│       │   └── hal/                    # HAL abstraction interfaces
│       │       ├── hal_screen.h
│       │       ├── hal_touch.h
│       │       ├── hal_rtc.h
│       │       ├── hal_imu.h
│       │       └── hal_power.h
│       ├── src/
│       │   ├── main.c                  # ESP-IDF app_main entry
│       │   ├── diag_menu.c             # UART menu implementation
│       │   ├── diag_runner.c           # Test runner implementation
│       │   └── hal/                    # CoreS3 board adapters
│       │       ├── hal_i2c_helpers.[ch] # Shared I²C bus (ref-counted)
│       │       ├── hal_screen.c        # SPI init → delegates to screen_GC9A01
│       │       ├── hal_touch.c         # I²C init → delegates to touch_FT6336
│       │       ├── hal_rtc.c           # I²C init → delegates to rtc_BM8563
│       │       ├── hal_imu.c           # I²C init → delegates to imu_BMI270
│       │       └── hal_power.c         # I²C init → delegates to power_AXP2101
│       └── CMakeLists.txt
├── doc/
│   └── architecture.md                 # Architecture document (Chinese)
├── example/
│   └── fugazi_ng_diag/                 # Reference codebase (Cisco NG-Diag)
└── CMakeLists.txt                      # Top-level ESP-IDF project
```

### Clean Architecture Layers

| Layer | Location | Responsibility |
|-------|----------|---------------|
| **Domain** | `include/diag_core.h` | Test entities, result enums, type definitions |
| **Interface Adapter** | `diag_menu.*`, `diag_runner.*` | Console presenter/controller, test orchestration |
| **Frameworks & Drivers** | `include/hal/*.h`, `src/hal/*.c`, `common/chips/*/` | HAL abstraction, board adapters, common chip drivers |

---

## Hardware Support

| Peripheral | Chip | Interface | HAL File |
|-----------|------|-----------|----------|
| Display | GC9A01 (240×240 circular LCD) | SPI | `hal_screen.[ch]` |
| Touch | FT6336 capacitive touch | I²C (0x38) | `hal_touch.[ch]` |
| RTC | BM8563 real-time clock | I²C (0x51) | `hal_rtc.[ch]` |
| IMU | BMI270 6-axis accelerometer + gyro | I²C (0x69) | `hal_imu.[ch]` |
| Power | AXP2101 power management unit | I²C (0x34) | `hal_power.[ch]` |

---

## Quick Start

### Prerequisites

- [ESP-IDF v5.x](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/) installed
- M5Stack CoreS3 connected via USB

### Build & Flash

```bash
# Set up ESP-IDF environment
source ~/esp/esp-idf/export.sh

# Navigate to project root
cd m5stack_diag

# Set target chip
idf.py set-target esp32s3

# Build
idf.py build

# Flash (adjust port as needed)
idf.py -p /dev/ttyACM0 flash

# Monitor serial output
idf.py -p /dev/ttyACM0 monitor
```

---

## Usage

Once connected via UART at **115200 baud**, the diagnostic menu is displayed:

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
| `run <name\|#>` | Run a single test by name or index |
| `run-all` | Execute every test sequentially |
| `status` | Show system status overview |
| `screen-on` | Turn the display on |
| `screen-off` | Turn the display off |
| `reboot` | Software reset the system |
| `shutdown` | Power off the system |
| `reset` | Clear stored test results |
| `exit` / `quit` | Exit the menu |

### Test List

| # | Name | Description |
|---|------|-------------|
| 0 | `i2c-scan` | Scan the I²C bus for peripheral devices |
| 1 | `screen` | Display color bars, text, and crosshairs |
| 2 | `touch` | Read FT6336 touch state and firmware version |
| 3 | `rtc` | Read BM8563 real-time clock |
| 4 | `imu` | Read BMI270 accelerometer and gyroscope data |
| 5 | `power` | Read AXP2101 battery voltage, charge status, and temperature |

### Example Session

```
diag> info

Test Suite: M5Stack CoreS3 Hardware Diagnostics
  6 tests registered

  [ 0] i2c-scan                         -- not run --
  [ 1] screen                           -- not run --
  [ 2] touch                            -- not run --
  [ 3] rtc                              -- not run --
  [ 4] imu                              -- not run --
  [ 5] power                            -- not run --

diag> run-all
Running ALL tests...

[ PASSED  ] I2C scan complete: 4 device(s) found  (123 ms)
[ PASSED  ] Screen test complete.                  (1520 ms)
[ PASSED  ] Touch: FT6336 fw=0x02 max_points=2     (45 ms)
[ PASSED  ] RTC time: 2025-07-19 14:32:05           (12 ms)
[ PASSED  ] IMU: chip_id=0x24                      (18 ms)
[ PASSED  ] Power: Battery 4120 mV (98%)           (15 ms)

========== Summary ==========
6 passed, 0 failed, 0 skipped

diag> status

========== System Status ==========
Battery: 4120 mV (98%)
USB: disconnected
RTC: 2025-07-19 14:32:12
IMU: online
====================================
```

---

## Project Structure

| Path | Purpose |
|------|---------|
| `common/chips/*/` | Standalone, reusable chip drivers — no board dependency |
| `common/src/M5Stack_CoreS3/include/hal/` | HAL interface contracts (board-level abstraction) |
| `common/src/M5Stack_CoreS3/src/hal/` | CoreS3 board adapters — pin mux, bus init, delegate to chip drivers |
| `common/src/M5Stack_CoreS3/src/diag_*` | Application logic — menu, runner |
| `example/fugazi_ng_diag/` | Reference production diagnostics codebase (Cisco) |

---

## License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.

---

<a name="繁體中文版"></a>

<p align="right">
  <a href="#m5stack-cores3-diagnostic-system">🇬🇧 English</a>
</p>

# M5Stack CoreS3 診斷系統

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.x-blue)](https://github.com/espressif/esp-idf)
[![Target](https://img.shields.io/badge/target-ESP32--S3-orange)](https://www.espressif.com/en/products/socs/esp32-s3)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

以 **ESP-IDF v5.x** 框架開發、採用**乾淨架構（Clean Architecture）**、透過 **UART Console** 操作的 M5Stack CoreS3 嵌入式硬體診斷系統。

---

## 概述

本專案為 CoreS3 開發板提供一套結構化的硬體驗證工具。透過互動式 UART 命令列介面，掃描、測試並回報所有板載週邊的狀態。

**主要特色：**
- 完整測試所有板載週邊：LCD 螢幕、觸控、RTC、IMU、電源管理
- I²C 匯流排掃描，確認各裝置是否存在
- 乾淨架構分層（Domain → Interface Adapter → Drivers）
- `common/chips/` 目錄下提供可重複使用的通用晶片驅動
- 互動式 UART 選單
- 批次模式（`run-all`）支援生產線測試

---

## 架構

```
.
├── common/
│   ├── chips/                          # 可重用晶片驅動（無電路板相依）
│   │   ├── screen_GC9A01/              # GC9A01 暫存器定義 + SPI 驅動
│   │   ├── touch_FT6336/               # FT6336 暫存器定義 + I²C 驅動
│   │   ├── rtc_BM8563/                 # BM8563 暫存器定義 + I²C 驅動
│   │   ├── imu_BMI270/                 # BMI270 暫存器定義 + I²C 驅動
│   │   └── power_AXP2101/              # AXP2101 暫存器定義 + I²C 驅動
│   └── src/M5Stack_CoreS3/             # CoreS3 診斷專案
│       ├── include/                    # Domain + Interface Adapter（純介面）
│       │   ├── diag_core.h             # 核心型別、列舉、測試實體
│       │   ├── diag_config.h           # 電路板配置與引腳定義
│       │   ├── diag_menu.h             # UART Console 選單介面
│       │   ├── diag_runner.h           # 測試排程介面
│       │   └── hal/                    # HAL 抽象介面
│       │       ├── hal_screen.h
│       │       ├── hal_touch.h
│       │       ├── hal_rtc.h
│       │       ├── hal_imu.h
│       │       └── hal_power.h
│       ├── src/
│       │   ├── main.c                  # ESP-IDF app_main 進入點
│       │   ├── diag_menu.c             # UART 選單實作
│       │   ├── diag_runner.c           # 測試執行器實作
│       │   └── hal/                    # CoreS3 電路板配接器
│       │       ├── hal_i2c_helpers.[ch] # 共享 I²C 匯流排（引用計數）
│       │       ├── hal_screen.c        # SPI init → 委派 screen_GC9A01
│       │       ├── hal_touch.c         # I²C init → 委派 touch_FT6336
│       │       ├── hal_rtc.c           # I²C init → 委派 rtc_BM8563
│       │       ├── hal_imu.c           # I²C init → 委派 imu_BMI270
│       │       └── hal_power.c         # I²C init → 委派 power_AXP2101
│       └── CMakeLists.txt
├── doc/
│   └── architecture.md                 # 架構說明文件
├── example/
│   └── fugazi_ng_diag/                 # 參考程式碼（Cisco NG-Diag）
└── CMakeLists.txt                      # 頂層 ESP-IDF 專案
```

### 乾淨架構分層

| 層級 | 位置 | 職責 |
|------|------|------|
| **Domain** | `include/diag_core.h` | 測試實體、結果列舉、型別定義 |
| **Interface Adapter** | `diag_menu.*`, `diag_runner.*` | Console 呈現/控制、測試編排 |
| **Frameworks & Drivers** | `include/hal/*.h`, `src/hal/*.c`, `common/chips/*/` | HAL 抽象、電路板配接、通用晶片驅動 |

---

## 硬體支援

| 週邊 | 晶片 | 介面 | HAL 檔案 |
|------|------|------|----------|
| 螢幕 | GC9A01（240×240 圓形 LCD） | SPI | `hal_screen.[ch]` |
| 觸控 | FT6336 電容式觸控 | I²C (0x38) | `hal_touch.[ch]` |
| 即時時鐘 | BM8563 | I²C (0x51) | `hal_rtc.[ch]` |
| 六軸感測器 | BMI270 加速度計 + 陀螺儀 | I²C (0x69) | `hal_imu.[ch]` |
| 電源管理 | AXP2101 | I²C (0x34) | `hal_power.[ch]` |

---

## 快速開始

### 前置需求

- 已安裝 [ESP-IDF v5.x](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/)
- 透過 USB 連接 M5Stack CoreS3

### 建置與燒錄

```bash
# 設定 ESP-IDF 環境
source ~/esp/esp-idf/export.sh

# 切換到專案目錄
cd m5stack_diag

# 設定目標晶片
idf.py set-target esp32s3

# 編譯
idf.py build

# 燒錄（請依實際情況調整埠號）
idf.py -p /dev/ttyACM0 flash

# 監控序列埠
idf.py -p /dev/ttyACM0 monitor
```

---

## 使用方式

透過 UART 以 **115200 baud** 連線後，將會顯示診斷選單：

```
============================================
   M5Stack CoreS3 Diagnostic System v1.0
============================================
Type 'help' for available commands.

diag>
```

### 指令列表

| 指令 | 說明 |
|------|------|
| `help` | 顯示說明 |
| `info` | 列出所有測試與狀態 |
| `run <name\|#>` | 執行單一測試（依名稱或編號） |
| `run-all` | 依序執行全部測試 |
| `status` | 顯示系統狀態總覽 |
| `screen-on` | 開啟螢幕 |
| `screen-off` | 關閉螢幕 |
| `reboot` | 軟體重置 |
| `shutdown` | 系統關機 |
| `reset` | 清除測試結果 |
| `exit` / `quit` | 離開選單 |

### 測試清單

| 編號 | 名稱 | 說明 |
|------|------|------|
| 0 | `i2c-scan` | 掃描 I²C 匯流排，檢查各週邊裝置 |
| 1 | `screen` | 顯示彩色條、文字、十字線 |
| 2 | `touch` | 讀取 FT6336 觸控狀態與韌體版本 |
| 3 | `rtc` | 讀取 BM8563 即時時鐘時間 |
| 4 | `imu` | 讀取 BMI270 加速度計與陀螺儀數值 |
| 5 | `power` | 讀取 AXP2101 電池電壓、充電狀態、溫度 |

### 操作範例

```
diag> info

Test Suite: M5Stack CoreS3 Hardware Diagnostics
  6 tests registered

  [ 0] i2c-scan                         -- not run --
  [ 1] screen                           -- not run --
  ...

diag> run-all

[ PASSED  ] I2C scan complete: 4 device(s) found  (123 ms)
[ PASSED  ] Screen test complete.                  (1520 ms)
...

========== Summary ==========
6 passed, 0 failed, 0 skipped
```

---

## 專案結構

| 路徑 | 用途 |
|------|------|
| `common/chips/*/` | 獨立、可重複使用的晶片驅動 — 無電路板相依 |
| `common/src/M5Stack_CoreS3/include/hal/` | HAL 介面合約（電路板層級抽象） |
| `common/src/M5Stack_CoreS3/src/hal/` | CoreS3 電路板配接器 — 引腳配置、匯流排初始化、委派晶片驅動 |
| `common/src/M5Stack_CoreS3/src/diag_*` | 應用邏輯 — 選單、執行器 |
| `example/fugazi_ng_diag/` | 參考生產診斷程式碼（Cisco） |

---

## 授權條款

本專案採用 **MIT 授權**。詳見 [LICENSE](LICENSE) 檔案。
