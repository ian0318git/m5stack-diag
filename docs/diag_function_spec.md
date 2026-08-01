# Diagnostics Functional Specification — M5Stack CoreS3

**Platform:** M5Stack CoreS3 (ESP32-S3)  
**Document version:** 1.0 — 2025-07-19  
**HFS source:** https://docs.m5stack.com/zh_CN/core/CoreS3  
**Diagnostics package:** `common/src/M5Stack_CoreS3/`  

---

## Overview

The M5Stack CoreS3 is a compact embedded IoT controller built around the
Espressif ESP32-S3 dual-core Xtensa LX7 SoC. The diagnostics package
provides hardware validation for every on-board peripheral across three
lifecycle stages: board bring-up (register-level access, fault isolation),
engineering validation (margin/stress observation), and manufacturing
(pass/fail batch testing). The package operates exclusively over the
ESP32-S3 built-in USB Serial/JTAG console, presenting an interactive
command menu driven by the interactive menu engine.

## SKUs

This DFS covers a single SKU: the M5Stack CoreS3 (with or without the
DinBase carrier). The diagnostics detect the absence of optional
peripherals (camera, proximity sensor on flex cable) at runtime and
report them as SKIPPED rather than FAILED.

---

## Hardware Overview

### Hardware Block Diagram

The CoreS3 hardware topology is a single ESP32-S3 SoC communicating with
all peripherals over an internal SYS I2C bus (SDA=G12, SCL=G11) and a
single shared SPI bus (MOSI=G37, SCK=G36, MISO=G35). The AXP2101 PMIC
manages power rails and battery charging; the AW9523B GPIO expander
controls reset and interrupt lines for the touch, display, camera, and
audio subsystems.

### I2C Address Map

| Address | Chip | Function | Power Domain | Notes |
|---------|------|----------|-------------|-------|
| 0x34 | AXP2101 | PMIC — battery, LDOs, DC-DC | Always-on (VBUS/battery) | Platform root — all other power depends on this |
| 0x58 | AW9523B | GPIO expander | AXP2101 rail | Controls RST/INT for touch, LCD, audio, camera |
| 0x38 | FT6336U | Capacitive touch controller | AXP2101 LDOIO0 (reg 0x90) | TOUCH_RST via AW9523B P0_0 |
| 0x51 | BM8563 | RTC | AXP2101 RTC_VDD | VL flag indicates power loss |
| 0x69 | BMI270 | 6-axis IMU (accel + gyro) | AXP2101 rail | Chip ID register 0x00 = 0x24 expected |
| 0x10 | BMM150 | 3-axis magnetometer | Shared with BMI270 | Via BMI270 auxiliary I2C sensor hub — not directly on SYS I2C |
| 0x36 | AW88298 | I2S speaker amplifier (1W) | AXP2101 rail | AW_RST via AW9523B P0_2 |
| 0x40 | ES7210 | Audio ADC (dual mic) | AXP2101 rail | I2S DATI=G13, BCK=G34, WCK=G33, MCLK=G0 |
| 0x21 | GC0308 | Camera (0.3 MP DVP) | AXP2101 rail | CAM_RST via AW9523B P1_0; on flex cable |
| 0x23 | LTR-553ALS-WA | Proximity + ambient light | AXP2101 rail | Same flex cable as camera |

### SPI Bus Topology

| Bus | MOSI | MISO | SCK | CS#0 | CS#1 |
|-----|------|------|-----|------|------|
| SPI2 (shared) | G37 | G35 | G36 | G3 (ILI9342C LCD) | G4 (microSD) |

The LCD and SD card slot share MOSI, MISO, and SCK on SPI2 with
independent chip-select lines. Both cannot be active simultaneously —
the diagnostics must ensure they are tested sequentially.

### Power Distribution

| Rail | Source | Destination |
|------|--------|-------------|
| VBUS | USB Type-C | AXP2101 input, battery charger |
| BAT | 500 mAh LiPo | AXP2101 battery input |
| DLDO1 | AXP2101 reg 0x12 | LCD backlight (LX1) |
| LDOIO0 | AXP2101 reg 0x90 | FT6336U touch VCC (3.3 V) |
| RTC_VDD | AXP2101 | BM8563 backup |
| SYS_3V3 | AXP2101 DC-DC | Digital core, I2C pull-ups, all peripheral ICs |

### AW9523B GPIO Expander — Pin Assignment

| Expander Pin | Dir | CoreS3 Connection |
|--------------|-----|-------------------|
| P0_0 | Out | FT6336U TOUCH_RST (active-low) |
| P0_2 | Out | AW88298 AW_RST (active-low) |
| P1_0 | Out | GC0308 CAM_RST (active-low) |
| P1_1 | Out | ILI9342C LCD_RST (active-low) |
| P1_2 | In | FT6336U TOUCH_INT (open-drain) |
| P1_3 | In | AW88298 AW_INT (open-drain) |

### Audio I2S Bus

| Signal | GPIO | Source | Sink |
|--------|------|--------|------|
| BCK (bit clock) | G34 | ESP32-S3 | ES7210, AW88298 |
| WCK (word clock) | G33 | ESP32-S3 | ES7210, AW88298 |
| DATI (mic data) | G13 | ES7210 | ESP32-S3 |
| DATO (speaker data) | G14 | ESP32-S3 | AW88298 |
| MCLK (master clock) | G0 | ESP32-S3 | ES7210, AW88298 |

### Camera DVP Bus (GC0308)

| Signal | GPIO | Signal | GPIO |
|--------|------|--------|------|
| PCLK | G45 | D0 | G39 |
| VSYNC | G46 | D1 | G40 |
| HREF | G38 | D2 | G41 |
| — | — | D3 | G42 |
| CAM_RST | AW9523B P1_0 | D4–D7 | G15, G16, G48, G47 |

### External I/O

| Port | Yellow | White | Red | Black |
|------|--------|-------|-----|-------|
| PORT.A | G2 (SDA) | G1 (SCL) | 5 V | GND |
| PORT.B | G9 | G8 | 5 V | GND |
| PORT.C | G17 | G18 | 5 V | GND |

