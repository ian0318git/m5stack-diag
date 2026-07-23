# M5Stack CoreS3 Diagnostics — Domain Glossary

## Diagnostic Domain

### Test (Diagnostic Test)
A single hardware validation procedure. Each test follows the signature `diag_result_t (*)(void *context)` and validates one aspect of a peripheral (register access, data readback, operator-observable behaviour). Tests are the atomic unit of work in the system.

### Test Suite
An ordered collection of tests that can be executed sequentially by the runner. Defined statically as a `diag_test_suite_t` table.

### Test Runner
Orchestrates test execution for a suite. Provides run-one, run-all, reset, and result-retrieval operations. Owns the lifecycle of test result records.

### Test Result
The outcome of executing a test. Five possible values ordered from best to worst: `PASSED`, `SKIPPED`, `FAILED`, `ERROR` (harness failure), `TIMEOUT` (did not complete).

### Test Record
A snapshot produced when a test finishes. Contains the test ID, result value, elapsed wall-clock time, and a free-text message.

### Error Context
A structured error accumulator attached to a test run. Tracks a component name, a location string, a list of error records, and up to two debug hint strings per error.

### Error Record
A single entry in the error context. Contains a formatted error message and optional debug guidance (e.g. "check power supply", "verify I2C pull-ups").

### Fugazi Menu Engine
An interactive CLI menu system. Tests are defined in a static `diag_menu_xtable_t` array and rendered as a numbered list. Supports batch execution (`run-all`), single-test selection, reset, and error display.

### Burn-In Test
A P2 on-demand test that runs all P0 and P1 tests in sequence for N iterations (default 100) or until the first failure. Used for intermittent fault reproduction during EDVT and field RMA.

## Hardware Topology

### SYS I2C Bus
The internal I2C bus (SDA=G12, SCL=G11) connecting the ESP32-S3 to all on-board peripheral ICs. Operates at 400 kHz (fast-mode). Shared by: AXP2101, AW9523B, FT6336U, BM8563, BMI270, ES7210, AW88298, GC0308, LTR-553.

### SPI2 Bus
The shared SPI bus (MOSI=G37, MISO=G35, SCK=G36) connecting the ESP32-S3 to the ILI9342C LCD (CS=G3) and the microSD card slot (CS=G4). Both peripherals share MOSI/MISO/SCLK with independent chip-select lines. Only one CS may be active at a time — managed by the SPI2 Bus Manager.

### I2S Audio Bus
The shared I2S bus (BCK=G34, WCK=G33, MCLK=G0) connecting the ESP32-S3 to the AW88298 speaker amplifier (DATO=G14) and the ES7210 audio ADC (DATI=G13). Configured for 16-bit, 48 ksps, Philips format.

### Power Domain
A voltage rail managed by the AXP2101 PMIC. Each on-board peripheral belongs to a specific power domain (e.g. SYS_3V3, LDOIO0, DLDO1, RTC_VDD). A peripheral is unreachable if its power domain is off.

### Flex Cable Assembly
An optional assembly containing the GC0308 camera (I2C@0x21) and LTR-553ALS-WA proximity/ALS sensor (I2C@0x23) on a shared flex cable. Both probe as NACK if the cable is absent; tests report SKIPPED rather than FAILED.

## Architecture Concepts

### Transport Seam
An abstract interface (`diag_i2c_t` / `diag_spi_t`) that decouples chip drivers from the platform-specific I2C/SPI implementation. Chip drivers call through function pointers; platform code provides concrete adapters (ESP-IDF, mock, etc.). See ADR-0001.

### Chip Driver
A platform-agnostic C module that drives a specific IC (e.g. AXP2101, BMI270). Communicates exclusively through the Transport Seam. Contains register definitions, init/deinit lifecycle, and data read functions. Has zero knowledge of the menu system, test framework, or ESP-IDF.

### Board Adapter (HAL)
A CoreS3-specific module that bridges between the test framework and Chip Drivers. Owns board-level concerns: I2C bus lifecycle, GPIO expander initialisation, pin mux configuration, and backlight power rail control. Delegates chip-specific operations to the appropriate Chip Driver through the Transport Seam.

### SPI2 Bus Manager
A ref-counted singleton that manages the shared SPI2 bus lifecycle. Tracks the LCD device handle and the SD card device handle separately. The bus is initialised on first use and freed when the last user releases it. See ADR-0002.

### AW9523B GPIO Expander
An I2C GPIO expander (addr 0x58) that controls peripheral reset and interrupt lines: P0_0 (TOUCH_RST), P0_2 (AW_RST), P1_0 (CAM_RST), P1_1 (LCD_RST), P1_2 (TOUCH_INT), P1_3 (AW_INT). Uses ref-counted init/deinit so multiple board adapters can safely share it. See ADR-0003.

### P0 / P1 / P2 Priority
A three-tier priority system from the DFS:
- **P0** — Mandatory for board bring-up (platform, display, touch, RTC, IMU, PMU). Must pass before any other test can run.
- **P1** — Required for full validation (audio, SD card, button, camera, proximity, backlight). Tested on demand.
- **P2** — System-level stress tests (burn-in, SPI lock, iteration counters). Used during EDVT and RMA.
