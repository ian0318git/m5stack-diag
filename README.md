> A production-grade board bring-up diagnostic tool, built during a self-directed career break to prove senior-level embedded work translates to a new platform.

<p align="right">
  <a href="#繁體中文版">🇹🇼 繁體中文</a>
</p>

# M5Stack CoreS3 Diagnostic System

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v6.0-blue)](https://github.com/espressif/esp-idf)
[![Target](https://img.shields.io/badge/target-ESP32--S3-orange)](https://www.espressif.com/en/products/socs/esp32-s3)
[![tag](https://img.shields.io/badge/tag-v1.0.0-green)](https://github.com/ian0318git/m5stack-diag)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

> **Senior Embedded Engineer Career Break Project**  
> Built during a career break to keep board bring-up, diagnostics architecture, and modern embedded practices sharp. Translates 25+ years of production networking / Day-0 experience into a clean, testable, open diagnostic framework.

An embedded diagnostic system for the **M5Stack CoreS3**, built on **ESP-IDF v6.0** with **Clean Architecture** principles and operated through a **USB Serial/JTAG console** (`/dev/ttyACM0`).

---

## Overview

This project provides a structured hardware validation tool for the CoreS3 development board. It scans, tests, and reports the status of every on-board peripheral through an interactive command-line interface over USB. Designed for **board bring-up**, **engineering validation (EDVT)**, and **manufacturing** — with structured error reporting and debug guidance for field technicians.

**Key features:**
- **25 tickets implemented** across 4 phases: platform → peripherals → advanced I/O → system integration
- **Full peripheral test suite**: LCD (ILI9342C), touch (FT6336U), RTC (BM8563), IMU (BMI270), PMU (AXP2101), speaker (AW88298), microphone (ES7210), SD card, camera (GC0308), proximity/ALS (LTR-553), backlight, button
- **Abstract transport seam** (`diag_transport.h`): all chip drivers communicate through `diag_i2c_t`/`diag_spi_t` — zero direct ESP-IDF dependency, mockable for unit tests
- **Shared SPI2 bus manager** for LCD + SD card coexistence (ref-counted lifecycle)
- **Interactive menu** with batch execution (`run-all`) and burn-in mode
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
│       │   ├── platform_wrappers.c       # Menu wrappers (int→void* bridge)
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
├── docs/
│   ├── architecture.md                 # Architecture overview
│   ├── diag_function_spec.md           # Diagnostics Functional Specification
│   └── platform_design_analysis.md       # Design pattern analysis of reference codebase
├── example/
│   └── platform_diag/                  # Reference production diagnostics codebase
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
| `menu` | Interactive number menu |
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

#### Interactive menu

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

## Development Process

This project was developed following the **[Matt Pocock Engineering Skills](https://github.com/mattpocock/mattpocock-skills)** framework — a structured AI-assisted development methodology. Each stage invoked a specific skill to maintain focus, enforce quality, and prevent premature optimisation.

| Stage | Skill | What was done |
|-------|-------|---------------|
| **0. Spec Writing** | `hfs-to-dfs-writer` | Transformed the M5Stack CoreS3 hardware-framework summary (HFS) — pin mappings, I²C address table, power distribution, bus topology — into a complete **Diagnostics Functional Specification (DFS)**. The DFS lives at `docs/diag_function_spec.md` and is the single source of truth for all 25 tickets |
| **1. Setup** | `setup-matt-pocock-skills` | Scaffolded per-repo config: issue tracker (GitHub), triage labels, domain docs layout |
| **2. Spec → Tickets** | `wayfinder` + `to-tickets` | Analysed engineering phase; broke the DFS into 25 actionable GitHub issues (#1–#25) across 4 phases |
| **3. Reference Analysis** | `codebase-design` | Deep-dive into `example/platform_diag/` (production diagnostics reference): OOP-in-C via FVT, callin/callout seam pattern, Null Object pattern. Output saved to `docs/platform_design_analysis.md` |
| **4. Architecture Evaluation** | `codebase-design` | Applied the deep/shallow/seam/adapter vocabulary to the CoreS3 codebase. Identified: shallow HAL pass-through, missing I2C/SPI transport seam, test functions coupled to main.c |
| **5. Architecture Refactor** | `mattpocock-skills:implement` | Implemented the improvements: `diag_transport.h` (abstract I²C/SPI seam), `hal_i2c_adapter.c` + `hal_spi_adapter.c` (ESP-IDF adapters), `hal_spi2_bus.c` (shared SPI2 manager), extracted test functions to `tests/` |
| **6. Implement Tickets** | `mattpocock-skills:implement` | Built all 25 tickets: 6 chip drivers (AW88298, ES7210, GC0308, LTR-553, backlight, button), 8 test functions, audio HAL, SPI2 bus manager, burn-in CLI command |
| **7. Code Review** | `code-review` | 10-angled max-effort review: 5 correctness angles + 3 cleanup + altitude + conventions. Found 15 findings → 15 fixed (100% fix rate). Included LTR553 register collision, SPI2 bus leak, BMI270 unchecked init, memory leaks, transport seam bypass |
| **8. Next** | `domain-modeling` (planned) | Formalise domain vocabulary in `CONTEXT.md` and architectural decisions in `docs/adr/` |

The framework's core principles — **correctness first**, **forced exception handling**, **no silent failures**, and the **deep module** heuristic — are reflected throughout the codebase. Each chip driver is a deep module: small interface (`diag_i2c_t` + init/read/deinit), large implementation hidden behind.

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
| `docs/diag_function_spec.md` | Full DFS with coverage matrix, failure analysis, debug steps — generated from HFS via `hfs-to-dfs-writer` skill |
| `docs/platform_design_analysis.md` | Design pattern analysis of the platform_diag reference codebase |
| `example/platform_diag/` | Reference production diagnostics codebase |

---

## CI/CD

The project uses **GitHub Actions** for continuous integration and deployment.

### CI — Continuous Integration

On every push to `main` and every PR to `main`:

| Job | What it does | Time |
|-----|-------------|------|
| **Host unit tests** | Compiles & runs 37 unit tests via `gcc` on `ubuntu-latest` | ~30s |
| **ESP-IDF build** | Cross-compiles firmware for esp32s3 via `espressif/idf:release-v6.0` docker | ~5-8 min |

### CD — Continuous Deployment

When a tag matching `v*` is pushed (e.g. `v1.1.0`):

1. ✅ **CI runs first** — unit tests + ESP-IDF build must pass
2. ✅ **Release created** — GitHub Release is auto-generated
3. ✅ **Binary attached** — `m5s3_diag.bin` is attached to the release

To trigger a release:

```bash
git tag -a v1.1.0 -m "v1.1.0"
git push origin v1.1.0
```

### Status Badges

![CI](https://github.com/ian0318git/m5stack-diag/actions/workflows/ci.yml/badge.svg)

---

## Development Journey

This project was built from scratch through every stage of the **Matt Pocock Engineering Skills** framework:

| Stage | Skill | Outcome |
|-------|-------|---------|
| **0. Spec** | `hfs-to-dfs-writer` | Diagnostics Functional Specification |
| **1. Setup** | `setup-matt-pocock-skills` | Project configuration |
| **2. Planning** | `wayfinder` + `to-tickets` | 25 GitHub Issues |
| **3. Reference** | `codebase-design` | platform_diag design pattern analysis |
| **4. Architecture** | `codebase-design` | CoreS3 architecture evaluation |
| **5. Refactoring** | `mattpocock-skills:implement` | Transport seam, SPI2 bus, test extraction |
| **6. Implementation** | `mattpocock-skills:implement` | All 25 tickets complete |
| **7. Code Review** | `code-review` | 10-angle max-effort, 15 findings fixed |
| **8. Domain** | `domain-modeling` | CONTEXT.md + 3 ADRs |
| **9. TDD** | `tdd` | 37 unit tests, 2 bugs found |
| **10. Research** | `research` | BMI270 config blob + M5Unified reverse engineering |
| **11. Grilling** | `grilling` | Architecture decision validation |
| **12. Bug Hunt** | `diagnosing-bugs` | BMI270: 4 bugs fixed (register offset, PWR_CTRL, INT_STATUS, config flag) |

Final hardware verification:

```
8 passed, 0 failed, 5 skipped
IMU: Accel (mg): x= +10  y= +988  z=  +4  ← gravity vector detected
```

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
[![tag](https://img.shields.io/badge/tag-v1.0.0-green)](https://github.com/ian0318git/m5stack-diag)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

> **資深嵌入式工程師 Career Break 專案**  
> 在職涯空窗期間打造，用來保持 Board Bring-up、診斷架構與現代嵌入式實務的敏銳度。把超過 25 年生產級網路設備 / Day-0 經驗，轉化成乾淨、可測試、可公開的診斷框架。

以 **ESP-IDF v6.0** 框架開發、採用**乾淨架構（Clean Architecture）**、透過 USB Serial/JTAG Console 操作的 M5Stack CoreS3 嵌入式硬體診斷系統。

---

## 概述

本專案為 CoreS3 開發板提供一套結構化的硬體驗證工具，涵蓋電路板除錯（board bring-up）、工程驗證（EDVT）與生產測試（manufacturing）三個階段。

**主要特色：**
- **25 個開發任務全部完成**，分為 4 個階段：平台基礎 → 主要週邊 → 進階 I/O → 系統整合
- **完整週邊測試**：LCD、觸控、RTC、IMU、電源管理、喇叭、麥克風、SD 卡、相機、光學感測、背光、按鈕
- **抽象傳輸層**（`diag_transport.h`）：所有晶片驅動透過 `diag_i2c_t`/`diag_spi_t` 介面通訊，與 ESP-IDF 完全解耦
- **SPI2 匯流排管理器**：LCD 與 SD 卡共用 SPI2 的參考計數生命週期管理
- **互動選單** + 批次執行 + Burn-In 測試
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

## 開發流程

本專案依照 **[Matt Pocock Engineering Skills](https://github.com/mattpocock/matt-pocock-skills)** 框架開發——一套結構化的 AI 輔助開發方法論。每個階段呼叫特定 skill 來維持焦點、確保品質，並避免過早最佳化。

| 階段 | Skill | 內容 |
|-------|-------|------|
| **0. 規格撰寫** | `hfs-to-dfs-writer` | 將 M5Stack CoreS3 硬體框架摘要（HFS）——接腳對應、I²C 位址表、電源分佈、匯流排拓撲——轉換為完整的**診斷功能規格書（DFS）**。DFS 位於 `docs/diag_function_spec.md`，是全部 25 個 tickets 的單一事實來源 |
| **1. 設定** | `setup-matt-pocock-skills` | 建立專案設定：issue tracker（GitHub）、triage labels、domain docs 佈局 |
| **2. Spec → Tickets** | `wayfinder` + `to-tickets` | 分析工程階段；將 DFS 拆解為 25 個可執行的 GitHub issues（#1–#25） |
| **3. 參考分析** | `codebase-design` | 深入分析 `example/platform_diag/`（產線診斷參考框架）：OOP-in-C via FVT、callin/callout seam 模式、Null Object 模式 |
| **4. 架構評估** | `codebase-design` | 對 CoreS3 程式碼進行 deep/shallow/seam/adapter 分析，識別出：淺層 HAL pass-through、缺少 I2C/SPI transport seam、test 與 main.c 耦合 |
| **5. 架構重構** | `mattpocock-skills:implement` | 實作改進：`diag_transport.h`（抽象 I²C/SPI seam）、`hal_i2c_adapter.c` + `hal_spi_adapter.c`（ESP-IDF adapters）、`hal_spi2_bus.c`（共用 SPI2 管理器）、提取 test 至 `tests/` |
| **6. 實作 Tickets** | `mattpocock-skills:implement` | 完成全部 25 個 tickets：6 個晶片驅動、8 個測試函式、audio HAL、SPI2 bus manager、burn-in CLI |
| **7. Code Review** | `code-review` | 10 角度最高強度審查：5 個正確性 + 3 個清理 + altitude + dimensions。15 個 finding 全部修復（修復率 100%） |
| **8. 下一步** | `domain-modeling`（規劃中） | 建立 `CONTEXT.md` 領域詞彙與 `docs/adr/` 架構決策記錄 |

---

## 開發旅程

本專案從零開始，完整走過 **Matt Pocock Engineering Skills** 框架的每個階段：

| 階段 | Skill | 成果 |
|------|-------|------|
| **0. Spec** | `hfs-to-dfs-writer` | 診斷功能規格書 DFS |
| **1. Setup** | `setup-matt-pocock-skills` | 專案設定 |
| **2. Planning** | `wayfinder` + `to-tickets` | 25 個 GitHub Issues |
| **3. Reference** | `codebase-design` | platform_diag 設計模式分析 |
| **4. Architecture** | `codebase-design` | CoreS3 架構評估 |
| **5. Refactoring** | `mattpocock-skills:implement` | Transport seam、SPI2 bus manager、tests extraction |
| **6. Implementation** | `mattpocock-skills:implement` | 25 tickets 全部完成 |
| **7. Code Review** | `code-review` | 10 角度 max-effort，15 findings 全部修復 |
| **8. Domain** | `domain-modeling` | CONTEXT.md + 3 ADRs |
| **9. TDD** | `tdd` | 37 unit tests，2 bugs found |
| **10. Research** | `research` | BMI270 config blob 調查 + M5Unified reverse engineering |
| **11. Grilling** | `grilling` | 架構決策驗證 |
| **12. Bug Hunt** | `diagnosing-bugs` | BMI270 4 bugs（register offset/PWR_CTRL/INT_STATUS/config flag） |

最終在實機上驗證通過：

```
8 passed, 0 failed, 5 skipped
IMU: Accel (mg): x= +10  y= +988  z=  +4  ← 重力感測正常
```

---

## 授權條款

本專案採用 **MIT 授權**。