### Buttons

| Button | GPIO | Function |
|--------|------|----------|
| PWR (side) | G41 | Click = on; hold 6 s = off |
| RST (bottom) | — | Click = reset; hold 3 s = download mode |

---

## Diagnostics Design Considerations and Requirements

### Hardware Bring-up

During board bring-up the diagnostics must provide:

- **Register-level read/write access** to every I2C device on the SYS bus,
  including a full address-range scan that lists every responding device.
- **GPIO expander control** — the AW9523B must be exercisable so that RST
  lines for the touch, display, audio, and camera can be asserted and
  released individually, independent of the power-up-default state.
- **SPI bus integrity verification** — both the LCD and SD card CS paths
  must be provably reachable without assuming a correctly configured
  device driver.

### Engineering Validation (EDVT)

During EDVT the diagnostics must provide:

- **Iteration count** — every test tracks execution count and cumulative
  error count so that intermittent failures can be reproduced by looping.
- **Voltage and temperature observation** — the AXP2101 PMIC supports
  on-chip ADC measurements for battery voltage, VBUS voltage, charge
  current, and die temperature; these are read and logged without
  requiring external measurement equipment.
- **Margin utilities** — the backlight brightness (AXP2101 DLDO1) and
  audio output level (AW88298) are software-adjustable to stress the
  power rail and analogue output chain.

### Manufacturing

On the production line the diagnostics must deliver:

- **Single-command batch execution** — `run-all` executes every P0 and P1
  test in dependency order and produces a PASSED/FAILED summary.
- **Explicit error codes** — every failure includes the interface address
  and component name so that a technician can identify the failing part
  without reading console output verbosely.
- **Test time** — the full P0 + P1 suite completes within 30 seconds.

### Test Coverage and Failure Analysis

The coverage matrix in the following section is the pivot between the
hardware topology and the detailed test cases. Every row marked
"Explicitly" / "Default: Yes" receives a full test case in the Tests and
Commands section.

### Field Support

The diagnostics package extends to field RMA support. Field technicians
run the same `run-all` command as manufacturing and additionally have
access to the `menu` interactive mode for targeted re-testing. No
external test equipment is required beyond the USB cable and a host
terminal.

---

## Diagnostics Design Assumptions

1. **USB console availability**: The ESP32-S3 built-in USB Serial/JTAG
   controller is the sole console channel. GPIO19 (USB_D+) and GPIO20
   (USB_D−) must not be reconfigured as GPIO outputs by any test, since
   doing so disconnects USB and terminates the diagnostic session.
2. **SPI bus exclusivity**: The LCD and SD card share the same SPI bus.
   The diagnostics serialise LCD and SD card tests; concurrent access is
   not supported.
3. **Battery presence**: The AXP2101 battery-present bit (register 0x00
   bit 3) may read 0 if the 500 mAh LiPo is fully depleted or not
   connected. This is not a hardware failure.
4. **Camera and proximity flex cable**: The GC0308 camera and
   LTR-553ALS-WA proximity sensor are on a shared flex cable that is
   optionally assembled. Both test as SKIPPED if the I2C probe at their
   addresses yields NACK.

---

## Items Not Covered By Diagnostics

| Scope gap | Rationale |
|-----------|-----------|
| CPU internal cache/core health | Proved implicitly by successful firmware boot and diagnostics execution itself. No dedicated March or parity-stress test is written. |
| Wi-Fi / BLE radio | The ESP32-S3 radio peripheral is not exercised. The diagnostics focus on board-level hardware; radio validation belongs to the RF test fixture. |
| BMM150 magnetometer | Accessible only through the BMI270 auxiliary I2C sensor hub, whose configuration registers are vendor-proprietary and require a large blob of initialisation data. The magnetometer is implicitly exercised if the BMI270 sensor hub enumeration succeeds. |
| DinBase DC input (9–24 V) | The DinBase power path is purely passive (no I2C monitoring). Validation requires external metering, not software diagnostics. |
| USB OTG (host mode) | The USB Type-C port is used by the console itself during diagnostics. Switching to host mode would terminate the session. |

---

## Diagnostics Overview

The diagnostics package is a single firmware image built with ESP-IDF v6.0
for the ESP32-S3 target. It runs as the main application (no OS, no Linux
kernel). The image is flashed via the standard `idf.py flash` command over
the USB Serial/JTAG port.

### Initialisation Sequence

1. Bootloader (ROM + stage 2) initialises PSRAM, flash, and the USB
   Serial/JTAG console.
2. `app_main()` initialises NVS flash storage, the USB Serial/JTAG driver,
   and the error-reporting context.
3. The interactive menu engine builds the test table from a static
   `diag_menu_xtable_t` array.
4. The interactive CLI loop begins, accepting commands over USB.
5. No test runs automatically at boot — the operator selects a test via
   the menu or types `run-all`.

---

## Tests and Commands

### Test Coverage Matrix

| Subsystem | Test/Utility | Interface | Component | Coverage | Default | Comment |
|-----------|-------------|-----------|-----------|----------|---------|---------|
| **Platform** | I2C Bus Scan | SYS I2C (G12/G11) | ESP32-S3→all I2C slaves | Explicitly | Yes | Probes 0x01–0x7F; cross-references expected addresses |
| | PMU Register Test | I2C@0x34 | ESP32-S3→AXP2101 | Explicitly | Yes | Verifies chip ID, reads battery/voltage/temperature ADCs |
| | GPIO Expander Test | I2C@0x58 | ESP32-S3→AW9523B | Explicitly | Yes | Verifies chip ID, toggles each output pin |
| **Display** | LCD Internal Test | SPI2 (G37/G36/G3/G35) | ESP32-S3→ILI9342C | Explicitly | Yes | Init sequence, colour fill, text, crosshair; RST via AW9523B P1_1 |
| | Backlight Test | AXP2101 DLDO1 | ESP32-S3→AXP2101→LCD LX1 | Explicitly | Yes | Enables DLDO1 at 3.3 V via PMU register 0x12; visual check by operator |
| **Touch** | Touch Register Test | I2C@0x38 + AW9523B P0_0/P1_2 | ESP32-S3→FT6336U | Explicitly | Yes | Power via AXP2101 LDOIO0; read device mode + firmware version |
| | Touch Read Test | I2C@0x38 | FT6336U→SYS I2C | Explicitly | Yes | Read touch-point coordinates; operator touches panel |
| **RTC** | RTC Register Test | I2C@0x51 | ESP32-S3→BM8563 | Explicitly | Yes | Read time registers; verify BCD range; check VL flag |
| **IMU** | IMU Register Test | I2C@0x69 | ESP32-S3→BMI270 | Explicitly | Yes | Read chip ID (0x24); soft-reset; read accel/gyro data |
| **Audio** | Speaker Amp Test | I2C@0x36 + I2S + AW9523B P0_2 | ESP32-S3→AW88298→Speaker | Explicitly | No | On-demand; generates 1 kHz tone for 500 ms |
| | Microphone ADC Test | I2C@0x40 + I2S | ES7210→ESP32-S3 | Explicitly | No | On-demand; captures 100 ms, reports RMS level |
| **Storage** | SD Card Mount Test | SPI2 (G4 CS) | ESP32-S3→microSD | Explicitly | No | Mounts FAT32; writes 4 KB; reads back and verifies |
| **Input** | Button Read | GPIO G41 | Side button | Explicitly | No | Operator presses button within 5 s window |
| **Camera** | Camera Register Test | I2C@0x21 + AW9523B P1_0 | ESP32-S3→GC0308 | Explicitly | No | I2C probe; chip ID read. SKIP if no flex cable detected |
| **Proximity** | Proximity Read | I2C@0x23 | LTR-553ALS-WA | Explicitly | No | Read ALS and proximity registers. SKIP if no flex cable |
| **Visual** | Charge LED | AXP2101 CHG_LED | AXP2101→LED | Manually | No | Operator observes LED state |
| **Connectivity** | Wi-Fi Station Connect | 2.4 GHz radio | ESP32-S3→AP→DHCP | Explicitly | Yes | Join SSID, obtain IP, report RSSI. SKIP if no credentials or no AP found |
| | NTP Time Sync | UDP/123 | SNTP→BM8563 | Explicitly | No | On-demand; writes RTC on success, RTC untouched on failure |
| | Diagnostic Upload | HTTP POST | ESP32-S3→server | Explicitly | No | On-demand; JSON of test results and error context |
| | MQTT Publish | MQTT over TCP | ESP32-S3→broker | Explicitly | No | On-demand; JSON published to broker topic |
| **Utilities** | I2C Bus Scan Utility | SYS I2C | All slaves | Manually | No | Full address-range scan |
| | System Status | All | All P0/P1 | Manually | No | Aggregate health summary |

### Explicit Test Cases

#### Platform — I2C Bus Scan

This test probes every I2C address from 0x01 to 0x7F on the SYS I2C bus
(SDA=G12, SCL=G11) using the `i2c_master_probe()` API with a 50 ms
timeout per address. For each address that returns ACK, the address is
matched against the known device table (§Hardware Overview — I2C Address
Map) and reported by name; unknown addresses are flagged as UNKNOWN.
After the full scan, a cross-reference pass confirms that every expected
P0 device (AXP2101 at 0x34, AW9523B at 0x58) responded, and warns for
any that did not.

##### Test Path Block Diagram

```
ESP32-S3 (I2C master) ── SDA=G12, SCL=G11 ──┬── AXP2101 @0x34
                                               ├── AW9523B @0x58
                                               ├── FT6336U @0x38
                                               ├── BM8563  @0x51
                                               ├── BMI270  @0x69
                                               ├── ES7210  @0x40
                                               ├── AW88298 @0x36
                                               ├── GC0308  @0x21
                                               └── LTR-553 @0x23
```

**Failure Analysis:**

A complete absence of ACKs on all addresses (zero devices found)
indicates that the I2C bus itself is not operational — either the
SDA/SCL pull-up resistors are missing or the ESP32-S3 I2C controller
is not clocking the bus. If some devices respond but a specific
address does not, the failure is local to that device: either its
power rail (AXP2101 LDO) is off, the corresponding RST line (via
AW9523B) is held asserted, the device is not populated on this SKU,
or the device itself is damaged.

If any failure occurs in this test, try the Debugging Steps to narrow
down the issue.

1. Check that SDA (G12) and SCL (G11) show valid logic levels with
   a voltmeter or oscilloscope. The idle state should be weakly pulled
   to 3.3 V.
2. If the bus is idle and no device responds, verify the AXP2101 PMIC
   is delivering system 3.3 V power.
3. If a single known address returns NACK, check the corresponding
   power rail and AW9523B RST line for that peripheral.
4. Replace the CoreS3 board and retest if a P0 device remains
   unresponsive after verifying power and reset.

---

#### Platform — PMU Register Test (AXP2101)

This test verifies communication with the AXP2101 PMIC at address 0x34.
The chip version register (0x01) is read and must return a non-zero
value. The power status register (0x00) is read to determine VBUS
presence, battery connection, and charging state. Battery voltage is
read as a 12-bit ADC value from registers 0x34–0x35 (1.1 mV/LSB) and
converted to millivolts. Battery percentage is estimated from a
voltage-based lookup: ≥4200 mV → 100 %, 3700–4200 mV → linear
0–100 %, 3400–3700 mV → 0–10 %, <3400 mV → 0 %. VBUS voltage,
charge current, and die temperature are also read from their respective
ADC registers and reported.

If the battery-present bit (register 0x00 bit 3) is 0, battery
percentage is reported as 0 and the test emits a warning — this is
expected if the battery is fully depleted or disconnected.

##### Test Path Block Diagram

```
ESP32-S3 ── I2C@0x34 ── AXP2101 PMIC ──┬── Battery (500 mAh LiPo)
                                         ├── VBUS (USB Type-C)
                                         ├── DLDO1 → LCD backlight
                                         ├── LDOIO0 → FT6336U VCC
                                         └── CHG_LED → Red LED
```

**Failure Analysis:**

If the AXP2101 does not ACK at 0x34, the entire board is without
power management — all downstream peripherals (AW9523B, FT6336U,
ILI9342C, BM8563, BMI270, audio, camera) are unreachable. This
failure is catastrophic and all subsequent tests must report
PRECONDITION_FAILED.

If any failure occurs in this test, try the Debugging Steps to narrow
down the issue.

1. Measure VBUS voltage on the USB Type-C connector. Without VBUS or
   battery, the AXP2101 has no power.
2. Check SDA (G12) and SCL (G11) for valid I2C signalling.
3. Replace the CoreS3 board and retest.

---

#### Platform — GPIO Expander Test (AW9523B)

This test verifies communication with the AW9523B GPIO expander at
address 0x58. The chip ID register (0x10) must return 0x23. If the ID
matches, LED mode is disabled on all pins (write 0x00 to registers 0x12
and 0x13), and each output pin is toggled low then high while reading
back the corresponding input register level to confirm the pin state.

Pins tested: P0_0 (TOUCH_RST), P0_2 (AW_RST), P1_0 (CAM_RST), P1_1
(LCD_RST). After the test, all pins are restored to their safe
default state.

##### Test Path Block Diagram

```
ESP32-S3 ── I2C@0x58 ── AW9523B ──┬── P0_0 → FT6336U RST
                                    ├── P0_2 → AW88298 RST
                                    ├── P1_0 → GC0308 RST
                                    ├── P1_1 → ILI9342C RST
                                    ├── P1_2 ← FT6336U INT
                                    └── P1_3 ← AW88298 INT
```

**Failure Analysis:**

If the AW9523B does not ACK at 0x58, the touch, display, audio, and
camera RST lines cannot be controlled. All four subsystems report
PRECONDITION_FAILED. If the chip ACKs but the chip ID is not 0x23,
a different or defective expander may be populated.

If any failure occurs in this test, try the Debugging Steps to narrow
down the issue.

1. Verify the AXP2101 is operational (PMU Register Test must pass).
2. Check that the AW9523B is on the SYS I2C bus at 0x58 by running
   the I2C Bus Scan.
3. Replace the CoreS3 board and retest.

---

#### Display — LCD Internal Test (ILI9342C)

This test initialises the ILI9342C display controller over the shared
SPI2 bus (MOSI=G37, SCK=G36, CS=G3, DC=G35). LCD_RST is controlled via
AW9523B P1_1. Backlight power is supplied by AXP2101 DLDO1 (register
0x12 written to 0x0C). The test proceeds through the following sequence:

1. Precondition check: AXP2101 and AW9523B must both respond. If not,
   the test reports PRECONDITION_FAILED.
2. Assert AW9523B P1_1 = 0 for 10 ms, then release.
3. Send the full ILI9342C init sequence (commands 0xCB, 0xCF, 0xE8,
   0xEA, 0xB1, 0xB6, 0xF2, 0x3A=0x55, 0x36, gamma curves, 0x11
   SLEEP_OUT with 120 ms wait, 0x29 DISP_ON).
4. Fill the 320×240 frame buffer sequentially with RED, GREEN, BLUE,
   and BLACK, each displayed for 500 ms.
5. Draw the text "CoreS3 Diagnostic" in cyan in the centre of the
   screen.
6. Draw horizontal and vertical white crosshair lines through the
   centre of the screen.
7. De-init the display and release the SPI bus.

##### Test Path Block Diagram

```
ESP32-S3 ── SPI2 (MOSI=G37, SCK=G36, CS=G3, DC=G35) ── ILI9342C
                │                                            │
                └── AW9523B P1_1 ── LCD_RST (active-low) ────┘
AXP2101 DLDO1 (reg 0x12) ── LX1 ── LCD backlight LED
```

**Failure Analysis:**

A failure at any step of the init sequence (SPI transaction error,
timeout on SLEEP_OUT) indicates either a bad SPI connection to the
ILI9342C, an incorrectly configured CS or DC pin, or a held-low RST
line. If the init succeeds but colour fills are visibly incorrect
(bars missing, wrong colours), the SPI clock rate or MADCTL register
(colour order / orientation) may be misconfigured. If the backlight
does not illuminate, the AXP2101 DLDO1 register may not be set
correctly.

If any failure occurs in this test, try the Debugging Steps to narrow
down the issue.

1. Run the I2C Bus Scan to confirm AXP2101 and AW9523B are present.
2. Run the PMU Register Test and verify that DLDO1 (register 0x12)
   reads back 0x0C.
3. Run the GPIO Expander Test and verify that P1_1 toggles correctly.
4. Check SPI2 signals (MOSI=G37, SCK=G36, CS=G3, DC=G35) with an
   oscilloscope during the init sequence.
5. Replace the CoreS3 board and retest.

---

#### Touch — Touch Register Test (FT6336U)

This test verifies the FT6336U capacitive touch controller over SYS I2C
at address 0x38. Touch power is supplied by the AXP2101 LDOIO0
(register 0x90 written to 0x07 for 3.3 V output). The AW9523B GPIO
expander controls TOUCH_RST (P0_0, active-low) and receives TOUCH_INT
(P1_2, open-drain input).

The test sequence is:

1. Precondition: verify AXP2101 and AW9523B respond.
2. Write AXP2101 register 0x90 = 0x07. Wait 50 ms for the LDO to
   stabilise.
3. Assert AW9523B P0_0 = 0 for 10 ms, then release. Wait 50 ms for
   the FT6336U internal boot sequence.
4. Probe the I2C bus at 0x38. If NACK, try 0x3A (alternative address).
   If both fail, report FAILED.
5. Read the device mode register (0x00). Expected values: 0x00 or 0x03.
6. Read the firmware version register (0xA3). Must be non-zero.

After the register test, the touch-point read test reads the touch
status register (0x02) to obtain the number of active touch points,
then reads coordinates from registers 0x03–0x0D for each point.

##### Test Path Block Diagram

```
AXP2101 LDOIO0 (reg 0x90) ── FT6336U VCC
ESP32-S3 ── I2C@0x38 ── FT6336U
AW9523B P0_0 ── FT6336U RST
AW9523B P1_2 ←── FT6336U INT
```

**Failure Analysis:**

If the FT6336U does not ACK at 0x38 or 0x3A, the most likely cause is
the power rail (AXP2101 LDOIO0 not enabled) or the RST line (AW9523B
P0_0 held low). If the chip ACKs but the device mode register returns
an unexpected value, the chip may be in an error state (e.g. firmware
CRC failure). If the firmware version reads 0, the chip is present but
not operational.

If any failure occurs in this test, try the Debugging Steps to narrow
down the issue.

1. Run the PMU Register Test and verify AXP2101 communication.
2. Run the GPIO Expander Test and verify P0_0 toggles.
3. Run the I2C Bus Scan to confirm whether 0x38 responds at all.
4. Probe the FT6336U at 0x3A as an alternative address.
5. Check the LDOIO0 voltage on the FT6336U VCC pin with a voltmeter.
6. Replace the CoreS3 board and retest.

---

#### RTC — RTC Register Test (BM8563)

This test verifies the BM8563 real-time clock at I2C address 0x51.
Seven time registers (seconds through years, registers 0x02–0x08) are
read as BCD values and converted to binary. The test validates that
month (1–12), day (1–31), hour (0–23), minute (0–59), and second
(0–59) are within expected ranges. If the seconds register has the VL
(validity) flag (bit 7) set, the RTC has lost power and the time is
invalid — the test clears the flag and reports a warning.

##### Test Path Block Diagram

```
ESP32-S3 ── I2C@0x51 ── BM8563 RTC
                             │
AXP2101 RTC_VDD ────────────┘ (backup power)
```

**Failure Analysis:**

If the BM8563 does not ACK at 0x51, the RTC_VDD backup rail from the
AXP2101 may be missing, or the I2C bus address (0x51) may be
contested by another device. If the VL flag is set, the RTC has lost
its time-keeping register contents — this is expected if the main
battery was disconnected for an extended period.

If any failure occurs in this test, try the Debugging Steps to narrow
down the issue.

1. Run the I2C Bus Scan to confirm that address 0x51 is present.
2. Verify that the AXP2101 is providing RTC_VDD (always on while
   VBUS or battery is present).
3. Clear the VL flag by writing to register 0x02 and re-read; if
   the flag remains set, the RTC internal oscillator may have stopped.
4. Replace the CoreS3 board and retest.

---

#### IMU — IMU Register Test (BMI270)

This test verifies the BMI270 6-axis IMU at I2C address 0x69. The
chip ID register (0x00) is read and must equal 0x24. A soft-reset
command (0xB6 to register 0x7C) is issued, followed by a 2 ms wait.
The accelerometer and gyroscope are enabled by writing 0x03 to
register 0x7E. After a 10 ms stabilisation, six bytes of accelerometer
data (registers 0x03–0x08) and six bytes of gyroscope data (registers
0x09–0x0E) are read. The raw 16-bit two's-complement values are
converted to milli-g (±2g range, 0.061 mg/LSB) and milli-degrees-per-second
(±2000 dps range, 61 mdps/LSB) and reported.

##### Test Path Block Diagram

```
ESP32-S3 ── I2C@0x69 ── BMI270 ──┬── Internal accelerometer
                                   ├── Internal gyroscope
                                   └── Aux I2C ── BMM150 magnetometer
```

**Failure Analysis:**

If the BMI270 does not ACK at 0x69, the device is not powered or not
connected. If it ACKs but the chip ID is not 0x24, a different or
defective IMU variant may be populated. If accel and gyro registers
return all zeros after a soft-reset and power-enable sequence, the
internal sensor hub may be stuck or the power-on initialisation may
have failed.

If any failure occurs in this test, try the Debugging Steps to narrow
down the issue.

1. Run the I2C Bus Scan to confirm address 0x69 is present.
2. Verify that the BMI270 power rail (provided by AXP2101) is active.
3. Read the chip ID register five times consecutively — an
   intermittent ID suggests a marginal I2C connection.
4. Replace the CoreS3 board and retest.

---

#### Audio — Speaker Amplifier Test (AW88298)

This on-demand test initialises the AW88298 I2S speaker amplifier at
address 0x36. AW_RST (AW9523B P0_2) is asserted low for 5 ms then
released. The chip ID register is read to confirm communication. The
I2S interface (BCK=G34, WCK=G33, DATO=G14, MCLK=G0) is configured for
16-bit, 48 ksps mono output. A 1 kHz sine wave is generated in
software and output for 500 ms. The test then stops the tone and
de-initialises the I2S interface.

##### Test Path Block Diagram

```
ESP32-S3 ── I2C@0x36 ── AW88298 ── Speaker (1 W)
                │
AW9523B P0_2 ──┤ (AW_RST)
                │
I2S: BCK=G34, WCK=G33, DATO=G14, MCLK=G0
```

**Failure Analysis:**

If the AW88298 does not ACK at 0x36, the amplifier is not reachable
on the I2C bus — either its power rail is off or its RST line
(AW9523B P0_2) is held low. If the chip ACKs but no audio is heard,
either the I2S configuration (BCK/WCK/DATO/MCLK) is incorrect,
the amplifier is in a fault state (check AW_INT via AW9523B P1_3),
or the speaker itself is disconnected.

If any failure occurs in this test, try the Debugging Steps to narrow
down the issue.

1. Run the PMU Register Test and GPIO Expander Test to confirm
   AXP2101 and AW9523B are operational.
2. Toggle AW9523B P0_2 manually and verify the AW88298 RST line
   changes state.
3. Read the AW88298 fault register (vendor-specific) through the
   I2C interface.
4. Check the I2S signals (BCK=G34, WCK=G33, DATO=G14) with an
   oscilloscope during tone generation.
5. Replace the CoreS3 board and retest.

---

#### Audio — Microphone ADC Test (ES7210)

This on-demand test initialises the ES7210 audio ADC at address 0x40.
The chip ID register is read to confirm communication. The I2S
interface (BCK=G34, WCK=G33, DATI=G13, MCLK=G0) is configured for
16-bit, 48 ksps stereo input. Audio data is captured for 100 ms
(4800 samples per channel). The RMS level of each channel is computed
and reported. A non-zero RMS indicates that the microphone circuit is
functional.

##### Test Path Block Diagram

```
ESP32-S3 ── I2C@0x40 ── ES7210 ── Dual microphones
I2S: BCK=G34, WCK=G33, DATI=G13, MCLK=G0
```

**Failure Analysis:**

If the ES7210 does not ACK at 0x40, the ADC is not reachable — the
power rail or the I2C connection is suspect. If the chip ACKs but all
captured samples are zero, either the I2S DATI line (G13) is not
connected, the microphones are not powered, or the ES7210 input gain
is set to zero.

If any failure occurs in this test, try the Debugging Steps to narrow
down the issue.

1. Run the PMU Register Test to confirm AXP2101 is delivering power.
2. Run the I2C Bus Scan to confirm address 0x40 responds.
3. Check DATI=G13 with an oscilloscope while speaking into the
   microphones.
4. Replace the CoreS3 board and retest.

---

#### Storage — SD Card Mount Test

This on-demand test mounts a microSD card in the SPI2 bus slot
(CS=G4, MOSI=G37, MISO=G35, SCK=G36) using the `sdspi_host` driver
with FAT32 filesystem support. If mounting fails, the test reports
that no card is present or the filesystem format is unsupported; this
is a warning, not a failure (the card may be absent). If mounting
succeeds, the card capacity is read and logged, a 4 KB test file is
written with a known pattern (0xAA repeated), read back, and verified.
The test file is deleted and the card is unmounted.

##### Test Path Block Diagram

```
ESP32-S3 ── SPI2 (CS=G4, MOSI=G37, MISO=G35, SCK=G36) ── microSD slot
```

**Failure Analysis:**

A mount failure with a card present indicates either a bad electrical
connection on the SPI bus (CS=G4, MISO=G35), a file system format
other than FAT32, or a card capacity exceeding 16 GB. A write-verify
mismatch indicates a failing card or an intermittent SPI bus.

If any failure occurs in this test, try the Debugging Steps to narrow
down the issue.

1. Confirm that the microSD card is inserted and formatted as FAT32.
2. Test with a known-good card of 16 GB or less.
3. Verify SPI2 signals (CS=G4, MOSI=G37, MISO=G35, SCK=G36) with
   an oscilloscope.
4. Replace the microSD card and retest.

---

#### Input — Button Read Test

This on-demand test configures GPIO41 as an input with a pull-up
resistor. The operator is prompted to press the side (PWR) button
within 5 seconds. The test polls GPIO41 for a low level (button
pressed) and reports whether the press was detected.

##### Test Path Block Diagram

```
Side button (PWR) ── G41 ── ESP32-S3 GPIO (input with pull-up)
```

**Failure Analysis:**

A timeout with no button press may indicate a bad solder joint on
the side button, a damaged button switch, or simply that no operator
was present to press it. This is advisory — the test passing is
operator-dependent.

If any failure occurs in this test, try the Debugging Steps to narrow
down the issue.

1. Press and hold the PWR button while the test is waiting.
2. Verify continuity between the button terminal and G41 with a
   multimeter.
3. Replace the CoreS3 board and retest.

---

#### Connectivity — Wi-Fi Station Test

This test joins a Wi-Fi access point in station mode and verifies that
the ESP32-S3 obtains an IP address via DHCP. On success it reports the
assigned IP, RSSI (with an excellent/good/fair/poor quality rating),
and channel. Credentials are resolved at runtime: NVS settings written
by `wifi-set` take priority over the `CONFIG_WIFI_DIAG_*` menuconfig
defaults. If no SSID is configured, the test reports SKIPPED with an
advisory note. The test is part of `run-all`; in a no-network
environment it reports SKIPPED (not FAILED) so a bare factory board
still passes the batch.

##### Test Path Block Diagram

```
ESP32-S3 radio ── 2.4 GHz ── Wi-Fi AP ── DHCP server
     │                                   │
     ├── (success) ── IP + RSSI + channel reported
     └── (failure)  ── disconnect reason code reported
```

**Result mapping (disconnect reason → test result):**

| Condition | Reason code | Result |
|-----------|-------------|--------|
| No SSID configured | — | SKIPPED (advisory) |
| AP not found / out of range | `WIFI_REASON_NO_AP_FOUND` | SKIPPED (advisory) |
| Wrong password / auth failure | `AUTH_FAIL`, `HANDSHAKE_TIMEOUT`, `MIC_FAILURE`, `NO_AP_FOUND_W_COMPATIBLE_SECURITY` | FAILED (password hint) |
| Any other connect/DHCP failure | other | FAILED (reachability hint) |

**Failure Analysis:**

A FAILED result after a successful prior run may indicate the AP is
out of range, the password changed, the DHCP server is unavailable, or
the AP is operating on a channel/band the ESP32-S3 cannot join (2.4 GHz
only). A SKIPPED result is not a failure — it means no credentials are
configured or no matching AP is present.

If any failure occurs in this test, try the Debugging Steps to narrow
down the issue.

1. Verify the AP is powered on, in range, and broadcasting 2.4 GHz.
2. Re-check the SSID spelling: `wifi-set ssid <name>`.
3. Re-check the password: `wifi-set pass <password>` (case-sensitive).
4. Confirm the AP security mode (WPA2/WPA3) is supported.
5. Confirm the DHCP server is running on the AP/LAN.
6. Review the disconnect reason code printed by `wifi connect`.

---

#### Camera — Camera Register Test (GC0308)

This on-demand test checks the GC0308 camera sensor at I2C address 0x21.
CAM_RST (AW9523B P1_0) is asserted low for 10 ms then released. The
chip ID register is read. If the address probes as NACK, the test
reports SKIPPED — the camera flex cable is an optional assembly.

##### Test Path Block Diagram

```
ESP32-S3 ── I2C@0x21 ── GC0308 ── DVP bus (G45/G46/G38/G39-42/G15-16/G47-48)
AW9523B P1_0 ── CAM_RST
```

**Failure Analysis:**

If the GC0308 does not ACK and this is unexpected (the unit should
have a camera), the flex cable may be loose, the CAM_RST line may be
held low, or the camera module itself may be defective.

If any failure occurs in this test, try the Debugging Steps to narrow
down the issue.

1. Check that the camera flex cable is fully inserted at both ends.
2. Run the GPIO Expander Test and verify P1_0 toggles correctly.
3. Replace the camera flex module and retest.

---

#### Proximity — Proximity Read Test (LTR-553)

This on-demand test reads the LTR-553ALS-WA proximity and ambient-light
sensor at I2C address 0x23. If the address probes as NACK, the test
reports SKIPPED — the sensor is on the same optional flex cable as the
camera. ALS channel data and proximity data registers are read and
their raw values reported.

##### Test Path Block Diagram

```
ESP32-S3 ── I2C@0x23 ── LTR-553ALS-WA (on camera flex cable)
```

**Failure Analysis:**

If the LTR-553 does not ACK and the camera is present and passing,
the LTR-553 itself may be defective. If both the camera and LTR-553
fail, the shared flex cable or its connector is suspect.

If any failure occurs in this test, try the Debugging Steps to narrow
down the issue.

1. Verify the camera flex cable is fully seated.
2. Run the I2C Bus Scan and check whether 0x23 responds.
3. Replace the camera flex module and retest.

---

### Utilities

#### I2C Bus Scan Utility

The `i2c-scan` test doubles as a utility: any operator can run it on
demand to list all I2C devices present on the bus. It is the same code
path as the explicit test case described above but is also documented
here as a manual debugging entry point.

#### System Status Utility

The `status` command runs a non-destructive probe of every P0 and P1
component in parallel (I2C probe only, no init sequences) and displays
a single-page summary table. This is the recommended first step when
debugging a unit:

```
AXP2101  @0x34:  OK (v0x15, BAT=4120mV 98%)
AW9523B  @0x58:  OK
FT6336U  @0x38:  OK (fw=0x02)
ILI9342C (SPI):  OK
BM8563   @0x51:  OK (2025-07-19 14:32)
BMI270   @0x69:  OK (chip_id=0x24)
```

#### NTP Time Sync Utility

The `ntp-sync` command synchronises the BM8563 RTC from an NTP server
over the Wi-Fi connection. The server defaults to
`CONFIG_WIFI_DIAG_NTP_SERVER` (`pool.ntp.org`) and can be overridden
per-run: `ntp-sync <server>`. On success the RTC is written with the
UTC time (no timezone handling); on any failure — no Wi-Fi, sync
timeout, or an implausible time (outside 2024–2100) — the RTC is left
untouched and the command returns FAILED.

#### Diagnostic Upload Utility

The `upload` command builds a JSON report of the runner's test records
and the error context, then POSTs it to the configured URL
(`CONFIG_WIFI_DIAG_UPLOAD_URL`, overridable via `wifi-set url`).
An HTTP 2xx response is required for PASSED.

```json
{
  "app": "m5s3_diag", "idf": "v6.0.2",
  "mac": "a4:cf:12:34:56:78",
  "wifi": { "ssid": "MyNet", "ip": "192.168.1.42", "rssi": -55, "channel": 6 },
  "rtc": "2026-08-01 12:34:56",
  "tests": [ { "id": "wifi", "result": "PASSED", "elapsed_ms": 3214, "message": "..." } ],
  "errors": [ { "component": "WIFI", "location": "MB/WIFI", "message": "...",
                "count": 1, "debug1": "...", "debug2": "..." } ],
  "summary": { "total": 14, "passed": 12, "skipped": 1, "failed": 1 }
}
```

#### MQTT Publish Utility

The `mqtt-pub` command publishes the same JSON report to an MQTT
broker (`CONFIG_WIFI_DIAG_MQTT_URL`, overridable via `wifi-set mqtt`).
The broker URI takes the form `mqtt://host:1883` (TCP, no TLS). The
report is published to topic `m5s3_diag/<mac>` (configurable per-run:
`mqtt-pub <topic>`) with QoS 1 — a broker PUBACK is required for
PASSED.

#### Ping Utility

The `wifi ping <host>` command verifies network reachability over the
Wi-Fi link. It sends 5 ICMP echo requests (64-byte payload, 1 s
interval, 1 s timeout) to the target — an IP literal or hostname
(default `1.1.1.1`) — and reports the reply count plus RTT statistics
(min/avg/max). At least one reply is required for PASSED; the result
is reported after the Wi-Fi session is torn down.

### System Level Tests

#### Burn-In Test (P2, on-demand)

The burn-in test runs all P0 and P1 tests in sequence for a
user-specified number of iterations (default 100) or until the first
failure. Each iteration's result is logged. The total iteration count
and any failures are reported at completion. This test is intended for
intermittent fault reproduction during EDVT and field RMA.

The Wi-Fi Station test is part of `run-all` (and therefore burn-in);
in a no-network environment it reports SKIPPED, which is not counted
as a failure, so burn-in gating is unaffected on bare boards.

---

## End User Interface

The diagnostics package exposes a two-tier interface. The first tier is a
traditional CLI with typed commands:

| Command | Description |
|---------|-------------|
| `help` | Display command list |
| `info` | List all tests and last-run status |
| `run <name\|#>` | Execute a single test by name or numeric index |
| `run-all` | Execute all P0+P1 tests in dependency order |
| `menu` | Enter interactive number menu |
| `errors` | Display the structured error report |
| `status` | Display aggregate system health |
| `reset` | Clear all stored test results and error records |
| `screen-on` / `screen-off` | Manually control the LCD power state |
| `reboot` | Software reset the ESP32-S3 |
| `shutdown` | Power off via AXP2101 |
| `wifi` | Wi-Fi status; `wifi connect` / `wifi disconnect` / `wifi ping <host>` |
| `wifi-set` | Store credentials to NVS: `wifi-set ssid\|pass\|url\|ntp\|mqtt <value>\|clear` |
| `ntp-sync` | Sync RTC from NTP: `ntp-sync [server]` |
| `upload` | POST JSON test report to the upload URL |
| `mqtt-pub` | Publish JSON test report to MQTT broker: `mqtt-pub [topic]` |

The second tier is the `menu` command, which presents a numbered list
of tests and accepts a single-digit (or two-digit) selection:

```
========== CoreS3 Diagnostics ==========
  Errors: 0  |  Run: 0
  [ 1] I2C Bus Scan                     ----
  [ 2] Display (ILI9342C)               ----
  [ 3] Touch (FT6336U)                  ----
  [ 4] RTC (BM8563)                     ----
  [ 5] IMU (BMI270)                     ----
  [ 6] Power (AXP2101)                  ----

  [a]ll  [r]eset  [e]rrors  [q]uit
  Select test #:
```

Test names are 1-indexed. Menu state persists across selections (last
result and error count are displayed until reset).

---

## Configuration and Restrictions

1. **USB D+ integrity**: GPIO20 must never be configured as a GPIO
   output. Doing so terminates the diagnostic session. The LCD
   `hal_screen_set_backlight()` function is explicitly a no-op for this
   reason — backlight control uses AXP2101 DLDO1 instead.
2. **SPI bus sharing**: The LCD and SD card share SPI2; simultaneous
   use causes bus contention. The diagnostics serialise these tests.
3. **Battery charging state**: The power test reports charging and
   battery percentage as informational values. A fully discharged
   battery (voltage below 3.0 V) with no VBUS present causes the
   AXP2101 to report BAT_EXIST = 0, which is not a diagnostic failure.
4. **Camera/proximity flex**: These are optional assemblies. The
   diagnostics detect their absence at the I2C probe level and report
   SKIPPED rather than FAILED.
5. **Wi-Fi credential precedence**: NVS settings written by `wifi-set`
   override the `CONFIG_WIFI_DIAG_*` menuconfig defaults. `wifi-set
   clear` erases the NVS overrides, reverting to the compiled defaults.
   Credentials are stored in plaintext in the NVS `diag_wifi`
   namespace; flash encryption is out of scope.
6. **Wi-Fi upload transport**: The upload URL defaults to plain `http://`;
   TLS is disabled (`CONFIG_ESP_HTTP_CLIENT_ENABLE_HTTPS=n`) to keep
   the image within the 1 MB factory partition. MQTT uses plain TCP
   (`mqtt://`) for the same reason.
7. **NTP timezone**: NTP writes UTC to the RTC; there is no timezone
   handling. The RTC is only modified by an explicit `ntp-sync`; a
   failed sync leaves it untouched.
8. **Wi-Fi radio**: 2.4 GHz only. The ESP32-S3 has no 5 GHz support.
   CoreS3 boards have no external antenna — the built-in PCB antenna
   is used.

---

## Regression Testing and Release

Each diagnostic software release is validated against a known-good
CoreS3 unit. The regression suite consists of `run-all` executed three
consecutive times with zero failures. The test image is built with
ESP-IDF v6.0 and the `idf.py build` toolchain. A candidate release
that produces any FAILED result on a known-good unit is held back
until the root cause is identified and corrected.

`run-all` now includes the Wi-Fi Station test. Regression runs are
performed in the lab Wi-Fi environment; on a board with no credentials
configured the test reports SKIPPED, which is not a failure and does
not hold the release.

---

## Requirements Traceability Considerations

This DFS is derived entirely from the public M5Stack CoreS3 product
page (https://docs.m5stack.com/zh_CN/core/CoreS3) and the ESP32-S3
datasheet. Every I2C address, GPIO assignment, power domain, and
peripheral interface named in this document is traceable to those
sources. No interfaces, buses, or components are tested that are not
described in the source documentation.

---

## References

| Reference | Location |
|-----------|----------|
| M5Stack CoreS3 Product Page | https://docs.m5stack.com/zh_CN/core/CoreS3 |
| ESP32-S3 Datasheet | Espressif ESP32-S3 datasheet v1.x |
| ILI9342C Datasheet | ILItek ILI9342C preliminary v0.1 |
| FT6336U Datasheet | FocalTech FT6336U datasheet |
| BM8563 Datasheet | NXP BM8563 product spec |
| BMI270 Datasheet | Bosch BMI270 datasheet |
| AXP2101 Datasheet | X-Powers AXP2101 datasheet |
| AW9523B Datasheet | Awinic AW9523B datasheet |
| ESP-IDF v6.0 Documentation | https://docs.espressif.com/projects/esp-idf/en/latest/ |

---

## Appendix — Build, Load, and Run

### Build

```bash
source ~/esp/esp-idf/export.sh
cd m5stack_diag
idf.py set-target esp32s3
idf.py build
```

### Flash

```bash
idf.py -p /dev/ttyACM0 flash
```

### Run

```bash
idf.py -p /dev/ttyACM0 monitor
# Or:
screen /dev/ttyACM0 115200
```

The diagnostics start automatically after boot. The `diag>` prompt
appears once the USB Serial/JTAG console is ready.

### Command-Line Output

Errors follow the cterr-style structured format:

```
========== Error Report ==========
  Total errors: 2
  [1] I2C / MB/I2C (x2)
      : I2C@0x38 FT6336U: no ACK — check AXP2101 LDOIO0
      > Check power supply to the missing device
      > Verify pull-up resistors and I2C connections
==================================
```
